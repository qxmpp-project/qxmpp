/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include <QSslSocket>

#include "QXmppConfiguration.h"
#include "QXmppConstants.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPacket.h"
#include "QXmppPresence.h"
#include "QXmppOutgoingClient.h"
#include "QXmppSrvInfo.h"
#include "QXmppStreamFeatures.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppSaslAuth.h"
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
    QXmppOutgoingClientPrivate();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QAbstractSocket::SocketError socketError;
    QXmppStanza::Error::Condition xmppStreamError;

    // State data
    QString bindId;
    QString sessionId;
    bool sessionAvailable;
    bool sessionStarted;
    QString streamId;
    QString streamFrom;
    QString streamVersion;
    QString nonSASLAuthId;

    // SASL
    QXmppSaslDigestMd5 saslDigest;
    int saslStep;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate()
    : sessionAvailable(false),
    saslStep(0)
{
}

/// Constructs an outgoing client stream.
///
/// \param parent

QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppStream(parent),
    d(new QXmppOutgoingClientPrivate)
{
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    // initialise logger
    bool check = connect(socket, SIGNAL(sslErrors(const QList<QSslError>&)),
                         this, SLOT(socketSslErrors(const QList<QSslError>&)));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(socketError(QAbstractSocket::SocketError)));
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
    const QString host = configuration().host();
    const quint16 port = configuration().port();

    // if an explicit host was provided, connect to it
    if (!host.isEmpty() && port)
    {
        info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
        socket()->setProxy(configuration().networkProxy());
        socket()->connectToHost(host, port);
        return;
    }

    // otherwise, lookup server
    const QString domain = configuration().domain();
    debug(QString("Looking up server for domain %1").arg(domain));
    QXmppSrvInfo::lookupService("_xmpp-client._tcp." + domain, this,
                                SLOT(connectToHost(QXmppSrvInfo)));
}

void QXmppOutgoingClient::connectToHost(const QXmppSrvInfo &serviceInfo)
{
    const QString domain = configuration().domain();
    QString host = configuration().host();
    quint16 port = configuration().port();

    if (!serviceInfo.records().isEmpty())
    {
        // take the first returned record
        host = serviceInfo.records().first().target();
        port = serviceInfo.records().first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2")
                .arg(domain, serviceInfo.errorString()));
        host = domain;
    }

    // connect to server
    info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
    socket()->setProxy(configuration().networkProxy());
    socket()->connectToHost(host, port);
}

/// Returns true if the socket is connected and a session has been started.
///

bool QXmppOutgoingClient::isConnected() const
{
    return QXmppStream::isConnected() && d->sessionStarted;
}

void QXmppOutgoingClient::socketSslErrors(const QList<QSslError> & error)
{
    warning("SSL errors");
    for(int i = 0; i< error.count(); ++i)
        warning(error.at(i).errorString());

    if (configuration().ignoreSslErrors())
        socket()->ignoreSslErrors();
}

void QXmppOutgoingClient::socketError(QAbstractSocket::SocketError ee)
{
    d->socketError = ee;
    emit error(QXmppClient::SocketError);
    warning(QString("Socket error: " + socket()->errorString()));
}

void QXmppOutgoingClient::handleStart()
{
    // reset authentication step
    d->saslStep = 0;
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
        // switch to old auth mechanism
        if(d->streamVersion.isEmpty())
            sendNonSASLAuthQuery();
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
        const bool useSasl = configuration().useSASLAuthentication();
        if((saslAvailable && nonSaslAvailable && !useSasl) ||
           (!saslAvailable && nonSaslAvailable))
        {
            sendNonSASLAuthQuery();
        }
        else if(saslAvailable)
        {
            // determine SASL Authentication mechanism to use
            QList<QXmppConfiguration::SASLAuthMechanism> mechanisms = features.authMechanisms();
            QXmppConfiguration::SASLAuthMechanism mechanism = configuration().sASLAuthMechanism();
            if (mechanisms.isEmpty())
            {
                warning("No supported SASL Authentication mechanism available");
                disconnectFromHost();
                return;
            }
            else if (!mechanisms.contains(mechanism))
            {
                info("Desired SASL Auth mechanism is not available, selecting first available one");
                mechanism = mechanisms.first();
            }

            // send SASL Authentication request
            switch(mechanism)
            {
            case QXmppConfiguration::SASLPlain:
                {
                    QString userPass('\0' + configuration().user() +
                                     '\0' + configuration().password());
                    QByteArray data = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>";
                    data += userPass.toUtf8().toBase64();
                    data += "</auth>";
                    sendData(data);
                }
                break;
            case QXmppConfiguration::SASLDigestMD5:
                sendData("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='DIGEST-MD5'/>");
                break;
            case QXmppConfiguration::SASLAnonymous:
                sendData("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='ANONYMOUS'/>");
                break;
            }
        }

        // check whether bind is available
        if (features.bindMode() != QXmppStreamFeatures::Disabled)
        {
            QXmppBindIq bind;
            bind.setType(QXmppIq::Set);
            bind.setResource(configuration().resource());
            d->bindId = bind.id();
            sendPacket(bind);
        }

        // check whether session is available
        if (features.sessionMode() != QXmppStreamFeatures::Disabled)
            d->sessionAvailable = true;
    }
    else if(ns == ns_stream && nodeRecv.tagName() == "error")
    {
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
        if(nodeRecv.tagName() == "success")
        {
            debug("Authenticated");
            handleStart();
        }
        else if(nodeRecv.tagName() == "challenge")
        {
            // TODO: Track which mechanism was used for when other SASL protocols which use challenges are supported
            d->saslStep++;
            switch (d->saslStep)
            {
            case 1 :
                sendAuthDigestMD5ResponseStep1(nodeRecv.text());
                break;
            case 2 :
                sendAuthDigestMD5ResponseStep2(nodeRecv.text());
                break;
            default :
                warning("Too many authentication steps");
                disconnectFromHost();
                break;
            }
        }
        else if(nodeRecv.tagName() == "failure")
        {
            if (!nodeRecv.firstChildElement("not-authorized").isNull())
                d->xmppStreamError = QXmppStanza::Error::NotAuthorized;
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

                    // start session if it is available
                    if (d->sessionAvailable)
                    {
                        QXmppSessionIq session;
                        session.setType(QXmppIq::Set);
                        session.setTo(configuration().domain());
                        d->sessionId = session.id();
                        sendPacket(session);
                    }
                }
            }
            // extensions

            // XEP-0078: Non-SASL Authentication
            else if(id == d->nonSASLAuthId && type == "result")
            {
                // successful Non-SASL Authentication
                debug("Authenticated (Non-SASL)");

                // xmpp connection made
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

// challenge is BASE64 encoded string
void QXmppOutgoingClient::sendAuthDigestMD5ResponseStep1(const QString& challenge)
{
    QByteArray ba = QByteArray::fromBase64(challenge.toAscii());
    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(ba);

    if (!map.contains("nonce"))
    {
        warning("sendAuthDigestMD5ResponseStep1: Invalid input");
        disconnectFromHost();
        return;
    }

    d->saslDigest.setAuthzid(map.value("authzid"));
    d->saslDigest.setCnonce(QXmppSaslDigestMd5::generateNonce());
    d->saslDigest.setDigestUri(QString("xmpp/%1").arg(configuration().domain()).toUtf8());
    d->saslDigest.setNc("00000001");
    d->saslDigest.setNonce(map.value("nonce"));
    d->saslDigest.setQop("auth");
    d->saslDigest.setRealm(map.value("realm"));
    d->saslDigest.setUsername(configuration().user().toUtf8());
    d->saslDigest.setPassword(configuration().password().toUtf8());

    // Build response
    QMap<QByteArray, QByteArray> response;
    response["username"] = d->saslDigest.username();
    if(!d->saslDigest.realm().isEmpty())
        response["realm"] = d->saslDigest.realm();
    response["nonce"] = d->saslDigest.nonce();
    response["cnonce"] = d->saslDigest.cnonce();
    response["nc"] = d->saslDigest.nc();
    response["qop"] = d->saslDigest.qop();
    response["digest-uri"] = d->saslDigest.digestUri();
    response["response"] = d->saslDigest.calculateDigest(
        QByteArray("AUTHENTICATE:") + d->saslDigest.digestUri());

    if(!d->saslDigest.authzid().isEmpty())
        response["authzid"] = d->saslDigest.authzid();
    response["charset"] = "utf-8";

    const QByteArray data = QXmppSaslDigestMd5::serializeMessage(response);
    QByteArray packet = "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
                        + data.toBase64() + "</response>";
    sendData(packet);
}

void QXmppOutgoingClient::sendAuthDigestMD5ResponseStep2(const QString &challenge)
{
    QByteArray ba = QByteArray::fromBase64(challenge.toAscii());
    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(ba);

    if (!map.contains("rspauth"))
    {
        warning("sendAuthDigestMD5ResponseStep2: Invalid input");
        disconnectFromHost();
        return;
    }

    // check new challenge
    if (map["rspauth"] !=
        d->saslDigest.calculateDigest(QByteArray(":") + d->saslDigest.digestUri()))
    {
        warning("sendAuthDigestMD5ResponseStep2: Bad challenge");
        disconnectFromHost();
        return;
    }

    sendData("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
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

/// Returns the type of the last socket error that occured.

QAbstractSocket::SocketError QXmppOutgoingClient::socketError()
{
    return d->socketError;
}

/// Returns the type of the last XMPP stream error that occured.

QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    return d->xmppStreamError;
}

