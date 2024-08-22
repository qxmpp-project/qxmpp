// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutgoingClient.h"

#include "QXmppBindIq.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMessage.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppOutgoingClient_p.h"
#include "QXmppPacket_p.h"
#include "QXmppPingIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Algorithms.h"
#include "Stream.h"
#include "StringLiterals.h"

#include <unordered_map>

#include <QHostAddress>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QTimer>

using std::visit;
using namespace std::chrono_literals;
using namespace QXmpp;
using namespace QXmpp::Private;

template<typename Result1, typename Result2, typename Handler>
auto join(QXmppTask<Result1> t1, QXmppTask<Result2> t2, QObject *context, Handler handler)
{
    using T = std::invoke_result_t<Handler, Result1, Result2>;

    QXmppPromise<T> p;
    t1.then(context, [context, p, t2, handler = std::move(handler)](Result1 &&result1) mutable {
        t2.then(context, [p = std::move(p), handler = std::move(handler), result1 = std::move(result1)](Result2 &&result2) mutable {
            p.finish(handler(std::move(result1), std::move(result2)));
        });
    });

    return p.task();
}

using ServerAddressesResult = std::variant<std::vector<ServerAddress>, QXmppError>;

static QXmppTask<ServerAddressesResult> lookupXmppSrvRecords(const QString &domain, const QString &serviceName, ServerAddress::ConnectionType connectionType, QObject *context)
{
    QXmppPromise<ServerAddressesResult> p;
    auto task = p.task();

    auto *dns = new QDnsLookup(QDnsLookup::SRV, u"_" + serviceName + u"._tcp." + domain, context);
    QObject::connect(dns, &QDnsLookup::finished, context, [dns, connectionType, p = std::move(p)]() mutable {
        if (auto error = dns->error(); error != QDnsLookup::NoError) {
            p.finish(QXmppError { dns->errorString(), error });
        } else {
            p.finish(transform<std::vector<ServerAddress>>(dns->serviceRecords(), [connectionType](auto record) {
                return ServerAddress { connectionType, record.target(), record.port() };
            }));
        }
    });

    dns->lookup();
    return task;
}

static QXmppTask<ServerAddressesResult> lookupXmppClientRecords(const QString &domain, QObject *context)
{
    return lookupXmppSrvRecords(domain, u"xmpp-client"_s, ServerAddress::Tcp, context);
}

static QXmppTask<ServerAddressesResult> lookupXmppsClientRecords(const QString &domain, QObject *context)
{
    return lookupXmppSrvRecords(domain, u"xmpps-client"_s, ServerAddress::Tls, context);
}

// Looks up xmpps-client and xmpp-client and combines them.
static QXmppTask<ServerAddressesResult> lookupXmppClientHybridRecords(const QString &domain, QObject *context)
{
    // prefer XMPPS records over XMPP records as direct TLS saves one round trip
    return join(
        lookupXmppsClientRecords(domain, context),
        lookupXmppClientRecords(domain, context),
        context,
        [](ServerAddressesResult &&r1, ServerAddressesResult &&r2) -> ServerAddressesResult {
            std::vector<ServerAddress> addresses;
            bool isError1 = std::holds_alternative<QXmppError>(r1);
            bool isError2 = std::holds_alternative<QXmppError>(r2);

            // no records could be fetched
            if (isError1 && isError2) {
                return std::get<QXmppError>(std::move(r2));
            }
            if (!isError1) {
                addresses = std::get<std::vector<ServerAddress>>(std::move(r1));
            }
            if (!isError2) {
                // append other addresses
                auto &&addresses2 = std::get<std::vector<ServerAddress>>(std::move(r2));
                addresses.insert(addresses.end(),
                                 std::make_move_iterator(addresses2.begin()),
                                 std::make_move_iterator(addresses2.end()));
            }

            return std::move(addresses);
        });
}

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate(QXmppOutgoingClient *qq)
    : socket(qq),
      streamAckManager(socket),
      iqManager(qq, streamAckManager),
      listener(qq),
      fastTokenManager(config),
      c2sStreamManager(qq),
      csiManager(qq),
      pingManager(qq),
      q(qq)
{
}

void QXmppOutgoingClientPrivate::connectToHost(const ServerAddress &address)
{
    auto sslConfig = QSslConfiguration::defaultConfiguration();

    // override CA certificates if requested
    if (!config.caCertificates().isEmpty()) {
        sslConfig.setCaCertificates(config.caCertificates());
    }
    // ALPN protocol 'xmpp-client'
    sslConfig.setAllowedNextProtocols({ QByteArrayLiteral("xmpp-client") });

    // set new ssl config
    q->socket()->setSslConfiguration(sslConfig);

    // respect proxy
    q->socket()->setProxy(config.networkProxy());
    // set the name the SSL certificate should match
    q->socket()->setPeerVerifyName(config.domain());

    socket.connectToHost(address);
}

void QXmppOutgoingClientPrivate::connectToAddressList(std::vector<ServerAddress> &&addresses)
{
    serverAddresses = std::move(addresses);
    nextServerAddressIndex = 0;
    connectToNextAddress();
}

void QXmppOutgoingClientPrivate::connectToNextAddress()
{
    nextAddressState = Current;
    connectToHost(serverAddresses.at(nextServerAddressIndex++));
}

///
/// Constructs an outgoing client stream.
///
QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppOutgoingClientPrivate>(this))
{
    // initialise socket
    auto *socket = new QSslSocket(this);
    d->socket.setSocket(socket);

    connect(socket, &QAbstractSocket::disconnected, this, &QXmppOutgoingClient::_q_socketDisconnected);
    connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QXmppOutgoingClient::socketSslErrors);
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppOutgoingClient::socketError);

    connect(&d->socket, &XmppSocket::started, this, &QXmppOutgoingClient::handleStart);
    connect(&d->socket, &XmppSocket::stanzaReceived, this, &QXmppOutgoingClient::handlePacketReceived);
    connect(&d->socket, &XmppSocket::streamReceived, this, &QXmppOutgoingClient::handleStream);
    connect(&d->socket, &XmppSocket::streamClosed, this, &QXmppOutgoingClient::disconnectFromHost);
}

QXmppOutgoingClient::~QXmppOutgoingClient()
{
    // causes tasks to be finished
    d->streamAckManager.resetCache();
    d->iqManager.cancelAll();
}

/// Returns a reference to the stream's configuration.
QXmppConfiguration &QXmppOutgoingClient::configuration()
{
    return d->config;
}

/// Returns access to the XMPP socket.
XmppSocket &QXmppOutgoingClient::xmppSocket() const
{
    return d->socket;
}

/// Returns the manager for packet acknowledgements from Stream Management.
StreamAckManager &QXmppOutgoingClient::streamAckManager() const
{
    return d->streamAckManager;
}

/// Returns the manager for outgoing IQ request tracking.
OutgoingIqManager &QXmppOutgoingClient::iqManager() const
{
    return d->iqManager;
}

/// Returns the manager for C2s Stream Management.
C2sStreamManager &QXmppOutgoingClient::c2sStreamManager() const
{
    return d->c2sStreamManager;
}

/// Returns the manager for enabling of message carbons via bind2.
CarbonManager &QXmppOutgoingClient::carbonManager() const
{
    return d->carbonManager;
}

/// Returns the manager for \xep{0352, Client State Indication}.
CsiManager &QXmppOutgoingClient::csiManager() const
{
    return d->csiManager;
}

/// Attempts to connect to the XMPP server.
void QXmppOutgoingClient::connectToHost()
{
    // if a host for resumption is available, connect to it
    if (d->c2sStreamManager.hasResumeAddress()) {
        auto [host, port] = d->c2sStreamManager.resumeAddress();
        d->connectToHost({ ServerAddress::Tcp, host, port });
        return;
    }

    // if an explicit host was provided, connect to it
    if (!d->config.host().isEmpty() && d->config.port()) {
        auto connectionType = d->config.streamSecurityMode() == QXmppConfiguration::LegacySSL
            ? ServerAddress::Tls
            : ServerAddress::Tcp;
        d->connectToHost({ connectionType, d->config.host(), d->config.port16() });
        return;
    }

    // legacy SSL
    if (d->config.streamSecurityMode() == QXmppConfiguration::LegacySSL) {
        if (!QSslSocket::supportsSsl()) {
            setError(u"Direct TLS is configured, but TLS support is not available locally"_s,
                     QAbstractSocket::SocketError::SslInternalError);
            return;
        }

        d->connectToAddressList({
            ServerAddress { ServerAddress::Tls, d->config.domain(), XMPPS_DEFAULT_PORT },
            ServerAddress { ServerAddress::Tls, d->config.domain(), XMPP_DEFAULT_PORT },
        });
        return;
    }

    // otherwise, lookup server
    const auto domain = d->config.domain();

    debug(u"Looking up service records for domain %1"_s.arg(domain));
    auto recordsTask = QSslSocket::supportsSsl()
        ? lookupXmppClientHybridRecords(domain, this)
        : lookupXmppClientRecords(domain, this);

    recordsTask.then(this, [this, domain](auto result) {
        if (auto error = std::get_if<QXmppError>(&result)) {
            warning(u"Lookup for domain %1 failed: %2"_s
                        .arg(domain, error->description));

            // as a fallback, use domain as hostname
            d->connectToAddressList({
                ServerAddress { ServerAddress::Tls, d->config.domain(), XMPPS_DEFAULT_PORT },
                ServerAddress { ServerAddress::Tcp, d->config.domain(), XMPP_DEFAULT_PORT },
            });
            return;
        }

        if (std::get<std::vector<ServerAddress>>(result).empty()) {
            warning(u"'%1' has no xmpp-client service records."_s.arg(domain));

            // as a fallback, use domain as hostname
            d->connectToAddressList({
                ServerAddress { ServerAddress::Tls, d->config.domain(), XMPPS_DEFAULT_PORT },
                ServerAddress { ServerAddress::Tcp, d->config.domain(), XMPP_DEFAULT_PORT },
            });
            return;
        }

        d->connectToAddressList(std::get<std::vector<ServerAddress>>(std::move(result)));
    });
}

///
/// Disconnects from the server and resets the stream management state.
///
/// \since QXmpp 1.0
///
void QXmppOutgoingClient::disconnectFromHost()
{
    d->c2sStreamManager.onStreamClosed();
    d->socket.disconnectFromHost();
}

/// Returns true if authentication has succeeded.
bool QXmppOutgoingClient::isAuthenticated() const
{
    return d->isAuthenticated;
}

/// Returns true if the socket is connected and a session has been started.
bool QXmppOutgoingClient::isConnected() const
{
    return d->socket.isConnected() && d->sessionStarted;
}

///
/// Sends an IQ and reports the response asynchronously.
///
/// It makes sure that the to address is set so the stream can correctly check the reponse's
/// sender.
///
/// \since QXmpp 1.5
///
QXmppTask<IqResult> QXmppOutgoingClient::sendIq(QXmppIq &&iq)
{
    // If 'to' is empty the user's bare JID is meant implicitly (see RFC6120, section 10.3.3.).
    auto to = iq.to();
    return d->iqManager.sendIq(std::move(iq), to.isEmpty() ? d->config.jidBare() : to);
}

QSslSocket *QXmppOutgoingClient::socket() const
{
    return d->socket.socket();
}

void QXmppOutgoingClient::_q_socketDisconnected()
{
    debug(u"Socket disconnected"_s);
    d->isAuthenticated = false;
    if (d->nextAddressState == QXmppOutgoingClientPrivate::TryNext) {
        d->connectToNextAddress();
    } else if (d->redirect) {
        d->connectToHost({ ServerAddress::Tcp, d->redirect->host, d->redirect->port });
        d->redirect.reset();
    } else {
        closeSession();
    }
}

void QXmppOutgoingClient::socketSslErrors(const QList<QSslError> &errors)
{
    // log errors
    warning(u"SSL errors"_s);
    for (int i = 0; i < errors.count(); ++i) {
        warning(errors.at(i).errorString());
    }

    // relay signal
    Q_EMIT sslErrors(errors);

    // if configured, ignore the errors
    if (configuration().ignoreSslErrors()) {
        socket()->ignoreSslErrors();
    }
}

void QXmppOutgoingClient::startSasl2Auth(const Sasl2::StreamFeature &sasl2Feature)
{
    // prepare bind2 request
    auto createBind2Request = [this](const auto &bind2Features) {
        Bind2Request request;
        request.tag = d->config.resourcePrefix();
        // extensions
        d->carbonManager.onBind2Request(request, bind2Features);
        d->csiManager.onBind2Request(request, bind2Features);
        d->c2sStreamManager.onBind2Request(request, bind2Features);
        return request;
    };

    // prepare authenticate request
    Sasl2::Authenticate sasl2Request;
    // bind2
    if (sasl2Feature.bind2Feature) {
        sasl2Request.bindRequest = createBind2Request(sasl2Feature.bind2Feature->features);
    }
    // other extensions
    d->fastTokenManager.onSasl2Authenticate(sasl2Request, sasl2Feature);
    d->c2sStreamManager.onSasl2Authenticate(sasl2Request, sasl2Feature);

    // start authentication
    d->setListener<Sasl2Manager>(&d->socket).authenticate(std::move(sasl2Request), d->config, sasl2Feature, this).then(this, [this](auto result) {
        if (auto success = std::get_if<Sasl2::Success>(&result)) {
            debug(u"Authenticated"_s);
            d->isAuthenticated = true;
            d->authenticationMethod = AuthenticationMethod::Sasl2;
            d->config.setJid(success->authorizationIdentifier);
            d->bind2Bound = std::move(success->bound);

            // extensions
            d->fastTokenManager.onSasl2Success(*success);
            d->c2sStreamManager.onSasl2Success(*success);
            if (d->bind2Bound) {
                d->c2sStreamManager.onBind2Bound(*d->bind2Bound);
            }

            if (success->smResumed) {
                // If the stream could be resumed, the session is immediately started and no stream
                // features are sent.
                openSession();
            } else {
                // new stream features will be sent by the server now
            }
        } else {
            auto [text, err] = std::get<Sasl2Manager::AuthError>(std::move(result));
            setError(text, std::move(err));
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::startNonSaslAuth()
{
    d->setListener<NonSaslAuthManager>(&d->socket).queryOptions(d->streamFrom, d->config.user()).then(this, [this](auto result) {
        if (auto *options = std::get_if<NonSaslAuthOptions>(&result)) {
            bool plainText = false;

            if (options->plain && options->digest) {
                plainText = (d->config.nonSASLAuthMechanism() != QXmppConfiguration::NonSASLDigest);
            } else if (options->plain) {
                plainText = true;
            } else if (options->digest) {
                plainText = false;
            } else {
                // TODO: errors: should trigger error signal
                warning(u"No supported Non-SASL Authentication mechanism available"_s);
                disconnectFromHost();
                return;
            }

            auto task = std::get<NonSaslAuthManager>(d->listener).authenticate(plainText, d->config.user(), d->config.password(), d->config.resource(), d->streamId);
            task.then(this, [this](auto result) {
                if (std::holds_alternative<Success>(result)) {
                    // successful Non-SASL Authentication
                    debug(u"Authenticated (Non-SASL)"_s);
                    d->isAuthenticated = true;
                    d->authenticationMethod = AuthenticationMethod::NonSasl;

                    // xmpp connection made
                    openSession();
                } else {
                    // TODO: errors: should trigger error signal
                    auto &error = std::get<QXmppError>(result);
                    warning(u"Could not authenticate using Non-SASL Authentication: "_s + error.description);
                    disconnectFromHost();
                    return;
                }
            });
        } else {
            // TODO: errors: should trigger error signal
            auto &error = std::get<QXmppError>(result);
            warning(u"Couldn't list Non-SASL Authentication mechanisms: "_s + error.description);
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::startSmResume()
{
    d->listener = &d->c2sStreamManager;
    d->c2sStreamManager.requestResume().then(this, [this] {
        if (d->c2sStreamManager.streamResumed()) {
            openSession();
        } else {
            // check whether bind is available
            if (d->bindModeAvailable) {
                startResourceBinding();
                return;
            }

            // otherwise we are done
            openSession();
        }
    });
}

void QXmppOutgoingClient::startSmEnable()
{
    d->listener = &d->c2sStreamManager;
    d->c2sStreamManager.requestEnable().then(this, [this] {
        // enabling of stream management may or may not have succeeded
        // we are connected now
        openSession();
    });
}

void QXmppOutgoingClient::startResourceBinding()
{
    d->setListener<BindManager>(&d->socket).bindAddress(d->config.resource()).then(this, [this](BindManager::Result r) {
        if (auto *addr = std::get_if<BoundAddress>(&r)) {
            d->config.setUser(addr->user);
            d->config.setDomain(addr->domain);
            d->config.setResource(addr->resource);

            if (d->c2sStreamManager.canRequestEnable()) {
                startSmEnable();
            } else {
                // we are connected now
                openSession();
            }
        } else if (auto *protocolError = std::get_if<ProtocolError>(&r)) {
            setError(u"Resource binding failed: "_s + protocolError->text,
                     StreamError::UndefinedCondition);
            disconnectFromHost();
        } else if (auto *stanzaError = std::get_if<QXmppStanza::Error>(&r)) {
            QString text = u"Resource binding failed: "_s + stanzaError->text();
            setError(text, BindError { *stanzaError });
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::openSession()
{
    info(u"Session established"_s);
    Q_ASSERT(!d->sessionStarted);
    d->sessionStarted = true;

    SessionBegin session {
        d->c2sStreamManager.enabled(),
        d->c2sStreamManager.streamResumed(),
        d->bind2Bound.has_value(),
        d->authenticationMethod == AuthenticationMethod::Sasl2 && d->fastTokenManager.tokenChanged(),
        d->authenticationMethod,
    };
    d->bind2Bound.reset();

    d->iqManager.onSessionOpened(session);
    d->carbonManager.onSessionOpened(session);
    d->csiManager.onSessionOpened(session);
    Q_EMIT connected(session);
}

void QXmppOutgoingClient::closeSession()
{
    d->sessionStarted = false;

    SessionEnd session {
        d->c2sStreamManager.canResume(),
    };

    d->streamAckManager.onSessionClosed();
    d->iqManager.onSessionClosed(session);
    Q_EMIT disconnected(session);
}

void QXmppOutgoingClient::setError(const QString &text, ConnectionError &&details)
{
    auto legacyError = visit<LegacyError>(
        overloaded {
            [](QAbstractSocket::SocketError e) { return e; },
            [](TimeoutError e) { return e; },
            [](StreamError e) {
                switch (e) {
                case StreamError::Conflict:
                    return QXmppStanza::Error::Conflict;
                case StreamError::NotAuthorized:
                    return QXmppStanza::Error::NotAuthorized;
                default:
                    return QXmppStanza::Error::UndefinedCondition;
                }
            },
            [](const AuthenticationError &) {
                return QXmppStanza::Error::NotAuthorized;
            },
            [](const BindError &e) {
                return e.stanzaError.condition();
            },
        },
        details);
    auto clientError = visit(
        overloaded {
            [](QAbstractSocket::SocketError) { return QXmppClient::SocketError; },
            [](TimeoutError) { return QXmppClient::KeepAliveError; },
            [](QXmppStanza::Error::Condition) { return QXmppClient::XmppStreamError; },
        },
        legacyError);

    d->error = { text, std::move(details), legacyError };
    warning(text);
    Q_EMIT errorOccurred(d->error->text, d->error->details, clientError);
}

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError socketError)
{
    if (!d->sessionStarted &&
        (d->serverAddresses.size() > d->nextServerAddressIndex)) {
        // some network error occurred during startup -> try next available SRV record server
        d->nextAddressState = QXmppOutgoingClientPrivate::TryNext;

        // If the socket is connected, wait for disconnect first.
        // If the socket isn't connected, we can directly try the next address.
        if (!d->socket.isConnected()) {
            d->connectToNextAddress();
        }
    } else {
        setError(d->socket.socket()->errorString(), socketError);
    }
}

void QXmppOutgoingClient::handleStart()
{
    // reset stream information
    d->streamId.clear();
    d->streamFrom.clear();
    d->streamVersion.clear();

    // reset active manager (e.g. authentication)
    d->listener = this;

    d->c2sStreamManager.onStreamStart();

    // start stream
    d->socket.sendData(serializeXml(StreamOpen {
        d->config.domain(),
        d->config.user().isEmpty() ? QString() : d->config.jidBare(),
        {},
        u"1.0"_s,
        ns_client.toString(),
    }));
}

void QXmppOutgoingClient::handleStream(const QDomElement &streamElement)
{
    if (d->streamId.isEmpty()) {
        d->streamId = streamElement.attribute(u"id"_s);
    }
    if (d->streamFrom.isEmpty()) {
        d->streamFrom = streamElement.attribute(u"from"_s);
    }
    if (d->streamVersion.isEmpty()) {
        d->streamVersion = streamElement.attribute(u"version"_s);

        // no version specified, signals XMPP Version < 1.0.
        // switch to old auth mechanism if enabled
        if (d->streamVersion.isEmpty() && configuration().useNonSASLAuthentication()) {
            startNonSaslAuth();
        }
    }
}

void QXmppOutgoingClient::handlePacketReceived(const QDomElement &nodeRecv)
{
    // if we receive any kind of data, stop the timeout timer
    d->pingManager.onDataReceived();

    auto index = d->listener.index();

    switch (visit(overloaded {
                      [&](auto *manager) { return manager->handleElement(nodeRecv); },
                      [&](auto &manager) { return manager.handleElement(nodeRecv); },
                  },
                  d->listener)) {
    case Accepted:
        return;
    case Rejected:
        setError(u"Unexpected element received."_s, StreamError::UndefinedCondition);
        disconnectFromHost();
        return;
    case Finished:
        // if the job is done, set OutgoingClient, but do not override a continuation job
        if (d->listener.index() == index) {
            d->listener = this;
        }
        return;
    }
}

HandleElementResult QXmppOutgoingClient::handleElement(const QDomElement &nodeRecv)
{
    // handle SM acks, stanza counter and IQ responses
    if (streamAckManager().handleStanza(nodeRecv) || iqManager().handleStanza(nodeRecv)) {
        return Accepted;
    }

    const QString ns = nodeRecv.namespaceURI();

    // give client opportunity to handle stanza
    bool handled = false;
    Q_EMIT elementReceived(nodeRecv, handled);
    if (handled) {
        return Accepted;
    }

    if (QXmppStreamFeatures::isStreamFeatures(nodeRecv)) {
        QXmppStreamFeatures features;
        features.parse(nodeRecv);
        handleStreamFeatures(features);
        return Accepted;
    } else if (ns == ns_stream && nodeRecv.tagName() == u"error") {
        auto result = StreamErrorElement::fromDom(nodeRecv);
        if (auto *streamError = std::get_if<StreamErrorElement>(&result)) {
            handleStreamError(*streamError);
        }
        return Accepted;
    } else if (ns == ns_client) {
        return handleStanza(nodeRecv) ? Accepted : Rejected;
    }
    return Rejected;
}

void QXmppOutgoingClient::handleStreamFeatures(const QXmppStreamFeatures &features)
{
    // STARTTLS
    if (handleStarttls(features)) {
        return;
    }

    // handle authentication
    const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
    const bool saslAvailable = !features.authMechanisms().isEmpty();

    // SASL 2
    if (features.sasl2Feature().has_value() && d->config.useSasl2Authentication()) {
        startSasl2Auth(features.sasl2Feature().value());
        return;
    }
    // SASL
    if (saslAvailable && configuration().useSASLAuthentication()) {
        d->setListener<SaslManager>(&d->socket).authenticate(d->config, features.authMechanisms(), this).then(this, [this](auto result) {
            if (std::holds_alternative<Success>(result)) {
                debug(u"Authenticated"_s);
                d->isAuthenticated = true;
                d->authenticationMethod = AuthenticationMethod::Sasl;
                handleStart();
            } else {
                auto [text, err] = std::get<SaslManager::AuthError>(std::move(result));
                setError(text, std::move(err));
                disconnectFromHost();
            }
        });
        return;
    }
    // Non-SASL
    if (nonSaslAvailable && configuration().useNonSASLAuthentication()) {
        startNonSaslAuth();
        return;
    }

    // store which features are available
    d->bindModeAvailable = (features.bindMode() != QXmppStreamFeatures::Disabled);
    d->c2sStreamManager.onStreamFeatures(features);
    d->csiManager.onStreamFeatures(features);

    // check whether the stream can be resumed
    if (d->c2sStreamManager.canRequestResume()) {
        startSmResume();
        return;
    }

    // check whether bind is available
    if (d->bindModeAvailable) {
        startResourceBinding();
        return;
    }

    // check whether SM is available
    if (d->c2sStreamManager.canRequestEnable()) {
        startSmEnable();
        return;
    }

    // otherwise we are done
    openSession();
}

void QXmppOutgoingClient::handleStreamError(const QXmpp::Private::StreamErrorElement &streamError)
{
    if (auto *redirect = std::get_if<StreamErrorElement::SeeOtherHost>(&streamError.condition)) {
        d->redirect = std::move(*redirect);

        // only disconnect socket (so stream mangement can resume state is not reset)
        d->socket.disconnectFromHost();
        debug(u"Received redirect to '%1:%2'"_s
                  .arg(redirect->host, redirect->port));
    } else {
        auto condition = std::get<StreamError>(streamError.condition);
        auto text = u"Received stream error (%1): %2"_s
                        .arg(StreamErrorElement::streamErrorToString(condition), streamError.text);

        setError(text, condition);
    }
}

bool QXmppOutgoingClient::handleStanza(const QDomElement &stanza)
{
    Q_ASSERT(stanza.namespaceURI() == ns_client);

    if (stanza.tagName() == u"iq") {
        const auto type = stanza.attribute(u"type"_s);

        if (type == u"result" || type == u"error") {
            // emit iq responses
            QXmppIq iqPacket;
            iqPacket.parse(stanza);
            Q_EMIT iqReceived(iqPacket);
            return true;
        } else if (type == u"get" || type == u"set") {
            // respond with error if we didn't understand the iq request
            QXmppIq iq(QXmppIq::Error);
            iq.setId(stanza.attribute(u"id"_s));
            iq.setTo(stanza.attribute(u"from"_s));
            iq.setError(QXmppStanza::Error {
                QXmppStanza::Error::Cancel,
                QXmppStanza::Error::FeatureNotImplemented,
            });
            d->streamAckManager.send(iq);
            return true;
        }
    } else if (stanza.tagName() == u"presence") {
        QXmppPresence presence;
        presence.parse(stanza);

        // emit presence
        Q_EMIT presenceReceived(presence);
        return true;
    } else if (stanza.tagName() == u"message") {
        QXmppMessage message;
        message.parse(stanza);

        // emit message
        Q_EMIT messageReceived(message);
        return true;
    }
    // unknown/invalid element
    return false;
}

bool QXmppOutgoingClient::handleStarttls(const QXmppStreamFeatures &features)
{
    if (!socket()->isEncrypted()) {
        // determine TLS mode to use
        auto localSecurity = configuration().streamSecurityMode();
        auto remoteSecurity = features.tlsMode();

        // TLS required by config but not offered by server
        if (localSecurity == QXmppConfiguration::TLSRequired && remoteSecurity == QXmppStreamFeatures::Disabled) {
            warning(u"Server does not support TLS"_s);
            disconnectFromHost();
            return true;
        }

        // TLS not available locally, but required by server/config
        if (!QSslSocket::supportsSsl() &&
            (localSecurity == QXmppConfiguration::TLSRequired || remoteSecurity == QXmppStreamFeatures::Required)) {
            warning(u"TLS is required to connect but not available locally"_s);
            disconnectFromHost();
            return true;
        }

        // supported by server, locally and enabled in config
        if (QSslSocket::supportsSsl() &&
            (localSecurity != QXmppConfiguration::TLSDisabled && remoteSecurity != QXmppStreamFeatures::Disabled)) {
            // enable TLS as it is support by both parties
            d->socket.sendData(serializeXml(StarttlsRequest()));
            d->setListener<StarttlsManager>().task().then(this, [this] {
                socket()->startClientEncryption();
            });
            return true;
        }
    }
    return false;
}

void QXmppOutgoingClient::throwKeepAliveError()
{
    setError(u"Ping timeout"_s, TimeoutError());
    disconnectFromHost();
}

void QXmppOutgoingClient::enableStreamManagement(bool resetSequenceNumber)
{
    d->streamAckManager.enableStreamManagement(resetSequenceNumber);
}

bool QXmppOutgoingClient::handleIqResponse(const QDomElement &stanza)
{
    return d->iqManager.handleStanza(stanza);
}

/// Returns the type of the last XMPP stream error that occurred.
QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    if (d->error) {
        if (auto *condition = std::get_if<QXmppStanza::Error::Condition>(&d->error->legacyError)) {
            return *condition;
        }
    }
    return QXmppStanza::Error::NoCondition;
}

namespace QXmpp::Private {

HandleElementResult StarttlsManager::handleElement(const QDomElement &el)
{
    if (StarttlsProceed::fromDom(el)) {
        m_promise.finish();
        return Finished;
    }
    return Rejected;
}

QXmppTask<BindManager::Result> BindManager::bindAddress(const QString &resource)
{
    Q_ASSERT(!m_promise);
    Q_ASSERT(m_iqId.isNull());

    m_promise = QXmppPromise<Result>();

    const auto iq = QXmppBindIq::bindAddressIq(resource);
    m_iqId = iq.id();
    m_socket->sendData(serializeXml(iq));

    return m_promise->task();
}

HandleElementResult BindManager::handleElement(const QDomElement &el)
{
    auto process = [](QXmppBindIq &&iq) -> Result {
        if (iq.type() == QXmppIq::Result) {
            if (iq.jid().isEmpty()) {
                return ProtocolError { u"Server did not return JID upon resource binding."_s };
            }

            static const QRegularExpression jidRegex(u"^([^@/]+)@([^@/]+)/(.+)$"_s);
            if (const auto match = jidRegex.match(iq.jid()); match.hasMatch()) {
                return BoundAddress {
                    match.captured(1),
                    match.captured(2),
                    match.captured(3),
                };
            }

            return ProtocolError { u"Bind IQ received with invalid JID"_s };
        }

        return iq.error();
    };

    if (QXmppBindIq::isBindIq(el) && el.attribute(u"id"_s) == m_iqId) {
        Q_ASSERT(m_promise.has_value());

        auto p = std::move(*m_promise);
        m_iqId.clear();
        m_promise.reset();

        QXmppBindIq bind;
        bind.parse(el);

        // do not accept other IQ types than result and error
        if (bind.type() == QXmppIq::Result || bind.type() == QXmppIq::Error) {
            p.finish(process(std::move(bind)));
            return Finished;
        }
    }
    return Rejected;
}

QXmppTask<NonSaslAuthManager::OptionsResult> NonSaslAuthManager::queryOptions(const QString &streamFrom, const QString &username)
{
    // no other running query
    Q_ASSERT(std::holds_alternative<NoQuery>(m_query));

    m_query = OptionsQuery();
    auto &query = std::get<OptionsQuery>(m_query);

    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(streamFrom);
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(username);

    m_socket->sendData(serializeXml(authQuery));

    return query.p.task();
}

QXmppTask<NonSaslAuthManager::AuthResult> NonSaslAuthManager::authenticate(bool plainText, const QString &username, const QString &password, const QString &resource, const QString &streamId)
{
    // no other running query
    Q_ASSERT(std::holds_alternative<NoQuery>(m_query));

    m_query = AuthQuery();
    auto &query = std::get<AuthQuery>(m_query);

    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(username);
    if (plainText) {
        authQuery.setPassword(password);
    } else {
        authQuery.setDigest(streamId, password);
    }
    authQuery.setResource(resource);
    query.id = authQuery.id();

    m_socket->sendData(serializeXml(authQuery));
    return query.p.task();
}

HandleElementResult NonSaslAuthManager::handleElement(const QDomElement &el)
{
    if (el.tagName() != u"iq") {
        return Rejected;
    }

    if (std::holds_alternative<OptionsQuery>(m_query)) {
        auto query = std::get<OptionsQuery>(std::move(m_query));
        m_query = {};

        auto iqType = el.attribute(u"type"_s);
        if (QXmppNonSASLAuthIq::isNonSASLAuthIq(el) && iqType == u"result") {
            auto queryEl = firstChildElement(el, u"query");

            bool digest = !firstChildElement(queryEl, u"digest").isNull();
            bool plain = !firstChildElement(queryEl, u"password").isNull();

            query.p.finish(NonSaslAuthOptions { plain, digest });
        } else {
            QXmppIq iq;
            iq.parse(el);

            query.p.finish(QXmppError { iq.error().text(), iq.error() });
        }
        return Finished;
    }

    if (std::holds_alternative<AuthQuery>(m_query)) {
        auto query = std::get<AuthQuery>(std::move(m_query));
        m_query = {};

        const auto id = el.attribute(u"id"_s);
        const auto type = el.attribute(u"type"_s);

        if (id == query.id) {
            if (type == u"result") {
                query.p.finish(Success());
            } else if (type == u"error") {
                QXmppIq iq;
                iq.parse(el);

                query.p.finish(QXmppError { iq.error().text(), iq.error() });
            } else {
                query.p.finish(QXmppError { u"Received unexpected IQ response."_s, {} });
            }
        } else {
            query.p.finish(QXmppError { u"Received IQ response with wrong ID."_s, {} });
        }
        return Finished;
    }
    return Rejected;
}

PingManager::PingManager(QXmppOutgoingClient *q)
    : q(q),
      pingTimer(new QTimer(q)),
      timeoutTimer(new QTimer(q))
{
    // send ping timer
    pingTimer->callOnTimeout(q, [this]() { sendPing(); });

    // timeout triggers connection error
    timeoutTimer->setSingleShot(true);
    timeoutTimer->callOnTimeout(q, &QXmppOutgoingClient::throwKeepAliveError);

    // on connect: start ping timer
    QObject::connect(q, &QXmppOutgoingClient::connected, q, [this]() {
        const auto interval = this->q->configuration().keepAliveInterval();

        // start ping timer
        if (interval > 0) {
            pingTimer->setInterval(interval * 1s);
            pingTimer->start();
        }
    });

    // on disconnect: stop all timers
    QObject::connect(q, &QXmppOutgoingClient::disconnected, q, [this]() {
        pingTimer->stop();
        timeoutTimer->stop();
    });
}

void PingManager::onDataReceived()
{
    timeoutTimer->stop();
}

void PingManager::sendPing()
{
    // use smaller stream management ack requests if possible
    if (q->streamAckManager().enabled()) {
        q->streamAckManager().sendAcknowledgementRequest();
    } else {
        // send ping packet
        QXmppPingIq ping;
        ping.setTo(q->configuration().domain());
        q->streamAckManager().send(ping);
    }

    // start timeout timer
    const int timeout = q->configuration().keepAliveTimeout();
    if (timeout > 0) {
        timeoutTimer->setInterval(timeout * 1s);
        timeoutTimer->start();
    }
}

OutgoingIqManager::OutgoingIqManager(QXmppLoggable *l, StreamAckManager &streamAckManager)
    : l(l),
      m_streamAckManager(streamAckManager)
{
}

OutgoingIqManager::~OutgoingIqManager() = default;

QXmppTask<IqResult> OutgoingIqManager::sendIq(QXmppIq &&iq, const QString &to)
{
    if (iq.id().isEmpty()) {
        warning(u"QXmpp: sendIq() error: ID is empty. Using random ID."_s);
        iq.setId(QXmppUtils::generateStanzaUuid());
    }
    if (hasId(iq.id())) {
        warning(u"QXmpp: sendIq() error:"
                "The IQ's ID (\"%1\") is already in use. Using random ID."_s
                    .arg(iq.id()));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }

    return sendIq(QXmppPacket(iq), iq.id(), to);
}

QXmppTask<IqResult> OutgoingIqManager::sendIq(QXmppPacket &&packet, const QString &id, const QString &to)
{
    auto task = start(id, to);

    // the task only finishes instantly if there was an error
    if (task.isFinished()) {
        return task;
    }

    // send request IQ and report sending errors (sending success is not reported in any way)
    m_streamAckManager.send(std::move(packet)).then(l, [this, id](SendResult result) {
        if (std::holds_alternative<QXmppError>(result)) {
            finish(id, std::get<QXmppError>(std::move(result)));
        }
    });

    return task;
}

bool OutgoingIqManager::hasId(const QString &id) const
{
    return m_requests.find(id) != m_requests.end();
}

bool OutgoingIqManager::isIdValid(const QString &id) const
{
    return !id.isEmpty() && !hasId(id);
}

QXmppTask<IqResult> OutgoingIqManager::start(const QString &id, const QString &to)
{
    if (!isIdValid(id)) {
        return makeReadyTask<IqResult>(
            QXmppError { u"Invalid IQ id: empty or in use."_s,
                         SendError::Disconnected });
    }

    if (to.isEmpty()) {
        return makeReadyTask<IqResult>(
            QXmppError { u"The 'to' address must be set so the stream can match the response."_s,
                         SendError::Disconnected });
    }

    auto [itr, success] = m_requests.emplace(id, IqState { {}, to });
    return itr->second.interface.task();
}

void OutgoingIqManager::finish(const QString &id, IqResult &&result)
{
    if (auto itr = m_requests.find(id); itr != m_requests.end()) {
        itr->second.interface.finish(std::move(result));
        m_requests.erase(itr);
    }
}

void OutgoingIqManager::cancelAll()
{
    for (auto &[id, state] : m_requests) {
        state.interface.finish(QXmppError {
            u"IQ has been cancelled."_s,
            QXmpp::SendError::Disconnected });
    }
    m_requests.clear();
}

void OutgoingIqManager::onSessionOpened(const SessionBegin &session)
{
    if (!session.smResumed) {
        // we can't expect a response because this is a new stream
        cancelAll();
    }
}

void OutgoingIqManager::onSessionClosed(const SessionEnd &session)
{
    if (!session.smCanResume) {
        // this stream can't be resumed; we can cancel all ongoing IQs
        cancelAll();
    }
}

bool OutgoingIqManager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() != u"iq") {
        return false;
    }

    // only accept "result" and "error" types
    const auto iqType = stanza.attribute(u"type"_s);
    if (iqType != u"result" && iqType != u"error") {
        return false;
    }

    const auto id = stanza.attribute(u"id"_s);
    auto itr = m_requests.find(id);
    if (itr == m_requests.end()) {
        return false;
    }

    auto &promise = itr->second.interface;
    const auto &expectedFrom = itr->second.jid;

    // Check that the sender of the response matches the recipient of the request.
    // Stanzas coming from the server on behalf of the user's account must have no "from"
    // attribute or have it set to the user's bare JID.
    // If 'from' is empty, the IQ has been sent by the server. In this case we don't need to
    // do the check as we trust the server anyways.
    if (auto from = stanza.attribute(u"from"_s); !from.isEmpty() && from != expectedFrom) {
        warning(u"Ignored received IQ response to request '%1' because of wrong sender '%2' instead of expected sender '%3'"_s
                    .arg(id, from, expectedFrom));
        return false;
    }

    // report IQ errors as QXmppError (this makes it impossible to parse the full error IQ,
    // but that is okay for now)
    if (iqType == u"error") {
        QXmppIq iq;
        iq.parse(stanza);
        if (auto err = iq.errorOptional()) {
            // report stanza error
            promise.finish(QXmppError { err->text(), *err });
        } else {
            // this shouldn't happen (no <error/> element in IQ of type error)
            using Err = QXmppStanza::Error;
            promise.finish(QXmppError { u"IQ error"_s, Err(Err::Cancel, Err::UndefinedCondition) });
        }
    } else {
        // report stanza element for parsing
        promise.finish(stanza);
    }

    m_requests.erase(itr);
    return true;
}

void OutgoingIqManager::warning(const QString &message)
{
    Q_EMIT l->logMessage(QXmppLogger::WarningMessage, message);
}

C2sStreamManager::C2sStreamManager(QXmppOutgoingClient *q)
    : q(q)
{
}

HandleElementResult C2sStreamManager::handleElement(const QDomElement &el)
{
    // resume
    if (std::holds_alternative<ResumeRequest>(m_request)) {
        auto request = std::get<ResumeRequest>(std::move(m_request));
        m_request = {};

        if (auto resumed = SmResumed::fromDom(el)) {
            onResumed(*resumed);
            request.p.finish();
            return Finished;
        }

        if (auto failed = SmFailed::fromDom(el)) {
            onResumeFailed(*failed);
            request.p.finish();
            return Finished;
        }
    }
    // enable
    if (std::holds_alternative<EnableRequest>(m_request)) {
        auto request = std::get<EnableRequest>(std::move(m_request));
        m_request = {};

        if (auto enabled = SmEnabled::fromDom(el)) {
            onEnabled(*enabled);
            request.p.finish();
            return Finished;
        }

        if (auto failed = SmFailed::fromDom(el)) {
            onEnableFailed(*failed);
            request.p.finish();
            return Finished;
        }
    }

    return Rejected;
}

void C2sStreamManager::onStreamStart()
{
    m_streamResumed = false;
    m_enabled = false;
    m_request = {};
}

void C2sStreamManager::onStreamFeatures(const QXmppStreamFeatures &features)
{
    m_smAvailable = features.streamManagementMode() != QXmppStreamFeatures::Disabled;
}

void C2sStreamManager::onStreamClosed()
{
    m_canResume = false;
}

void C2sStreamManager::onSasl2Authenticate(Sasl2::Authenticate &auth, const Sasl2::StreamFeature &feature)
{
    if (feature.streamResumptionAvailable && !m_enabled && m_canResume) {
        auto lastAckNumber = q->streamAckManager().lastIncomingSequenceNumber();
        auth.smResume = SmResume { lastAckNumber, m_smId };
    }
}

void C2sStreamManager::onSasl2Success(const Sasl2::Success &success)
{
    if (success.smResumed) {
        onResumed(*success.smResumed);
    }
    if (success.smFailed) {
        onResumeFailed(*success.smFailed);
    }
}

void C2sStreamManager::onBind2Request(Bind2Request &request, const std::vector<QString> &bind2Features)
{
    if (contains(bind2Features, ns_stream_management)) {
        request.smEnable = SmEnable { true };
    }
}

void C2sStreamManager::onBind2Bound(const Bind2Bound &bound)
{
    if (bound.smEnabled) {
        onEnabled(*bound.smEnabled);
    }
    if (bound.smFailed) {
        onEnableFailed(*bound.smFailed);
    }
}

QXmppTask<void> C2sStreamManager::requestResume()
{
    Q_ASSERT(std::holds_alternative<NoRequest>(m_request));
    m_request = ResumeRequest();

    auto lastAckNumber = q->streamAckManager().lastIncomingSequenceNumber();
    q->xmppSocket().sendData(serializeXml(SmResume { lastAckNumber, m_smId }));

    return std::get<ResumeRequest>(m_request).p.task();
}

QXmppTask<void> C2sStreamManager::requestEnable()
{
    Q_ASSERT(std::holds_alternative<NoRequest>(m_request));
    m_request = EnableRequest();

    q->xmppSocket().sendData(serializeXml(SmEnable { true }));

    return std::get<EnableRequest>(m_request).p.task();
}

void C2sStreamManager::onEnabled(const SmEnabled &enabled)
{
    // Called whenever stream management is enabled, either by requestEnable() or by onBind2Bound()
    q->debug(u"Stream management enabled"_s);
    m_smId = enabled.id;
    m_canResume = enabled.resume;
    if (enabled.resume && !enabled.location.isEmpty()) {
        setResumeAddress(enabled.location);
    }

    m_enabled = true;
    q->streamAckManager().enableStreamManagement(true);
}

void C2sStreamManager::onEnableFailed(const SmFailed &)
{
    q->warning(u"Failed to enable stream management"_s);
}

void C2sStreamManager::onResumed(const SmResumed &resumed)
{
    q->debug(u"Stream resumed"_s);
    q->streamAckManager().setAcknowledgedSequenceNumber(resumed.h);
    m_streamResumed = true;
    m_enabled = true;
    q->streamAckManager().enableStreamManagement(false);
}

void C2sStreamManager::onResumeFailed(const SmFailed &)
{
    q->debug(u"Stream resumption failed"_s);
}

bool C2sStreamManager::setResumeAddress(const QString &address)
{
    if (const auto location = parseHostAddress(address);
        !location.first.isEmpty()) {
        m_resumeHost = location.first;

        if (location.second > 0) {
            m_resumePort = location.second;
        } else {
            m_resumePort = XMPP_DEFAULT_PORT;
        }
        return true;
    }

    m_resumeHost.clear();
    m_resumePort = 0;
    return false;
}

void CarbonManager::onBind2Request(Bind2Request &request, const std::vector<QString> &bind2Features)
{
    request.carbonsEnable = m_enableViaBind2 && contains(bind2Features, ns_carbons);
    m_requested = request.carbonsEnable;
}

void CarbonManager::onSessionOpened(const SessionBegin &session)
{
    // reset state for new streams
    if (!session.smResumed) {
        m_enabled = false;
    }
    // set if enabled via bind2
    if (session.bind2Used) {
        m_enabled = m_requested;
    }
}

CsiManager::CsiManager(QXmppOutgoingClient *client)
    : m_client(client)
{
}

void CsiManager::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        sendState();
    }
}

void CsiManager::onSessionOpened(const SessionBegin &session)
{
    if (m_client->c2sStreamManager().streamResumed()) {
        // stream could be resumed, previous state is still correct
        // if sending of the previous state did not succeed, resend
        if (!m_synced) {
            sendState();
        }
    } else {
        // new stream starts with Active if not set to Inactive via bind2
        auto initialState = session.bind2Used && m_bind2InactiveSet ? Inactive : Active;

        if (m_state == initialState) {
            m_synced = true;
        } else {
            // update state
            sendState();
        }
    }
}

void CsiManager::onStreamFeatures(const QXmppStreamFeatures &features)
{
    m_featureAvailable = features.clientStateIndicationMode() == QXmppStreamFeatures::Enabled;
}

void CsiManager::onBind2Request(Bind2Request &request, const std::vector<QString> &bind2Features)
{
    // The state in the bind2 request is only used for new streams.
    // If the stream is resumed, the bind2 request is ignored.

    request.csiInactive = (m_state == Inactive && contains(bind2Features, ns_csi));
    m_bind2InactiveSet = request.csiInactive;
}

void CsiManager::sendState()
{
    if (m_client->isAuthenticated() && m_featureAvailable) {
        auto xml = m_state == Active ? serializeXml(CsiActive()) : serializeXml(CsiInactive());
        m_synced = m_client->xmppSocket().sendData(xml);
    } else {
        m_synced = false;
    }
}

}  // namespace QXmpp::Private
