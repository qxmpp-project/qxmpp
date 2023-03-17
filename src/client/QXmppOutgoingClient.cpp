// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutgoingClient.h"

#include "QXmppConfiguration.h"
#include "QXmppConstants_p.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppPresence.h"
#include "QXmppSasl_p.h"
#include "QXmppStreamFeatures.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"

#include <QCryptographicHash>
#include <QDnsLookup>
#include <QFuture>
#include <QNetworkProxy>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QUrl>

// IQ types
#include "QXmppBindIq.h"
#include "QXmppPingIq.h"
#include "QXmppSessionIq.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QHostAddress>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>
#include <QXmlStreamWriter>

class QXmppOutgoingClientPrivate
{
public:
    QXmppOutgoingClientPrivate(QXmppOutgoingClient *q);
    void connectToHost(const QString &host, quint16 port);
    void connectToNextDNSHost();

    void sendNonSASLAuth(bool plaintext);
    void sendNonSASLAuthQuery();
    void sendBind();
    void sendSessionStart();
    void sendStreamManagementEnable();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QXmppStanza::Error::Condition xmppStreamError;

    // DNS
    QDnsLookup dns;
    int nextSrvRecordIdx;

    // Stream
    QString streamId;
    QString streamFrom;
    QString streamVersion;

    // Redirection
    QString redirectHost;
    quint16 redirectPort;

    // Session
    QString bindId;
    QString sessionId;
    bool bindModeAvailable;
    bool sessionAvailable;
    bool sessionStarted;

    // Authentication
    bool isAuthenticated;
    QString nonSASLAuthId;
    QXmppSaslClient *saslClient;

    // Stream Management
    bool streamManagementAvailable;
    QString smId;
    bool canResume;
    bool isResuming;
    QString resumeHost;
    quint16 resumePort;
    bool streamManagementEnabled;
    bool streamResumed;

    // Client State Indication
    bool clientStateIndicationEnabled;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;

private:
    QXmppOutgoingClient *q;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate(QXmppOutgoingClient *qq)
    : nextSrvRecordIdx(0),
      redirectPort(0),
      bindModeAvailable(false),
      sessionAvailable(false),
      sessionStarted(false),
      isAuthenticated(false),
      saslClient(nullptr),
      streamManagementAvailable(false),
      canResume(false),
      isResuming(false),
      resumePort(0),
      streamManagementEnabled(false),
      streamResumed(false),
      clientStateIndicationEnabled(false),
      pingTimer(nullptr),
      timeoutTimer(nullptr),
      q(qq)
{
}

void QXmppOutgoingClientPrivate::connectToHost(const QString &host, quint16 port)
{
    q->info(QString("Connecting to %1:%2").arg(host, QString::number(port)));

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
            q->warning("Not connecting as legacy SSL was requested, but SSL support is not available");
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
    connectToHost(
        dns.serviceRecords().at(curIdx).target(),
        dns.serviceRecords().at(curIdx).port());
}

///
/// Constructs an outgoing client stream.
///
QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppStream(parent),
      d(std::make_unique<QXmppOutgoingClientPrivate>(this))
{
    // initialise socket
    auto *socket = new QSslSocket(this);
    setSocket(socket);

    connect(socket, &QAbstractSocket::disconnected, this, &QXmppOutgoingClient::_q_socketDisconnected);
    connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QXmppOutgoingClient::socketSslErrors);
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppOutgoingClient::socketError);

    // DNS lookups
    connect(&d->dns, &QDnsLookup::finished, this, &QXmppOutgoingClient::_q_dnsLookupFinished);

    // XEP-0199: XMPP Ping
    d->pingTimer = new QTimer(this);
    connect(d->pingTimer, &QTimer::timeout, this, &QXmppOutgoingClient::pingSend);

    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    connect(d->timeoutTimer, &QTimer::timeout, this, &QXmppOutgoingClient::pingTimeout);

    connect(this, &QXmppStream::connected, this, &QXmppOutgoingClient::pingStart);
    connect(this, &QXmppStream::disconnected, this, &QXmppOutgoingClient::pingStop);

    // IQ response handling
    connect(this, &QXmppStream::connected, this, [=]() {
        if (!d->streamResumed) {
            // we can't expect a response because this is a new stream
            cancelOngoingIqs();
        }
    });
    connect(this, &QXmppStream::disconnected, this, [=]() {
        if (!d->canResume) {
            // this stream can't be resumed; we can cancel all ongoing IQs
            cancelOngoingIqs();
        }
    });
}

QXmppOutgoingClient::~QXmppOutgoingClient() = default;

/// Returns a reference to the stream's configuration.

QXmppConfiguration &QXmppOutgoingClient::configuration()
{
    return d->config;
}

/// Attempts to connect to the XMPP server.

void QXmppOutgoingClient::connectToHost()
{
    // if a host for resumption is available, connect to it
    if (d->canResume && !d->resumeHost.isEmpty() && d->resumePort) {
        d->connectToHost(d->resumeHost, d->resumePort);
        return;
    }

    // if an explicit host was provided, connect to it
    if (!d->config.host().isEmpty() && d->config.port()) {
        d->connectToHost(d->config.host(), d->config.port());
        return;
    }

    // otherwise, lookup server
    const QString domain = configuration().domain();
    debug(QString("Looking up server for domain %1").arg(domain));
    d->dns.setName("_xmpp-client._tcp." + domain);
    d->dns.setType(QDnsLookup::SRV);
    d->dns.lookup();
    d->nextSrvRecordIdx = 0;
}

///
/// Disconnects from the server and resets the stream management state.
///
/// \since QXmpp 1.0
///
void QXmppOutgoingClient::disconnectFromHost()
{
    d->canResume = false;
    QXmppStream::disconnectFromHost();
}

void QXmppOutgoingClient::_q_dnsLookupFinished()
{
    if (d->dns.error() == QDnsLookup::NoError &&
        !d->dns.serviceRecords().isEmpty()) {
        // take the first returned record
        d->connectToNextDNSHost();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2")
                    .arg(d->dns.name(), d->dns.errorString()));
        d->connectToHost(d->config.domain(), d->config.port());
    }
}

/// Returns true if authentication has succeeded.

bool QXmppOutgoingClient::isAuthenticated() const
{
    return d->isAuthenticated;
}

/// Returns true if the socket is connected and a session has been started.

bool QXmppOutgoingClient::isConnected() const
{
    return QXmppStream::isConnected() && d->sessionStarted;
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
/// Returns whether Stream Management is currently enabled.
///
/// \since QXmpp 1.4
///
bool QXmppOutgoingClient::isStreamManagementEnabled() const
{
    return d->streamManagementEnabled;
}

///
/// Returns true if the current stream is a successful resumption of a previous
/// stream.
///
/// In case a stream has been resumed, some tasks like fetching the roster again
/// are not required.
///
/// \since QXmpp 1.4
///
bool QXmppOutgoingClient::isStreamResumed() const
{
    return d->streamResumed;
}

///
/// Sends an IQ and reports the response asynchronously.
///
/// It makes sure that the to address is set so the stream can correctly check the reponse's
/// sender.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppStream::IqResult> QXmppOutgoingClient::sendIq(QXmppIq &&iq)
{
    // If 'to' is empty the user's bare JID is meant implicitly (see RFC6120, section 10.3.3.).
    auto to = iq.to();
    return QXmppStream::sendIq(std::move(iq), to.isEmpty() ? d->config.jidBare() : to);
}

void QXmppOutgoingClient::_q_socketDisconnected()
{
    debug("Socket disconnected");
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
    warning("SSL errors");
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

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    if (!d->sessionStarted &&
        (d->dns.serviceRecords().count() > d->nextSrvRecordIdx)) {
        // some network error occurred during startup -> try next available SRV record server
        d->connectToNextDNSHost();
    } else {
        Q_EMIT error(QXmppClient::SocketError);
    }
}

/// \cond
void QXmppOutgoingClient::handleStart()
{
    QXmppStream::handleStart();

    // reset stream information
    d->streamId.clear();
    d->streamFrom.clear();
    d->streamVersion.clear();

    // reset authentication step
    if (d->saslClient) {
        delete d->saslClient;
        d->saslClient = nullptr;
    }

    // reset session information
    d->bindId.clear();
    d->sessionId.clear();
    d->sessionAvailable = false;
    d->sessionStarted = false;

    // reset stream management
    d->streamResumed = false;
    d->streamManagementEnabled = false;

    // start stream
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain().toUtf8());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    sendData(data);
}

void QXmppOutgoingClient::handleStream(const QDomElement &streamElement)
{
    if (d->streamId.isEmpty()) {
        d->streamId = streamElement.attribute("id");
    }
    if (d->streamFrom.isEmpty()) {
        d->streamFrom = streamElement.attribute("from");
    }
    if (d->streamVersion.isEmpty()) {
        d->streamVersion = streamElement.attribute("version");

        // no version specified, signals XMPP Version < 1.0.
        // switch to old auth mechanism if enabled
        if (d->streamVersion.isEmpty() && configuration().useNonSASLAuthentication()) {
            d->sendNonSASLAuthQuery();
        }
    }
}

void QXmppOutgoingClient::handleStanza(const QDomElement &nodeRecv)
{
    // if we receive any kind of data, stop the timeout timer
    d->timeoutTimer->stop();

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
                supportedMechanisms.removeAll("X-FACEBOOK-PLATFORM");
            }
            if (configuration().windowsLiveAccessToken().isEmpty()) {
                supportedMechanisms.removeAll("X-MESSENGER-OAUTH2");
            }
            if (configuration().googleAccessToken().isEmpty()) {
                supportedMechanisms.removeAll("X-OAUTH2");
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
                warning("No supported SASL Authentication mechanism available");
                disconnectFromHost();
                return;
            } else {
                usedMechanism = commonMechanisms.first();
            }

            d->saslClient = QXmppSaslClient::create(usedMechanism, this);
            if (!d->saslClient) {
                warning("SASL mechanism negotiation failed");
                disconnectFromHost();
                return;
            }
            info(QString("SASL mechanism '%1' selected").arg(d->saslClient->mechanism()));
            d->saslClient->setHost(d->config.domain());
            d->saslClient->setServiceType("xmpp");
            if (d->saslClient->mechanism() == "X-FACEBOOK-PLATFORM") {
                d->saslClient->setUsername(configuration().facebookAppId());
                d->saslClient->setPassword(configuration().facebookAccessToken());
            } else if (d->saslClient->mechanism() == "X-MESSENGER-OAUTH2") {
                d->saslClient->setPassword(configuration().windowsLiveAccessToken());
            } else if (d->saslClient->mechanism() == "X-OAUTH2") {
                d->saslClient->setUsername(configuration().user());
                d->saslClient->setPassword(configuration().googleAccessToken());
            } else {
                d->saslClient->setUsername(configuration().user());
                d->saslClient->setPassword(configuration().password());
            }

            // send SASL auth request
            QByteArray response;
            if (!d->saslClient->respond(QByteArray(), response)) {
                warning("SASL initial response failed");
                disconnectFromHost();
                return;
            }
            sendPacket(QXmppSaslAuth(d->saslClient->mechanism(), response));
            return;
        } else if (nonSaslAvailable && configuration().useNonSASLAuthentication()) {
            d->sendNonSASLAuthQuery();
            return;
        }

        // store which features are available
        d->sessionAvailable = (features.sessionMode() != QXmppStreamFeatures::Disabled);
        d->bindModeAvailable = (features.bindMode() != QXmppStreamFeatures::Disabled);
        d->streamManagementAvailable = (features.streamManagementMode() != QXmppStreamFeatures::Disabled);

        // check whether the stream can be resumed
        if (d->streamManagementAvailable && d->canResume) {
            d->isResuming = true;
            QXmppStreamManagementResume streamManagementResume(lastIncomingSequenceNumber(), d->smId);
            QByteArray data;
            QXmlStreamWriter xmlStream(&data);
            streamManagementResume.toXml(&xmlStream);
            sendData(data);
            return;
        }

        // check whether bind is available
        if (d->bindModeAvailable) {
            d->sendBind();
            return;
        }

        // check whether session is available
        if (d->sessionAvailable) {
            d->sendSessionStart();
            return;
        }

        // otherwise we are done
        d->sessionStarted = true;
        Q_EMIT connected();
    } else if (ns == ns_stream && nodeRecv.tagName() == "error") {
        // handle redirects
        const auto otherHost = nodeRecv.firstChildElement("see-other-host");
        if (!otherHost.isNull() && setResumeAddress(otherHost.text())) {
            QXmppStream::disconnectFromHost();
            return;
        }

        if (!nodeRecv.firstChildElement("conflict").isNull()) {
            d->xmppStreamError = QXmppStanza::Error::Conflict;
        } else if (!nodeRecv.firstChildElement("not-authorized").isNull()) {
            d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
        } else {
            d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
        }
        Q_EMIT error(QXmppClient::XmppStreamError);
    } else if (ns == ns_sasl) {
        if (!d->saslClient) {
            warning("SASL stanza received, but no mechanism selected");
            return;
        }
        if (nodeRecv.tagName() == "success") {
            debug("Authenticated");
            d->isAuthenticated = true;
            handleStart();
        } else if (nodeRecv.tagName() == "challenge") {
            QXmppSaslChallenge challenge;
            challenge.parse(nodeRecv);

            QByteArray response;
            if (d->saslClient->respond(challenge.value(), response)) {
                sendPacket(QXmppSaslResponse(response));
            } else {
                warning("Could not respond to SASL challenge");
                disconnectFromHost();
            }
        } else if (nodeRecv.tagName() == "failure") {
            QXmppSaslFailure failure;
            failure.parse(nodeRecv);

            // RFC3920 defines the error condition as "not-authorized", but
            // some broken servers use "bad-auth" instead. We tolerate this
            // by remapping the error to "not-authorized".
            if (failure.condition() == "not-authorized" || failure.condition() == "bad-auth") {
                d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
            } else {
                d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
            }
            Q_EMIT error(QXmppClient::XmppStreamError);

            warning("Authentication failure");
            disconnectFromHost();
        }
    } else if (ns == ns_client) {

        if (nodeRecv.tagName() == "iq") {
            QDomElement element = nodeRecv.firstChildElement();
            QString id = nodeRecv.attribute("id");
            QString type = nodeRecv.attribute("type");
            if (type.isEmpty()) {
                warning("QXmppStream: iq type can't be empty");
            }

            if (id == d->sessionId) {
                QXmppSessionIq session;
                session.parse(nodeRecv);
                d->sessionStarted = true;

                if (d->streamManagementAvailable) {
                    d->sendStreamManagementEnable();
                } else {
                    // we are connected now
                    Q_EMIT connected();
                }
            } else if (QXmppBindIq::isBindIq(nodeRecv) && id == d->bindId) {
                QXmppBindIq bind;
                bind.parse(nodeRecv);

                // bind result
                if (bind.type() == QXmppIq::Result) {
                    if (!bind.jid().isEmpty()) {
                        static const QRegularExpression jidRegex("^([^@/]+)@([^@/]+)/(.+)$");

                        if (const auto match = jidRegex.match(bind.jid()); match.hasMatch()) {
                            configuration().setUser(match.captured(1));
                            configuration().setDomain(match.captured(2));
                            configuration().setResource(match.captured(3));
                        } else {
                            warning("Bind IQ received with invalid JID: " + bind.jid());
                        }
                    }

                    if (d->sessionAvailable) {
                        d->sendSessionStart();
                    } else {
                        d->sessionStarted = true;

                        if (d->streamManagementAvailable) {
                            d->sendStreamManagementEnable();
                        } else {
                            // we are connected now
                            Q_EMIT connected();
                        }
                    }
                } else if (bind.type() == QXmppIq::Error) {
                    d->xmppStreamError = bind.error().condition();
                    Q_EMIT error(QXmppClient::XmppStreamError);
                    warning("Resource binding error received: " + bind.error().text());
                    disconnectFromHost();
                }
            }
            // extensions

            // XEP-0078: Non-SASL Authentication
            else if (id == d->nonSASLAuthId && type == "result") {
                // successful Non-SASL Authentication
                debug("Authenticated (Non-SASL)");
                d->isAuthenticated = true;

                // xmpp connection made
                d->sessionStarted = true;
                Q_EMIT connected();
            } else if (QXmppNonSASLAuthIq::isNonSASLAuthIq(nodeRecv)) {
                if (type == "result") {
                    bool digest = !nodeRecv.firstChildElement("query").firstChildElement("digest").isNull();
                    bool plain = !nodeRecv.firstChildElement("query").firstChildElement("password").isNull();
                    bool plainText = false;

                    if (plain && digest) {
                        if (configuration().nonSASLAuthMechanism() ==
                            QXmppConfiguration::NonSASLDigest) {
                            plainText = false;
                        } else {
                            plainText = true;
                        }
                    } else if (plain) {
                        plainText = true;
                    } else if (digest) {
                        plainText = false;
                    } else {
                        warning("No supported Non-SASL Authentication mechanism available");
                        disconnectFromHost();
                        return;
                    }
                    d->sendNonSASLAuth(plainText);
                }
            }
            // XEP-0199: XMPP Ping
            else if (QXmppPingIq::isPingIq(nodeRecv)) {
                QXmppPingIq req;
                req.parse(nodeRecv);

                QXmppIq iq(QXmppIq::Result);
                iq.setId(req.id());
                iq.setTo(req.from());
                sendPacket(iq);
            } else {
                QXmppIq iqPacket;
                iqPacket.parse(nodeRecv);

                // if we didn't understant the iq, reply with error
                // except for "result" and "error" iqs
                if (type != "result" && type != "error") {
                    QXmppIq iq(QXmppIq::Error);
                    iq.setId(iqPacket.id());
                    iq.setTo(iqPacket.from());
                    QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                                             QXmppStanza::Error::FeatureNotImplemented);
                    iq.setError(error);
                    sendPacket(iq);
                } else {
                    Q_EMIT iqReceived(iqPacket);
                }
            }
        } else if (nodeRecv.tagName() == "presence") {
            QXmppPresence presence;
            presence.parse(nodeRecv);

            // emit presence
            Q_EMIT presenceReceived(presence);
        } else if (nodeRecv.tagName() == "message") {
            QXmppMessage message;
            message.parse(nodeRecv);

            // emit message
            Q_EMIT messageReceived(message);
        }
    } else if (QXmppStreamManagementEnabled::isStreamManagementEnabled(nodeRecv)) {
        QXmppStreamManagementEnabled streamManagementEnabled;
        streamManagementEnabled.parse(nodeRecv);
        d->smId = streamManagementEnabled.id();
        d->canResume = streamManagementEnabled.resume();
        if (streamManagementEnabled.resume() && !streamManagementEnabled.location().isEmpty()) {
            setResumeAddress(streamManagementEnabled.location());
        }

        d->streamManagementEnabled = true;
        enableStreamManagement(true);
        // we are connected now
        Q_EMIT connected();
    } else if (QXmppStreamManagementResumed::isStreamManagementResumed(nodeRecv)) {
        QXmppStreamManagementResumed streamManagementResumed;
        streamManagementResumed.parse(nodeRecv);
        setAcknowledgedSequenceNumber(streamManagementResumed.h());
        d->isResuming = false;
        d->streamResumed = true;

        d->streamManagementEnabled = true;
        enableStreamManagement(false);
        // we are connected now
        // TODO: The stream was resumed. Therefore, we should not send presence information or request the roster.
        Q_EMIT connected();
    } else if (QXmppStreamManagementFailed::isStreamManagementFailed(nodeRecv)) {
        if (d->isResuming) {
            // resuming failed. We can try to bind a resource now.
            d->isResuming = false;

            // check whether bind is available
            if (d->bindModeAvailable) {
                d->sendBind();
                return;
            }

            // check whether session is available
            if (d->sessionAvailable) {
                d->sendSessionStart();
                return;
            }

            // otherwise we are done
            d->sessionStarted = true;
            Q_EMIT connected();
        } else {
            // we are connected now, but stream management is disabled
            Q_EMIT connected();
        }
    }
}
/// \endcond

void QXmppOutgoingClient::pingStart()
{
    const int interval = configuration().keepAliveInterval();
    // start ping timer
    if (interval > 0) {
        d->pingTimer->setInterval(interval * 1000);
        d->pingTimer->start();
    }
}

void QXmppOutgoingClient::pingStop()
{
    // stop all timers
    d->pingTimer->stop();
    d->timeoutTimer->stop();
}

void QXmppOutgoingClient::pingSend()
{
    // send ping packet
    QXmppPingIq ping;
    ping.setTo(configuration().domain());
    sendPacket(ping);

    // start timeout timer
    const int timeout = configuration().keepAliveTimeout();
    if (timeout > 0) {
        d->timeoutTimer->setInterval(timeout * 1000);
        d->timeoutTimer->start();
    }
}

void QXmppOutgoingClient::pingTimeout()
{
    warning("Ping timeout");
    QXmppStream::disconnectFromHost();
    Q_EMIT error(QXmppClient::KeepAliveError);
}

bool QXmppOutgoingClient::setResumeAddress(const QString &address)
{
    if (const auto location = parseHostAddress(address);
        !location.first.isEmpty()) {
        d->resumeHost = location.first;

        if (location.second > 0) {
            d->resumePort = location.second;
        } else {
            d->resumePort = 5222;
        }
        return true;
    }

    d->resumeHost.clear();
    d->resumePort = 0;
    return false;
}

std::pair<QString, int> QXmppOutgoingClient::parseHostAddress(const QString &address)
{
    QUrl url("//" + address);
    if (url.isValid() && !url.host().isEmpty()) {
        return { url.host(), url.port() };
    }
    return { {}, -1 };
}

void QXmppOutgoingClientPrivate::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(q->configuration().user());
    if (plainText) {
        authQuery.setPassword(q->configuration().password());
    } else {
        authQuery.setDigest(streamId, q->configuration().password());
    }
    authQuery.setResource(q->configuration().resource());
    nonSASLAuthId = authQuery.id();
    q->sendPacket(authQuery);
}

void QXmppOutgoingClientPrivate::sendNonSASLAuthQuery()
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(streamFrom);
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(q->configuration().user());
    q->sendPacket(authQuery);
}

void QXmppOutgoingClientPrivate::sendBind()
{
    QXmppBindIq bind;
    bind.setType(QXmppIq::Set);
    bind.setResource(q->configuration().resource());
    bindId = bind.id();
    q->sendPacket(bind);
}

void QXmppOutgoingClientPrivate::sendSessionStart()
{
    QXmppSessionIq session;
    session.setType(QXmppIq::Set);
    session.setTo(q->configuration().domain());
    sessionId = session.id();
    q->sendPacket(session);
}

void QXmppOutgoingClientPrivate::sendStreamManagementEnable()
{
    QXmppStreamManagementEnable streamManagementEnable(true);
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    streamManagementEnable.toXml(&xmlStream);
    q->sendData(data);
}

/// Returns the type of the last XMPP stream error that occurred.

QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    return d->xmppStreamError;
}
