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

using DnsRecordsResult = std::variant<QList<QDnsServiceRecord>, QXmppError>;

static QXmppTask<DnsRecordsResult> lookupXmppClientRecords(const QString &domain, QObject *context)
{
    QXmppPromise<DnsRecordsResult> p;
    auto task = p.task();

    auto *dns = new QDnsLookup(QDnsLookup::SRV, u"_xmpp-client._tcp." + domain, context);
    QObject::connect(dns, &QDnsLookup::finished, context, [dns, p = std::move(p)]() mutable {
        if (auto error = dns->error(); error != QDnsLookup::NoError) {
            p.finish(QXmppError { dns->errorString(), error });
        } else {
            p.finish(dns->serviceRecords());
        }
    });

    dns->lookup();
    return task;
}

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate(QXmppOutgoingClient *qq)
    : socket(qq),
      streamAckManager(socket),
      iqManager(qq, streamAckManager),
      manager(qq),
      c2sStreamManager(qq),
      csiManager(qq),
      pingManager(qq),
      q(qq)
{
}

void QXmppOutgoingClientPrivate::connectToHost(const QString &host, quint16 port)
{
    q->info(QStringLiteral("Connecting to %1:%2").arg(host, QString::number(port)));

    // override CA certificates if requested
    if (!config.caCertificates().isEmpty()) {
        QSslConfiguration newSslConfig;
        newSslConfig.setCaCertificates(config.caCertificates());
        q->socket()->setSslConfiguration(newSslConfig);
    }

    // respect proxy
    q->socket()->setProxy(config.networkProxy());

    // set the name the SSL certificate should match
    q->socket()->setPeerVerifyName(config.domain());

    // connect to host
    const QXmppConfiguration::StreamSecurityMode localSecurity = q->configuration().streamSecurityMode();
    if (localSecurity == QXmppConfiguration::LegacySSL) {
        if (!q->socket()->supportsSsl()) {
            q->warning(QStringLiteral("Not connecting as legacy SSL was requested, but SSL support is not available"));
            return;
        }
        q->socket()->connectToHostEncrypted(host, port);
    } else {
        q->socket()->connectToHost(host, port);
    }
}

void QXmppOutgoingClientPrivate::connectToNextDNSHost()
{
    auto curIdx = nextSrvRecordIdx++;
    connectToHost(srvRecords.at(curIdx).target(), srvRecords.at(curIdx).port());
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
        d->connectToHost(host, port);
        return;
    }

    // if an explicit host was provided, connect to it
    if (!d->config.host().isEmpty() && d->config.port()) {
        d->connectToHost(d->config.host(), d->config.port());
        return;
    }

    // otherwise, lookup server
    const auto domain = configuration().domain();
    debug(QStringLiteral("Looking up service records for domain %1").arg(domain));
    lookupXmppClientRecords(domain, this).then(this, [this, domain](auto result) {
        if (auto error = std::get_if<QXmppError>(&result)) {
            warning(QStringLiteral("Lookup for domain %1 failed: %2")
                        .arg(domain, error->description));

            // as a fallback, use domain as the host name
            d->connectToHost(d->config.domain(), d->config.port());
            return;
        }

        d->srvRecords = std::get<QList<QDnsServiceRecord>>(std::move(result));
        d->nextSrvRecordIdx = 0;

        if (d->srvRecords.isEmpty()) {
            warning(QStringLiteral("'%1' has no xmpp-client service records.").arg(domain));

            // as a fallback, use domain as the host name
            d->connectToHost(d->config.domain(), d->config.port());
            return;
        }

        d->connectToNextDNSHost();
    });
}

///
/// Disconnects from the server and resets the stream management state.
///
/// \since QXmpp 1.0
///
void QXmppOutgoingClient::disconnectFromHost()
{
    d->c2sStreamManager.onDisconnecting();
    d->streamAckManager.handleDisconnect();
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
    debug(QStringLiteral("Socket disconnected"));
    d->isAuthenticated = false;
    if (d->redirect) {
        d->connectToHost(d->redirect->host, d->redirect->port);
        d->redirect.reset();
    } else {
        closeSession();
    }
}

void QXmppOutgoingClient::socketSslErrors(const QList<QSslError> &errors)
{
    // log errors
    warning(QStringLiteral("SSL errors"));
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
    d->manager = Sasl2Manager(&d->socket);

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
    d->c2sStreamManager.onSasl2Authenticate(sasl2Request, sasl2Feature);

    // start authentication
    auto &sasl2 = std::get<Sasl2Manager>(d->manager);
    sasl2.authenticate(std::move(sasl2Request), d->config, sasl2Feature, this).then(this, [this](auto result) {
        if (auto success = std::get_if<Sasl2::Success>(&result)) {
            debug(QStringLiteral("Authenticated"));
            d->isAuthenticated = true;
            d->config.setJid(success->authorizationIdentifier);
            d->bind2Bound = std::move(success->bound);

            // extensions
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
    d->manager = NonSaslAuthManager(&d->socket);
    std::get<NonSaslAuthManager>(d->manager).queryOptions(d->streamFrom, d->config.user()).then(this, [this](auto result) {
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
                warning(QStringLiteral("No supported Non-SASL Authentication mechanism available"));
                disconnectFromHost();
                return;
            }

            auto task = std::get<NonSaslAuthManager>(d->manager).authenticate(plainText, d->config.user(), d->config.password(), d->config.resource(), d->streamId);
            task.then(this, [this](auto result) {
                if (std::holds_alternative<Success>(result)) {
                    // successful Non-SASL Authentication
                    debug(QStringLiteral("Authenticated (Non-SASL)"));
                    d->isAuthenticated = true;

                    // xmpp connection made
                    openSession();
                } else {
                    // TODO: errors: should trigger error signal
                    auto &error = std::get<QXmppError>(result);
                    warning(QStringLiteral("Could not authenticate using Non-SASL Authentication: ") + error.description);
                    disconnectFromHost();
                    return;
                }
            });
        } else {
            // TODO: errors: should trigger error signal
            auto &error = std::get<QXmppError>(result);
            warning(QStringLiteral("Couldn't list Non-SASL Authentication mechanisms: ") + error.description);
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::startSmResume()
{
    d->manager = &d->c2sStreamManager;
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
    d->manager = &d->c2sStreamManager;
    d->c2sStreamManager.requestEnable().then(this, [this] {
        // enabling of stream management may or may not have succeeded
        // we are connected now
        openSession();
    });
}

void QXmppOutgoingClient::startResourceBinding()
{
    d->manager = BindManager(&d->socket);
    std::get<BindManager>(d->manager).bindAddress(d->config.resource()).then(this, [this](BindManager::Result r) {
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
            setError(QStringLiteral("Resource binding failed: ") + protocolError->text,
                     StreamError::UndefinedCondition);
            disconnectFromHost();
        } else if (auto *stanzaError = std::get_if<QXmppStanza::Error>(&r)) {
            QString text = QStringLiteral("Resource binding failed: ") + stanzaError->text();
            setError(text, BindError { *stanzaError });
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::openSession()
{
    info(QStringLiteral("Session established"));
    Q_ASSERT(!d->sessionStarted);
    d->sessionStarted = true;

    SessionBegin session {
        d->c2sStreamManager.enabled(),
        d->c2sStreamManager.streamResumed(),
        d->bind2Bound.has_value(),
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
        (d->srvRecords.count() > d->nextSrvRecordIdx)) {
        // some network error occurred during startup -> try next available SRV record server
        d->connectToNextDNSHost();
    } else {
        setError(d->socket.socket()->errorString(), socketError);
    }
}

void QXmppOutgoingClient::handleStart()
{
    d->streamAckManager.handleStart();

    // reset stream information
    d->streamId.clear();
    d->streamFrom.clear();
    d->streamVersion.clear();

    // reset active manager (e.g. authentication)
    d->manager = this;

    d->c2sStreamManager.onStreamStart();

    // start stream
    d->socket.sendData(serializeXml(StreamOpen {
        d->config.domain(),
        d->config.user().isEmpty() ? QString() : d->config.jidBare(),
        ns_client,
    }));
}

void QXmppOutgoingClient::handleStream(const QDomElement &streamElement)
{
    if (d->streamId.isEmpty()) {
        d->streamId = streamElement.attribute(QStringLiteral("id"));
    }
    if (d->streamFrom.isEmpty()) {
        d->streamFrom = streamElement.attribute(QStringLiteral("from"));
    }
    if (d->streamVersion.isEmpty()) {
        d->streamVersion = streamElement.attribute(QStringLiteral("version"));

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

    auto index = d->manager.index();

    switch (visit(overloaded {
                      [&](auto *manager) { return manager->handleElement(nodeRecv); },
                      [&](auto &manager) { return manager.handleElement(nodeRecv); },
                  },
                  d->manager)) {
    case Accepted:
        return;
    case Rejected:
        setError(QStringLiteral("Unexpected element received."), StreamError::UndefinedCondition);
        disconnectFromHost();
        return;
    case Finished:
        // if the job is done, set OutgoingClient, but do not override a continuation job
        if (d->manager.index() == index) {
            d->manager = this;
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

        // handle authentication
        const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
        const bool saslAvailable = !features.authMechanisms().isEmpty();

        if (features.sasl2Feature().has_value() && d->config.useSasl2Authentication()) {
            startSasl2Auth(features.sasl2Feature().value());
            return Accepted;
        } else if (saslAvailable && configuration().useSASLAuthentication()) {
            d->manager = SaslManager(&d->socket);
            std::get<SaslManager>(d->manager).authenticate(d->config, features.authMechanisms(), this).then(this, [this](auto result) {
                if (std::holds_alternative<Success>(result)) {
                    debug(QStringLiteral("Authenticated"));
                    d->isAuthenticated = true;
                    handleStart();
                } else {
                    auto [text, err] = std::get<SaslManager::AuthError>(std::move(result));
                    setError(text, std::move(err));
                    disconnectFromHost();
                }
            });
            return Accepted;
        } else if (nonSaslAvailable && configuration().useNonSASLAuthentication()) {
            startNonSaslAuth();
            return Accepted;
        }

        // store which features are available
        d->bindModeAvailable = (features.bindMode() != QXmppStreamFeatures::Disabled);
        d->c2sStreamManager.onStreamFeatures(features);
        d->csiManager.onStreamFeatures(features);

        // check whether the stream can be resumed
        if (d->c2sStreamManager.canRequestResume()) {
            startSmResume();
            return Accepted;
        }

        // check whether bind is available
        if (d->bindModeAvailable) {
            startResourceBinding();
            return Accepted;
        }

        // check whether SM is available
        if (d->c2sStreamManager.canRequestEnable()) {
            startSmEnable();
            return Accepted;
        }

        // otherwise we are done
        openSession();
        return Accepted;
    } else if (ns == ns_stream && nodeRecv.tagName() == u"error") {
        visit(
            overloaded {
                [&](StreamErrorElement streamError) {
                    if (auto *redirect = std::get_if<StreamErrorElement::SeeOtherHost>(&streamError.condition)) {
                        d->redirect = std::move(*redirect);

                        // only disconnect socket (so stream mangement can resume state is not reset)
                        d->socket.disconnectFromHost();
                        debug(QStringLiteral("Received redirect to '%1:%2'")
                                  .arg(redirect->host, redirect->port));
                    } else {
                        auto condition = std::get<StreamError>(streamError.condition);
                        auto text = QStringLiteral("Received stream error (%1): %2")
                                        .arg(StreamErrorElement::streamErrorToString(condition), streamError.text);

                        setError(text, condition);
                    }
                },
                [&](QXmppError &&err) {
                    // invalid stream error element received
                    setError(QStringLiteral("Received invalid stream error (%1)").arg(err.description),
                             StreamError::UndefinedCondition);
                },
            },
            StreamErrorElement::fromDom(nodeRecv));
    } else if (ns == ns_client) {
        if (nodeRecv.tagName() == u"iq") {
            QString type = nodeRecv.attribute(QStringLiteral("type"));
            if (type.isEmpty()) {
                warning(QStringLiteral("QXmppStream: iq type can't be empty"));
            }

            if (!d->pingManager.handleIq(nodeRecv)) {
                QXmppIq iqPacket;
                iqPacket.parse(nodeRecv);

                // if we didn't understant the iq, reply with error
                // except for "result" and "error" iqs
                if (type != u"result" && type != u"error") {
                    QXmppIq iq(QXmppIq::Error);
                    iq.setId(iqPacket.id());
                    iq.setTo(iqPacket.from());
                    QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                                             QXmppStanza::Error::FeatureNotImplemented);
                    iq.setError(error);
                    d->streamAckManager.send(iq);
                } else {
                    Q_EMIT iqReceived(iqPacket);
                }
            }
        } else if (nodeRecv.tagName() == u"presence") {
            QXmppPresence presence;
            presence.parse(nodeRecv);

            // emit presence
            Q_EMIT presenceReceived(presence);
        } else if (nodeRecv.tagName() == u"message") {
            QXmppMessage message;
            message.parse(nodeRecv);

            // emit message
            Q_EMIT messageReceived(message);
        } else {
            return Rejected;
        }
    } else {
        return Rejected;
    }
    return Accepted;
}

void QXmppOutgoingClient::throwKeepAliveError()
{
    setError(QStringLiteral("Ping timeout"), TimeoutError());
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
                return ProtocolError { QStringLiteral("Server did not return JID upon resource binding.") };
            }

            static const QRegularExpression jidRegex(QStringLiteral("^([^@/]+)@([^@/]+)/(.+)$"));
            if (const auto match = jidRegex.match(iq.jid()); match.hasMatch()) {
                return BoundAddress {
                    match.captured(1),
                    match.captured(2),
                    match.captured(3),
                };
            }

            return ProtocolError { QStringLiteral("Bind IQ received with invalid JID") };
        }

        return iq.error();
    };

    if (QXmppBindIq::isBindIq(el) && el.attribute(QStringLiteral("id")) == m_iqId) {
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

        auto iqType = el.attribute(QStringLiteral("type"));
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

        const auto id = el.attribute(QStringLiteral("id"));
        const auto type = el.attribute(QStringLiteral("type"));

        if (id == query.id) {
            if (type == u"result") {
                query.p.finish(Success());
            } else if (type == u"error") {
                QXmppIq iq;
                iq.parse(el);

                query.p.finish(QXmppError { iq.error().text(), iq.error() });
            } else {
                query.p.finish(QXmppError { QStringLiteral("Received unexpected IQ response."), {} });
            }
        } else {
            query.p.finish(QXmppError { QStringLiteral("Received IQ response with wrong ID."), {} });
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

bool PingManager::handleIq(const QDomElement &el)
{
    if (QXmppPingIq::isPingIq(el)) {
        QXmppPingIq req;
        req.parse(el);

        QXmppIq iq(QXmppIq::Result);
        iq.setId(req.id());
        iq.setTo(req.from());
        q->streamAckManager().send(iq);
        return true;
    }
    return false;
}

void PingManager::sendPing()
{
    // send ping packet
    QXmppPingIq ping;
    ping.setTo(q->configuration().domain());
    q->streamAckManager().send(ping);

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
        warning(QStringLiteral("QXmppStream::sendIq() error: ID is empty. Using random ID."));
        iq.setId(QXmppUtils::generateStanzaUuid());
    }
    if (hasId(iq.id())) {
        warning(QStringLiteral("QXmppStream::sendIq() error:"
                               "The IQ's ID (\"%1\") is already in use. Using random ID.")
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
            QXmppError { QStringLiteral("Invalid IQ id: empty or in use."),
                         SendError::Disconnected });
    }

    if (to.isEmpty()) {
        return makeReadyTask<IqResult>(
            QXmppError { QStringLiteral("The 'to' address must be set so the stream can match the response."),
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
            QStringLiteral("IQ has been cancelled."),
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
    const auto iqType = stanza.attribute(QStringLiteral("type"));
    if (iqType != u"result" && iqType != u"error") {
        return false;
    }

    const auto id = stanza.attribute(QStringLiteral("id"));
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
    if (auto from = stanza.attribute(QStringLiteral("from")); !from.isEmpty() && from != expectedFrom) {
        warning(QStringLiteral("Ignored received IQ response to request '%1' because of wrong sender '%2' instead of expected sender '%3'")
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
            promise.finish(QXmppError { QStringLiteral("IQ error"), Err(Err::Cancel, Err::UndefinedCondition) });
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

void C2sStreamManager::onDisconnecting()
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
    q->debug(QStringLiteral("Stream management enabled"));
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
    q->warning(QStringLiteral("Failed to enable stream management"));
}

void C2sStreamManager::onResumed(const SmResumed &resumed)
{
    q->debug(QStringLiteral("Stream resumed"));
    q->streamAckManager().setAcknowledgedSequenceNumber(resumed.h);
    m_streamResumed = true;
    m_enabled = true;
    q->streamAckManager().enableStreamManagement(false);
}

void C2sStreamManager::onResumeFailed(const SmFailed &)
{
    q->debug(QStringLiteral("Stream resumption failed"));
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
