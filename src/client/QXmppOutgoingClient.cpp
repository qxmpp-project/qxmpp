/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <QCryptographicHash>
#include <QNetworkProxy>
#include <QSslSocket>
#include <QUrl>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QDnsLookup>
#else
#include "qdnslookup.h"
#endif

#include "QXmppConfiguration.h"
#include "QXmppConstants.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include "QXmppOutgoingClient.h"
#include "QXmppStreamFeatures.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppSasl_p.h"
#include "QXmppUtils.h"

// IQ types
#include "QXmppBindIq.h"
#include "QXmppPingIq.h"
#include "QXmppSessionIq.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QTimer>

class QXmppOutgoingClientPrivate
{
public:
    QXmppOutgoingClientPrivate(QXmppOutgoingClient *q);
    void connectToHost(const QString &host, quint16 port);

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QXmppStanza::Error::Condition xmppStreamError;

    // DNS
    QDnsLookup dns;

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
    bool sessionAvailable;
    bool sessionStarted;

    // Authentication
    bool isAuthenticated;
    QString nonSASLAuthId;
    QXmppSaslClient *saslClient;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;

private:
    QXmppOutgoingClient *q;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate(QXmppOutgoingClient *qq)
    : redirectPort(0)
    , sessionAvailable(false)
    , sessionStarted(false)
    , isAuthenticated(false)
    , saslClient(0)
    , pingTimer(0)
    , timeoutTimer(0)
    , q(qq)
{
}

void QXmppOutgoingClientPrivate::connectToHost(const QString &host, quint16 port)
{
    q->info(QString("Connecting to %1:%2").arg(host, QString::number(port)));

    // override CA certificates if requested
    if (!config.caCertificates().isEmpty())
        q->socket()->setCaCertificates(config.caCertificates());

    // respect proxy
    q->socket()->setProxy(config.networkProxy());

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    // set the name the SSL certificate should match
    q->socket()->setPeerVerifyName(config.domain());
#endif

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

/// Constructs an outgoing client stream.
///
/// \param parent

QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppStream(parent),
    d(new QXmppOutgoingClientPrivate(this))
{
    bool check;
    Q_UNUSED(check);

    // initialise socket
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    check = connect(socket, SIGNAL(disconnected()),
                    this, SLOT(_q_socketDisconnected()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                    this, SLOT(socketSslErrors(QList<QSslError>)));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    // DNS lookups
    check = connect(&d->dns, SIGNAL(finished()),
                    this, SLOT(_q_dnsLookupFinished()));
    Q_ASSERT(check);

    // XEP-0199: XMPP Ping
    d->pingTimer = new QTimer(this);
    check = connect(d->pingTimer, SIGNAL(timeout()),
                    this, SLOT(pingSend()));
    Q_ASSERT(check);

    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    check = connect(d->timeoutTimer, SIGNAL(timeout()),
                    this, SLOT(pingTimeout()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(connected()),
                    this, SLOT(pingStart()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(disconnected()),
                    this, SLOT(pingStop()));
    Q_ASSERT(check);
}

/// Destroys an outgoing client stream.

QXmppOutgoingClient::~QXmppOutgoingClient()
{
    delete d;
}

/// Returns a reference to the stream's configuration.

QXmppConfiguration& QXmppOutgoingClient::configuration()
{
    return d->config;
}

/// Attempts to connect to the XMPP server.

void QXmppOutgoingClient::connectToHost()
{
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
}

void QXmppOutgoingClient::_q_dnsLookupFinished()
{
    if (d->dns.error() == QDnsLookup::NoError &&
        !d->dns.serviceRecords().isEmpty()) {
        // take the first returned record
        d->connectToHost(
            d->dns.serviceRecords().first().target(),
            d->dns.serviceRecords().first().port());
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

void QXmppOutgoingClient::_q_socketDisconnected()
{
    debug("Socket disconnected");
    d->isAuthenticated = false;
    if (!d->redirectHost.isEmpty() && d->redirectPort > 0) {
        d->connectToHost(d->redirectHost, d->redirectPort);
        d->redirectHost = QString();
        d->redirectPort = 0;
    } else {
        emit disconnected();
    }
}

void QXmppOutgoingClient::socketSslErrors(const QList<QSslError> & error)
{
    warning("SSL errors");
    for(int i = 0; i< error.count(); ++i)
        warning(error.at(i).errorString());

    if (configuration().ignoreSslErrors())
        socket()->ignoreSslErrors();
}

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit error(QXmppClient::SocketError);
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
        d->saslClient = 0;
    }

    // reset session information
    d->bindId.clear();
    d->sessionId.clear();
    d->sessionAvailable = false;
    d->sessionStarted = false;

    // start stream
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain().toUtf8());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    sendData(data);
}

void QXmppOutgoingClient::handleStream(const QDomElement &streamElement)
{
    if(d->streamId.isEmpty())
        d->streamId = streamElement.attribute("id");
    if (d->streamFrom.isEmpty())
        d->streamFrom = streamElement.attribute("from");
    if(d->streamVersion.isEmpty())
    {
        d->streamVersion = streamElement.attribute("version");

        // no version specified, signals XMPP Version < 1.0.
        // switch to old auth mechanism if enabled
        if(d->streamVersion.isEmpty() && configuration().useNonSASLAuthentication()) {
            sendNonSASLAuthQuery();
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
    emit elementReceived(nodeRecv, handled);
    if (handled)
        return;

    if(QXmppStreamFeatures::isStreamFeatures(nodeRecv))
    {
        QXmppStreamFeatures features;
        features.parse(nodeRecv);

        if (!socket()->isEncrypted())
        {
            // determine TLS mode to use
            const QXmppConfiguration::StreamSecurityMode localSecurity = configuration().streamSecurityMode();
            const QXmppStreamFeatures::Mode remoteSecurity = features.tlsMode();
            if (!socket()->supportsSsl() &&
                (localSecurity == QXmppConfiguration::TLSRequired ||
                 remoteSecurity == QXmppStreamFeatures::Required))
            {
                warning("Disconnecting as TLS is required, but SSL support is not available");
                disconnectFromHost();
                return;
            }
            if (localSecurity == QXmppConfiguration::TLSRequired &&
                remoteSecurity == QXmppStreamFeatures::Disabled)
            {
                warning("Disconnecting as TLS is required, but not supported by the server");
                disconnectFromHost();
                return;
            }

            if (socket()->supportsSsl() &&
                localSecurity != QXmppConfiguration::TLSDisabled &&
                remoteSecurity != QXmppStreamFeatures::Disabled)
            {
                // enable TLS as it is support by both parties
                sendData("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
                return;
            }
        }

        // handle authentication
        const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
        const bool saslAvailable = !features.authMechanisms().isEmpty();
        if (saslAvailable && configuration().useSASLAuthentication())
        {
            // supported and preferred SASL auth mechanisms
            QStringList supportedMechanisms = QXmppSaslClient::availableMechanisms();
            const QString preferredMechanism = configuration().saslAuthMechanism();
            if (configuration().facebookAppId().isEmpty() || configuration().facebookAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-FACEBOOK-PLATFORM");
            if (configuration().windowsLiveAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-MESSENGER-OAUTH2");
            if (configuration().googleAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-OAUTH2");

            // determine SASL Authentication mechanism to use
            QStringList commonMechanisms;
            QString usedMechanism;
            foreach (const QString &mechanism, features.authMechanisms()) {
                if (supportedMechanisms.contains(mechanism))
                    commonMechanisms << mechanism;
            }
            if (commonMechanisms.isEmpty()) {
                warning("No supported SASL Authentication mechanism available");
                disconnectFromHost();
                return;
            } else if (!commonMechanisms.contains(preferredMechanism)) {
                info(QString("Desired SASL Auth mechanism '%1' is not available, selecting first available one").arg(preferredMechanism));
                usedMechanism = commonMechanisms.first();
            } else {
                usedMechanism = preferredMechanism;
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
        } else if(nonSaslAvailable && configuration().useNonSASLAuthentication()) {
            sendNonSASLAuthQuery();
            return;
        }

        // store whether session is available
        d->sessionAvailable = (features.sessionMode() != QXmppStreamFeatures::Disabled);

        // check whether bind is available
        if (features.bindMode() != QXmppStreamFeatures::Disabled)
        {
            QXmppBindIq bind;
            bind.setType(QXmppIq::Set);
            bind.setResource(configuration().resource());
            d->bindId = bind.id();
            sendPacket(bind);
            return;
        }

        // check whether session is available
        if (d->sessionAvailable)
        {
            // start session if it is available
            QXmppSessionIq session;
            session.setType(QXmppIq::Set);
            session.setTo(configuration().domain());
            d->sessionId = session.id();
            sendPacket(session);
        } else {
            // otherwise we are done
            d->sessionStarted = true;
            emit connected();
        }
    }
    else if(ns == ns_stream && nodeRecv.tagName() == "error")
    {
        // handle redirects
        QRegExp redirectRegex("([^:]+)(:[0-9]+)?");
        if (redirectRegex.exactMatch(nodeRecv.firstChildElement("see-other-host").text())) {
            d->redirectHost = redirectRegex.cap(0);
            if (!redirectRegex.cap(2).isEmpty())
                d->redirectPort = redirectRegex.cap(2).mid(1).toUShort();
            else
                d->redirectPort = 5222;
            disconnectFromHost();
            return;
        }

        if (!nodeRecv.firstChildElement("conflict").isNull())
            d->xmppStreamError = QXmppStanza::Error::Conflict;
        else
            d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
        emit error(QXmppClient::XmppStreamError);
    }
    else if(ns == ns_tls)
    {
        if(nodeRecv.tagName() == "proceed")
        {
            debug("Starting encryption");
            socket()->startClientEncryption();
            return;
        }
    }
    else if(ns == ns_sasl)
    {
        if (!d->saslClient) {
            warning("SASL stanza received, but no mechanism selected");
            return;
        }
        if(nodeRecv.tagName() == "success")
        {
            debug("Authenticated");
            d->isAuthenticated = true;
            handleStart();
        }
        else if(nodeRecv.tagName() == "challenge")
        {
            QXmppSaslChallenge challenge;
            challenge.parse(nodeRecv);

            QByteArray response;
            if (d->saslClient->respond(challenge.value(), response)) {
                sendPacket(QXmppSaslResponse(response));
            } else {
                warning("Could not respond to SASL challenge");
                disconnectFromHost();
            }
        }
        else if(nodeRecv.tagName() == "failure")
        {
            QXmppSaslFailure failure;
            failure.parse(nodeRecv);

            if (failure.condition() == "not-authorized")
                d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
            else if (failure.condition() == "bad-auth")
                d->xmppStreamError = QXmppStanza::Error::BadAuth;
            else
                d->xmppStreamError = QXmppStanza::Error::UndefinedCondition;
            emit error(QXmppClient::XmppStreamError);

            warning("Authentication failure");
            disconnectFromHost();
        }
    }
    else if(ns == ns_client)
    {

        if(nodeRecv.tagName() == "iq")
        {
            QDomElement element = nodeRecv.firstChildElement();
            QString id = nodeRecv.attribute("id");
            QString type = nodeRecv.attribute("type");
            if(type.isEmpty())
                warning("QXmppStream: iq type can't be empty");

            if(id == d->sessionId)
            {
                QXmppSessionIq session;
                session.parse(nodeRecv);

                // xmpp connection made
                d->sessionStarted = true;
                emit connected();
            }
            else if(QXmppBindIq::isBindIq(nodeRecv) && id == d->bindId)
            {
                QXmppBindIq bind;
                bind.parse(nodeRecv);

                // bind result
                if (bind.type() == QXmppIq::Result)
                {
                    if (!bind.jid().isEmpty())
                    {
                        QRegExp jidRegex("^([^@/]+)@([^@/]+)/(.+)$");
                        if (jidRegex.exactMatch(bind.jid()))
                        {
                            configuration().setUser(jidRegex.cap(1));
                            configuration().setDomain(jidRegex.cap(2));
                            configuration().setResource(jidRegex.cap(3));
                        } else {
                            warning("Bind IQ received with invalid JID: " + bind.jid());
                        }
                    }

                    if (d->sessionAvailable)
                    {
                        // start session if it is available
                        QXmppSessionIq session;
                        session.setType(QXmppIq::Set);
                        session.setTo(configuration().domain());
                        d->sessionId = session.id();
                        sendPacket(session);
                    } else {
                        // otherwise we are done
                        d->sessionStarted = true;
                        emit connected();
                    }
                }
            }
            // extensions

            // XEP-0078: Non-SASL Authentication
            else if(id == d->nonSASLAuthId && type == "result")
            {
                // successful Non-SASL Authentication
                debug("Authenticated (Non-SASL)");
                d->isAuthenticated = true;

                // xmpp connection made
                d->sessionStarted = true;
                emit connected();
            }
            else if(QXmppNonSASLAuthIq::isNonSASLAuthIq(nodeRecv))
            {
                if(type == "result")
                {
                    bool digest = !nodeRecv.firstChildElement("query").
                         firstChildElement("digest").isNull();
                    bool plain = !nodeRecv.firstChildElement("query").
                         firstChildElement("password").isNull();
                    bool plainText = false;

                    if(plain && digest)
                    {
                        if(configuration().nonSASLAuthMechanism() ==
                           QXmppConfiguration::NonSASLDigest)
                            plainText = false;
                        else
                            plainText = true;
                    }
                    else if(plain)
                        plainText = true;
                    else if(digest)
                        plainText = false;
                    else
                    {
                        warning("No supported Non-SASL Authentication mechanism available");
                        disconnectFromHost();
                        return;
                    }
                    sendNonSASLAuth(plainText);
                }
            }
            // XEP-0199: XMPP Ping
            else if(QXmppPingIq::isPingIq(nodeRecv))
            {
                QXmppPingIq req;
                req.parse(nodeRecv);

                QXmppIq iq(QXmppIq::Result);
                iq.setId(req.id());
                iq.setTo(req.from());
                sendPacket(iq);
            }
            else
            {
                QXmppIq iqPacket;
                iqPacket.parse(nodeRecv);

                // if we didn't understant the iq, reply with error
                // except for "result" and "error" iqs
                if (type != "result" && type != "error")
                {
                    QXmppIq iq(QXmppIq::Error);
                    iq.setId(iqPacket.id());
                    iq.setTo(iqPacket.from());
                    QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                        QXmppStanza::Error::FeatureNotImplemented);
                    iq.setError(error);
                    sendPacket(iq);
                } else {
                    emit iqReceived(iqPacket);
                }
            }
        }
        else if(nodeRecv.tagName() == "presence")
        {
            QXmppPresence presence;
            presence.parse(nodeRecv);

            // emit presence
            emit presenceReceived(presence);
        }
        else if(nodeRecv.tagName() == "message")
        {
            QXmppMessage message;
            message.parse(nodeRecv);

            // emit message
            emit messageReceived(message);
        }
    }
}
/// \endcond

void QXmppOutgoingClient::pingStart()
{
    const int interval = configuration().keepAliveInterval();
    // start ping timer
    if (interval > 0)
    {
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
    if (timeout > 0)
    {
        d->timeoutTimer->setInterval(timeout * 1000);
        d->timeoutTimer->start();
    }
}

void QXmppOutgoingClient::pingTimeout()
{
    warning("Ping timeout");
    disconnectFromHost();
    emit error(QXmppClient::KeepAliveError);
}

void QXmppOutgoingClient::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(configuration().user());
    if (plainText)
        authQuery.setPassword(configuration().password());
    else
        authQuery.setDigest(d->streamId, configuration().password());
    authQuery.setResource(configuration().resource());
    d->nonSASLAuthId = authQuery.id();
    sendPacket(authQuery);
}

void QXmppOutgoingClient::sendNonSASLAuthQuery()
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(d->streamFrom);
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(configuration().user());
    sendPacket(authQuery);
}

/// Returns the type of the last XMPP stream error that occured.

QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    return d->xmppStreamError;
}

