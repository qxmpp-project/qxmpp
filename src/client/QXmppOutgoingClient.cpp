// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutgoingClient.h"

#include "QXmppConfiguration.h"
#include "QXmppConstants_p.h"
#include "QXmppIq.h"
#include "QXmppMessage.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppPacket_p.h"
#include "QXmppPresence.h"
#include "QXmppPromise.h"
#include "QXmppSasl_p.h"
#include "QXmppStreamFeatures.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <QCryptographicHash>
#include <QDnsLookup>
#include <QFuture>
#include <QNetworkProxy>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QStringBuilder>
#include <QUrl>

// IQ types
#include "QXmppBindIq.h"
#include "QXmppPingIq.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QHostAddress>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTimer>
#include <QXmlStreamWriter>

using namespace QXmpp;
using namespace QXmpp::Private;

namespace QXmpp::Private {

using DnsRecordsResult = std::variant<QList<QDnsServiceRecord>, QXmppError>;

QXmppTask<DnsRecordsResult> lookupXmppClientRecords(const QString &domain, QObject *context)
{
    QXmppPromise<DnsRecordsResult> p;
    auto task = p.task();

    auto *dns = new QDnsLookup(QDnsLookup::SRV, u"_xmpp-client._tcp." % domain, context);
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

struct BoundAddress {
    QString user;
    QString domain;
    QString resource;
};

// Resource Binding
class BindManager
{
public:
    using Result = std::variant<BoundAddress, QXmppError>;

    explicit BindManager(XmppSocket &socket) : m_socket(socket) { }

    void reset();
    QXmppTask<Result> bindAddress(const QString &resource);
    bool handleElement(const QDomElement &el);

private:
    XmppSocket &m_socket;
    QString m_iqId;
    std::optional<QXmppPromise<Result>> m_promise;
};

struct NonSaslAuthOptions {
    bool plain;
    bool digest;
};

// Authentication using Non-SASL auth
class NonSaslAuthManager
{
public:
    using OptionsResult = std::variant<NonSaslAuthOptions, QXmppError>;
    using AuthResult = std::variant<Success, QXmppError>;

    explicit NonSaslAuthManager(XmppSocket &socket) : m_socket(socket) { }

    void reset();
    QXmppTask<OptionsResult> queryOptions(const QString &streamFrom, const QString &username);
    QXmppTask<AuthResult> authenticate(bool plainText, const QString &username, const QString &password, const QString &resource, const QString &streamId);
    bool handleElement(const QDomElement &el);

private:
    struct NoQuery {
    };
    struct OptionsQuery {
        QXmppPromise<OptionsResult> p;
    };
    struct AuthQuery {
        QXmppPromise<AuthResult> p;
        QString id;
    };

    XmppSocket &m_socket;
    std::variant<NoQuery, OptionsQuery, AuthQuery> m_query;
};

// XEP-0199: XMPP Ping
class PingManager
{
public:
    explicit PingManager(QXmppOutgoingClient *q);

    void onDataReceived();
    bool handleIq(const QDomElement &el);

private:
    void sendPing();

    QXmppOutgoingClient *q;
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

}  // namespace QXmpp::Private

class QXmppOutgoingClientPrivate
{
public:
    explicit QXmppOutgoingClientPrivate(QXmppOutgoingClient *q);
    void connectToHost(const QString &host, quint16 port);
    void connectToNextDNSHost();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QXmppStanza::Error::Condition xmppStreamError;

    // Core stream
    XmppSocket socket;
    StreamAckManager streamAckManager;
    OutgoingIqManager iqManager;

    // DNS
    QList<QDnsServiceRecord> srvRecords;
    int nextSrvRecordIdx;

    // Stream
    QString streamId;
    QString streamFrom;
    QString streamVersion;

    // Redirection
    QString redirectHost;
    quint16 redirectPort;

    // Session
    BindManager bindManager;
    bool bindModeAvailable;
    bool sessionStarted;

    // Authentication
    bool isAuthenticated;
    NonSaslAuthManager nonSaslAuthManager;
    std::unique_ptr<QXmppSaslClient> saslClient;

    // Client State Indication
    bool clientStateIndicationEnabled;

    C2sStreamManager c2sStreamManager;
    PingManager pingManager;

private:
    QXmppOutgoingClient *q;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate(QXmppOutgoingClient *qq)
    : socket(qq),
      streamAckManager(socket),
      iqManager(qq, streamAckManager),
      nextSrvRecordIdx(0),
      redirectPort(0),
      bindManager(socket),
      bindModeAvailable(false),
      sessionStarted(false),
      isAuthenticated(false),
      nonSaslAuthManager(socket),
      clientStateIndicationEnabled(false),
      c2sStreamManager(qq),
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
    connect(&d->socket, &XmppSocket::stanzaReceived, this, &QXmppOutgoingClient::handleStanza);
    connect(&d->socket, &XmppSocket::streamReceived, this, &QXmppOutgoingClient::handleStream);
    connect(&d->socket, &XmppSocket::streamClosed, this, &QXmppOutgoingClient::disconnectFromHost);

    // IQ response handling
    connect(this, &QXmppOutgoingClient::connected, this, [=]() {
        if (!d->c2sStreamManager.streamResumed()) {
            // we can't expect a response because this is a new stream
            iqManager().cancelAll();
        }
    });
    connect(this, &QXmppOutgoingClient::disconnected, this, [=]() {
        if (!d->c2sStreamManager.canResume()) {
            // this stream can't be resumed; we can cancel all ongoing IQs
            iqManager().cancelAll();
        }
    });
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
C2sStreamManager &QXmppOutgoingClient::c2sStreamManager()
{
    return d->c2sStreamManager;
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
    d->bindManager.reset();
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
/// Returns true if client state indication (xep-0352) is supported by the server
///
/// \since QXmpp 1.0
///
bool QXmppOutgoingClient::isClientStateIndicationEnabled() const
{
    return d->clientStateIndicationEnabled;
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
    if (!d->redirectHost.isEmpty() && d->redirectPort > 0) {
        d->connectToHost(d->redirectHost, d->redirectPort);
        d->redirectHost = QString();
        d->redirectPort = 0;
    } else {
        Q_EMIT disconnected();
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

void QXmppOutgoingClient::startNonSaslAuth()
{
    d->nonSaslAuthManager.queryOptions(d->streamFrom, d->config.user()).then(this, [this](auto result) {
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

            auto task = d->nonSaslAuthManager.authenticate(plainText, d->config.user(), d->config.password(), d->config.resource(), d->streamId);
            task.then(this, [this](auto result) {
                if (std::holds_alternative<Success>(result)) {
                    // successful Non-SASL Authentication
                    debug(QStringLiteral("Authenticated (Non-SASL)"));
                    d->isAuthenticated = true;

                    // xmpp connection made
                    d->sessionStarted = true;
                    Q_EMIT connected();
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

void QXmppOutgoingClient::startResourceBinding()
{
    d->bindManager.bindAddress(d->config.resource()).then(this, [this](BindManager::Result r) {
        if (auto *addr = std::get_if<BoundAddress>(&r)) {
            d->config.setUser(addr->user);
            d->config.setDomain(addr->domain);
            d->config.setResource(addr->resource);

            d->sessionStarted = true;

            if (d->c2sStreamManager.canRequestEnable()) {
                d->c2sStreamManager.requestEnable();
            } else {
                // we are connected now
                Q_EMIT connected();
            }
        } else {
            auto err = std::get<QXmppError>(std::move(r));

            if (auto stanzaError = err.value<QXmppStanza::Error>()) {
                d->xmppStreamError = stanzaError->condition();
            } else {
                d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
            }

            Q_EMIT error(QXmppClient::XmppStreamError);
            warning(QStringLiteral("Resource binding failed: ") + err.description);
            disconnectFromHost();
        }
    });
}

void QXmppOutgoingClient::onSMResumeFinished()
{
    if (d->c2sStreamManager.streamResumed()) {
        // we are connected now
        Q_EMIT connected();
    } else {
        // check whether bind is available
        if (d->bindModeAvailable) {
            startResourceBinding();
            return;
        }

        // otherwise we are done
        d->sessionStarted = true;
        Q_EMIT connected();
    }
}

void QXmppOutgoingClient::onSMEnableFinished()
{
    // enabling of stream management may or may not have succeeded
    // we are connected now
    Q_EMIT connected();
}

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    if (!d->sessionStarted &&
        (d->srvRecords.count() > d->nextSrvRecordIdx)) {
        // some network error occurred during startup -> try next available SRV record server
        d->connectToNextDNSHost();
    } else {
        Q_EMIT error(QXmppClient::SocketError);
    }
}

void QXmppOutgoingClient::handleStart()
{
    d->streamAckManager.handleStart();

    // reset stream information
    d->streamId.clear();
    d->streamFrom.clear();
    d->streamVersion.clear();

    // reset authentication step
    d->nonSaslAuthManager.reset();
    d->saslClient.reset();

    // reset session information
    d->sessionStarted = false;

    d->c2sStreamManager.onStreamStart();

    // start stream
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain().toUtf8());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    d->socket.sendData(data);
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

void QXmppOutgoingClient::handleStanza(const QDomElement &nodeRecv)
{
    // if we receive any kind of data, stop the timeout timer
    d->pingManager.onDataReceived();

    // handle possible stream management packets first
    if (streamAckManager().handleStanza(nodeRecv) || iqManager().handleStanza(nodeRecv)) {
        return;
    }

    const QString ns = nodeRecv.namespaceURI();

    // give client opportunity to handle stanza
    bool handled = false;
    Q_EMIT elementReceived(nodeRecv, handled);
    if (handled) {
        return;
    }

    if (QXmppStreamFeatures::isStreamFeatures(nodeRecv)) {
        QXmppStreamFeatures features;
        features.parse(nodeRecv);

        if (features.clientStateIndicationMode() == QXmppStreamFeatures::Enabled) {
            d->clientStateIndicationEnabled = true;
        }

        // handle authentication
        const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
        const bool saslAvailable = !features.authMechanisms().isEmpty();
        if (saslAvailable && configuration().useSASLAuthentication()) {
            // supported and preferred SASL auth mechanisms
            const QString preferredMechanism = configuration().saslAuthMechanism();
            QStringList supportedMechanisms = QXmppSaslClient::availableMechanisms();
            if (supportedMechanisms.contains(preferredMechanism)) {
                supportedMechanisms.removeAll(preferredMechanism);
                supportedMechanisms.prepend(preferredMechanism);
            }
            if (configuration().facebookAppId().isEmpty() || configuration().facebookAccessToken().isEmpty()) {
                supportedMechanisms.removeAll(QStringLiteral("X-FACEBOOK-PLATFORM"));
            }
            if (configuration().windowsLiveAccessToken().isEmpty()) {
                supportedMechanisms.removeAll(QStringLiteral("X-MESSENGER-OAUTH2"));
            }
            if (configuration().googleAccessToken().isEmpty()) {
                supportedMechanisms.removeAll(QStringLiteral("X-OAUTH2"));
            }

            // determine SASL Authentication mechanism to use
            QStringList commonMechanisms;
            QString usedMechanism;
            for (const auto &mechanism : std::as_const(supportedMechanisms)) {
                if (features.authMechanisms().contains(mechanism)) {
                    commonMechanisms << mechanism;
                }
            }
            if (commonMechanisms.isEmpty()) {
                warning(QStringLiteral("No supported SASL Authentication mechanism available"));
                disconnectFromHost();
                return;
            } else {
                usedMechanism = commonMechanisms.first();
            }

            d->saslClient.reset(QXmppSaslClient::create(usedMechanism, this));
            if (!d->saslClient) {
                warning(QStringLiteral("SASL mechanism negotiation failed"));
                disconnectFromHost();
                return;
            }
            info(QStringLiteral("SASL mechanism '%1' selected").arg(d->saslClient->mechanism()));
            d->saslClient->setHost(d->config.domain());
            d->saslClient->setServiceType(QStringLiteral("xmpp"));
            if (d->saslClient->mechanism() == u"X-FACEBOOK-PLATFORM") {
                d->saslClient->setUsername(configuration().facebookAppId());
                d->saslClient->setPassword(configuration().facebookAccessToken());
            } else if (d->saslClient->mechanism() == u"X-MESSENGER-OAUTH2") {
                d->saslClient->setPassword(configuration().windowsLiveAccessToken());
            } else if (d->saslClient->mechanism() == u"X-OAUTH2") {
                d->saslClient->setUsername(configuration().user());
                d->saslClient->setPassword(configuration().googleAccessToken());
            } else {
                d->saslClient->setUsername(configuration().user());
                d->saslClient->setPassword(configuration().password());
            }

            // send SASL auth request
            QByteArray response;
            if (!d->saslClient->respond(QByteArray(), response)) {
                warning(QStringLiteral("SASL initial response failed"));
                disconnectFromHost();
                return;
            }
            d->streamAckManager.send(QXmppSaslAuth(d->saslClient->mechanism(), response));
            return;
        } else if (nonSaslAvailable && configuration().useNonSASLAuthentication()) {
            startNonSaslAuth();
            return;
        }

        // store which features are available
        d->bindModeAvailable = (features.bindMode() != QXmppStreamFeatures::Disabled);
        d->c2sStreamManager.onStreamFeatures(features);

        // check whether the stream can be resumed
        if (d->c2sStreamManager.canRequestResume()) {
            d->c2sStreamManager.requestResume();
            return;
        }

        // check whether bind is available
        if (d->bindModeAvailable) {
            startResourceBinding();
            return;
        }

        // otherwise we are done
        d->sessionStarted = true;
        Q_EMIT connected();
    } else if (ns == ns_stream && nodeRecv.tagName() == u"error") {
        // handle redirects
        const auto otherHost = firstChildElement(nodeRecv, u"see-other-host");
        if (!otherHost.isNull()) {
            // try to parse address
            if (auto [host, port] = parseHostAddress(otherHost.text()); !host.isEmpty()) {
                d->redirectHost = host;
                d->redirectPort = port > 0 ? port : 5222;

                // only disconnect socket (so stream mangement can resume state is not reset)
                d->socket.disconnectFromHost();
                return;
            }
        }

        if (!firstChildElement(nodeRecv, u"conflict").isNull()) {
            d->xmppStreamError = QXmppStanza::Error::Conflict;
        } else if (!firstChildElement(nodeRecv, u"not-authorized").isNull()) {
            d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
        } else {
            d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
        }
        Q_EMIT error(QXmppClient::XmppStreamError);
    } else if (ns == ns_sasl) {
        if (!d->saslClient) {
            warning(QStringLiteral("SASL stanza received, but no mechanism selected"));
            return;
        }
        if (nodeRecv.tagName() == u"success") {
            debug(QStringLiteral("Authenticated"));
            d->isAuthenticated = true;
            handleStart();
        } else if (nodeRecv.tagName() == u"challenge") {
            QXmppSaslChallenge challenge;
            challenge.parse(nodeRecv);

            QByteArray response;
            if (d->saslClient->respond(challenge.value(), response)) {
                d->streamAckManager.send(QXmppSaslResponse(response));
            } else {
                warning(QStringLiteral("Could not respond to SASL challenge"));
                disconnectFromHost();
            }
        } else if (nodeRecv.tagName() == u"failure") {
            QXmppSaslFailure failure;
            failure.parse(nodeRecv);

            // RFC3920 defines the error condition as "not-authorized", but
            // some broken servers use "bad-auth" instead. We tolerate this
            // by remapping the error to "not-authorized".
            if (failure.condition() == u"not-authorized" || failure.condition() == u"bad-auth") {
                d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
            } else {
                d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
            }
            Q_EMIT error(QXmppClient::XmppStreamError);

            warning(QStringLiteral("Authentication failure"));
            disconnectFromHost();
        }
    } else if (ns == ns_client) {
        if (nodeRecv.tagName() == u"iq") {
            QString type = nodeRecv.attribute(QStringLiteral("type"));
            if (type.isEmpty()) {
                warning(QStringLiteral("QXmppStream: iq type can't be empty"));
            }

            if (d->bindManager.handleElement(nodeRecv) ||
                d->nonSaslAuthManager.handleElement(nodeRecv) ||
                d->pingManager.handleIq(nodeRecv)) {
                // handled in manager
            } else {
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
        }
    } else {
        d->c2sStreamManager.handleElement(nodeRecv);
    }
}

void QXmppOutgoingClient::throwKeepAliveError()
{
    warning(QStringLiteral("Ping timeout"));
    disconnectFromHost();
    Q_EMIT error(QXmppClient::KeepAliveError);
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
    return d->xmppStreamError;
}

namespace QXmpp::Private {

void BindManager::reset()
{
    m_iqId.clear();
    m_promise.reset();
}

QXmppTask<BindManager::Result> BindManager::bindAddress(const QString &resource)
{
    Q_ASSERT(!m_promise);
    Q_ASSERT(m_iqId.isNull());

    m_promise = QXmppPromise<Result>();

    const auto iq = QXmppBindIq::bindAddressIq(resource);
    m_iqId = iq.id();
    m_socket.sendData(serializeNonza(iq));

    return m_promise->task();
}

bool BindManager::handleElement(const QDomElement &el)
{
    auto process = [](QXmppBindIq &&iq) -> Result {
        if (iq.type() == QXmppIq::Result) {
            if (iq.jid().isEmpty()) {
                return QXmppError { QStringLiteral("Server did not return JID upon resource binding."), {} };
            }

            static const QRegularExpression jidRegex(QStringLiteral("^([^@/]+)@([^@/]+)/(.+)$"));
            if (const auto match = jidRegex.match(iq.jid()); match.hasMatch()) {
                return BoundAddress {
                    match.captured(1),
                    match.captured(2),
                    match.captured(3),
                };
            }

            return QXmppError { QStringLiteral("Bind IQ received with invalid JID"), {} };
        }

        auto error = iq.error();
        return QXmppError { error.text(), error };
    };

    if (QXmppBindIq::isBindIq(el) && el.attribute(QStringLiteral("id")) == m_iqId) {
        Q_ASSERT(m_promise.has_value());

        auto p = std::move(*m_promise);
        reset();

        QXmppBindIq bind;
        bind.parse(el);

        // do not accept other IQ types than result and error
        if (bind.type() != QXmppIq::Result && bind.type() != QXmppIq::Error) {
            return false;
        }

        // report result
        p.finish(process(std::move(bind)));
        return true;
    }
    return false;
}

void NonSaslAuthManager::reset()
{
    m_query = {};
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

    m_socket.sendData(serializeNonza(authQuery));

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

    m_socket.sendData(serializeNonza(authQuery));
    return query.p.task();
}

bool NonSaslAuthManager::handleElement(const QDomElement &el)
{
    if (std::holds_alternative<OptionsQuery>(m_query)) {
        auto query = std::get<OptionsQuery>(std::move(m_query));
        m_query = {};

        if (QXmppNonSASLAuthIq::isNonSASLAuthIq(el) && el.attribute(QStringLiteral("type")) == u"result") {
            auto queryEl = firstChildElement(el, u"query");

            bool digest = !firstChildElement(queryEl, u"digest").isNull();
            bool plain = !firstChildElement(queryEl, u"password").isNull();

            query.p.finish(NonSaslAuthOptions { plain, digest });
        } else {
            QXmppIq iq;
            iq.parse(el);

            query.p.finish(QXmppError { iq.error().text(), iq.error() });
        }
        return true;
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
        return true;
    }
    return false;
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
            pingTimer->setInterval(interval * 1000);
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
        timeoutTimer->setInterval(timeout * 1000);
        timeoutTimer->start();
    }
}

C2sStreamManager::C2sStreamManager(QXmppOutgoingClient *q)
    : q(q)
{
}

bool C2sStreamManager::handleElement(const QDomElement &el)
{
    // enable succeeded
    if (QXmppStreamManagementEnabled::isStreamManagementEnabled(el)) {
        QXmppStreamManagementEnabled streamManagementEnabled;
        streamManagementEnabled.parse(el);

        m_smId = streamManagementEnabled.id();
        m_canResume = streamManagementEnabled.resume();
        if (streamManagementEnabled.resume() && !streamManagementEnabled.location().isEmpty()) {
            setResumeAddress(streamManagementEnabled.location());
        }

        m_enabled = true;
        q->streamAckManager().enableStreamManagement(true);

        q->onSMEnableFinished();
        return true;
    }

    // resume succeeded
    if (QXmppStreamManagementResumed::isStreamManagementResumed(el)) {
        QXmppStreamManagementResumed streamManagementResumed;
        streamManagementResumed.parse(el);
        q->streamAckManager().setAcknowledgedSequenceNumber(streamManagementResumed.h());
        m_isResuming = false;
        m_streamResumed = true;

        m_enabled = true;
        q->streamAckManager().enableStreamManagement(false);

        q->onSMResumeFinished();
        return true;
    }

    // enable/resume failed
    if (QXmppStreamManagementFailed::isStreamManagementFailed(el)) {
        if (m_isResuming) {
            // resuming failed. We can try to bind a resource now.
            m_isResuming = false;

            q->onSMResumeFinished();
        } else {
            q->onSMEnableFinished();
        }
        return true;
    }
    return false;
}

void C2sStreamManager::onStreamStart()
{
    m_streamResumed = false;
    m_enabled = false;
}

void C2sStreamManager::onStreamFeatures(const QXmppStreamFeatures &features)
{
    m_smAvailable = features.streamManagementMode() != QXmppStreamFeatures::Disabled;
}

void C2sStreamManager::onDisconnecting()
{
    m_canResume = false;
}

void C2sStreamManager::requestResume()
{
    m_isResuming = true;

    auto lastAckNumber = q->streamAckManager().lastIncomingSequenceNumber();
    q->xmppSocket().sendData(serializeNonza(QXmppStreamManagementResume(lastAckNumber, m_smId)));
}

void C2sStreamManager::requestEnable()
{
    q->xmppSocket().sendData(serializeNonza(QXmppStreamManagementEnable(true)));
}

bool C2sStreamManager::setResumeAddress(const QString &address)
{
    if (const auto location = parseHostAddress(address);
        !location.first.isEmpty()) {
        m_resumeHost = location.first;

        if (location.second > 0) {
            m_resumePort = location.second;
        } else {
            m_resumePort = 5222;
        }
        return true;
    }

    m_resumeHost.clear();
    m_resumePort = 0;
    return false;
}

}  // namespace QXmpp::Private
