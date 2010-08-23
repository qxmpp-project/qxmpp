/*
 * Copyright (C) 2008-2010 The QXmpp developers
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
#include "QXmppStreamFeatures.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppUtils.h"

// IQ types
#include "QXmppArchiveIq.h"
#include "QXmppBindIq.h"
#include "QXmppByteStreamIq.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppIbbIq.h"
#include "QXmppJingleIq.h"
#include "QXmppMucIq.h"
#include "QXmppPingIq.h"
#include "QXmppRpcIq.h"
#include "QXmppRosterIq.h"
#include "QXmppSessionIq.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppVCard.h"
#include "QXmppVersionIq.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QTimer>

static const QString capabilitiesNode = "http://code.google.com/p/qxmpp";

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
    int authStep;
    QString bindId;
    QString sessionId;
    bool sessionAvailable;
    bool sessionStarted;
    QString streamId;
    QString streamFrom;
    QString streamVersion;
    QString nonSASLAuthId;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

QXmppOutgoingClientPrivate::QXmppOutgoingClientPrivate()
    : authStep(0),
    sessionAvailable(false)
{
}

QXmppOutgoingClient::QXmppOutgoingClient(QObject *parent)
    : QXmppStream(parent),
    d(new QXmppOutgoingClientPrivate)
{
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    // initialise logger
    setLogger(QXmppLogger::getLogger());

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

QXmppOutgoingClient::~QXmppOutgoingClient()
{
    delete d;
}

QXmppConfiguration& QXmppOutgoingClient::configuration()
{
    return d->config;
}

void QXmppOutgoingClient::connectToHost()
{
    info(QString("Connecting to: %1:%2").arg(configuration().
            host()).arg(configuration().port()));

    socket()->setProxy(configuration().networkProxy());
    socket()->connectToHost(configuration().host(),
                            configuration().port());
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
    d->authStep = 0;
    d->sessionStarted = false;

    // start stream
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain());
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
            const QXmppConfiguration::StreamSecurityMode remoteSecurity = features.securityMode();
            if (!socket()->supportsSsl() &&
                (localSecurity == QXmppConfiguration::TLSRequired ||
                 remoteSecurity == QXmppConfiguration::TLSRequired))
            {
                warning("Disconnecting as TLS is required, but SSL support is not available");
                disconnectFromHost();
                return;
            }
            if (localSecurity == QXmppConfiguration::TLSRequired &&
                remoteSecurity == QXmppConfiguration::TLSDisabled)
            {
                warning("Disconnecting as TLS is required, but not supported by the server");
                disconnectFromHost();
                return;
            }

            if (socket()->supportsSsl() &&
                remoteSecurity != QXmppConfiguration::TLSDisabled && 
                localSecurity != QXmppConfiguration::TLSDisabled)
            {
                // enable TLS as it is support by both parties
                sendData("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
                return;
            }
        }

        // handle authentication
        const bool nonSaslAvailable = features.isNonSaslAuthAvailable();
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
                                     '\0' + configuration().passwd());
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
        if (features.isBindAvailable())
        {
            QXmppBindIq bind(QXmppIq::Set);
            bind.setResource(configuration().resource());
            d->bindId = bind.id();
            sendPacket(bind);
        }

        // check whether session is available
        if (features.isSessionAvailable())
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
            d->authStep++;
            switch (d->authStep)
            {
            case 1 :
                sendAuthDigestMD5ResponseStep1(nodeRecv.text());
                break;
            case 2 :
                sendAuthDigestMD5ResponseStep2();
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
            else if(QXmppRosterIq::isRosterIq(nodeRecv))
            {
                QXmppRosterIq rosterIq;
                rosterIq.parse(nodeRecv);
                emit rosterIqReceived(rosterIq);
            }
            // extensions

            // XEP-0009: Jabber-RPC
            else if(QXmppRpcInvokeIq::isRpcInvokeIq(nodeRecv))
            {
                QXmppRpcInvokeIq rpcIqPacket;
                rpcIqPacket.parse(nodeRecv);
                emit rpcCallInvoke(rpcIqPacket);
            }
            else if(QXmppRpcResponseIq::isRpcResponseIq(nodeRecv))
            {
                QXmppRpcResponseIq rpcResponseIq;
                rpcResponseIq.parse(nodeRecv);
                emit rpcCallResponse(rpcResponseIq);
            }
            else if(QXmppRpcErrorIq::isRpcErrorIq(nodeRecv))
            {
                QXmppRpcErrorIq rpcErrorIq;
                rpcErrorIq.parse(nodeRecv);
                emit rpcCallError(rpcErrorIq);
            }

            // XEP-0030: Service Discovery
            else if(QXmppDiscoveryIq::isDiscoveryIq(nodeRecv))
            {
                QXmppDiscoveryIq discoIq;
                discoIq.parse(nodeRecv);

                if (discoIq.type() == QXmppIq::Get &&
                    discoIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
                    (discoIq.queryNode().isEmpty() || discoIq.queryNode().startsWith(capabilitiesNode)))
                {
                    // respond to info query
                    QXmppDiscoveryIq qxmppFeatures = capabilities();
                    qxmppFeatures.setId(discoIq.id());
                    qxmppFeatures.setTo(discoIq.from());
                    qxmppFeatures.setQueryNode(discoIq.queryNode());
                    sendPacket(qxmppFeatures);
                } else {
                    emit discoveryIqReceived(discoIq);
                }
            }
            // XEP-0045: Multi-User Chat
            else if (QXmppMucAdminIq::isMucAdminIq(nodeRecv))
            {
                QXmppMucAdminIq mucIq;
                mucIq.parse(nodeRecv);
                emit mucAdminIqReceived(mucIq);
            }
            else if (QXmppMucOwnerIq::isMucOwnerIq(nodeRecv))
            {
                QXmppMucOwnerIq mucIq;
                mucIq.parse(nodeRecv);
                emit mucOwnerIqReceived(mucIq);
            }
            // XEP-0047 In-Band Bytestreams
            else if(QXmppIbbCloseIq::isIbbCloseIq(nodeRecv))
            {
                QXmppIbbCloseIq ibbCloseIq;
                ibbCloseIq.parse(nodeRecv);
                emit ibbCloseIqReceived(ibbCloseIq);
            }
            else if(QXmppIbbDataIq::isIbbDataIq(nodeRecv))
            {
                QXmppIbbDataIq ibbDataIq;
                ibbDataIq.parse(nodeRecv);
                emit ibbDataIqReceived(ibbDataIq);
            }
            else if(QXmppIbbOpenIq::isIbbOpenIq(nodeRecv))
            {
                QXmppIbbOpenIq ibbOpenIq;
                ibbOpenIq.parse(nodeRecv);
                emit ibbOpenIqReceived(ibbOpenIq);
            }
            // XEP-0054: vcard-temp
            else if(nodeRecv.firstChildElement("vCard").
                    namespaceURI() == ns_vcard)
            {
                QXmppVCard vcardIq;
                vcardIq.parse(nodeRecv);
                emit vCardIqReceived(vcardIq);
            }
            // XEP-0065: SOCKS5 Bytestreams
            else if(QXmppByteStreamIq::isByteStreamIq(nodeRecv))
            {
                QXmppByteStreamIq byteStreamIq;
                byteStreamIq.parse(nodeRecv);
                emit byteStreamIqReceived(byteStreamIq);
            }
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
            // XEP-0092: Software Version
            else if(QXmppVersionIq::isVersionIq(nodeRecv))
            {
                QXmppVersionIq versionIq;
                versionIq.parse(nodeRecv);

                if (versionIq.type() == QXmppIq::Get)
                {
                    // respond to query
                    QXmppVersionIq responseIq;
                    responseIq.setType(QXmppIq::Result);
                    responseIq.setId(versionIq.id());
                    responseIq.setTo(versionIq.from());
                    responseIq.setName(qApp->applicationName());
                    responseIq.setVersion(qApp->applicationVersion());
                    sendPacket(responseIq);
                } else {
                    emit iqReceived(versionIq);
                }

            }
            // XEP-0095: Stream Initiation
            else if(QXmppStreamInitiationIq::isStreamInitiationIq(nodeRecv))
            {
                QXmppStreamInitiationIq siIq;
                siIq.parse(nodeRecv);
                emit streamInitiationIqReceived(siIq);
            }
            // XEP-0136: Message Archiving
            else if(QXmppArchiveChatIq::isArchiveChatIq(nodeRecv))
            {
                QXmppArchiveChatIq archiveIq;
                archiveIq.parse(nodeRecv);
                emit archiveChatIqReceived(archiveIq);
            }
            else if(QXmppArchiveListIq::isArchiveListIq(nodeRecv))
            {
                QXmppArchiveListIq archiveIq;
                archiveIq.parse(nodeRecv);
                emit archiveListIqReceived(archiveIq);
            }
            else if(QXmppArchivePrefIq::isArchivePrefIq(nodeRecv))
            {
                QXmppArchivePrefIq archiveIq;
                archiveIq.parse(nodeRecv);
                emit archivePrefIqReceived(archiveIq);
            }
            // XEP-0166: Jingle
            else if(QXmppJingleIq::isJingleIq(nodeRecv))
            {
                QXmppJingleIq jingleIq;
                jingleIq.parse(nodeRecv);
                emit jingleIqReceived(jingleIq);
            }
            // XEP-0199: XMPP Ping
            else if(QXmppPingIq::isPingIq(nodeRecv))
            {
                QXmppPingIq req;
                req.parse(nodeRecv);

                QXmppIq iq(QXmppIq::Result);
                iq.setId(req.id());
                iq.setTo(req.from());
                iq.setFrom(req.to());
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
                    iq.setFrom(iqPacket.to());
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
    ping.setFrom(configuration().jid());
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
    QByteArray ba = QByteArray::fromBase64(challenge.toUtf8());

    QMap<QByteArray, QByteArray> map = parseDigestMd5(ba);

    if (!map.contains("nonce"))
    {
        warning("sendAuthDigestMD5ResponseStep1: Invalid input");
        disconnectFromHost();
        return;
    }

    const QByteArray user = configuration().user().toUtf8();
    const QByteArray passwd = configuration().passwd().toUtf8();
    const QByteArray domain = configuration().domain().toUtf8();
    const QByteArray realm = map.value("realm");

    QByteArray cnonce(32, 'm');
    for(int n = 0; n < cnonce.size(); ++n)
        cnonce[n] = (char)(256.0*qrand()/(RAND_MAX+1.0));

    // The random data can the '=' char is not valid as it is a delimiter,
    // so to be safe, base64 the nonce
    cnonce = cnonce.toBase64();

    const QByteArray nc = "00000001";
    const QByteArray digest_uri = "xmpp/" + domain;
    const QByteArray authzid = map.value("authzid");
    const QByteArray nonce = map.value("nonce");
    const QByteArray a1 = user + ':' + realm + ':' + passwd;

    // Build response
    QMap<QByteArray, QByteArray> response;
    response["username"] = user;
    if(!realm.isEmpty())
        response["realm"] = realm;
    response["nonce"] = map["nonce"];
    response["cnonce"] = cnonce;
    response["nc"] = nc;
    response["qop"] = "auth";
    response["digest-uri"] = digest_uri;
    response["response"] = calculateDigestMd5(a1, nonce, nc, cnonce, digest_uri, authzid).toHex();
    if(!authzid.isEmpty())
        response["authzid"] = authzid;
    response["charset"] = "utf-8";

    const QByteArray data = serializeDigestMd5(response);
    QByteArray packet = "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
                        + data.toBase64() + "</response>";
    sendData(packet);
}

void QXmppOutgoingClient::sendAuthDigestMD5ResponseStep2()
{
    sendData("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
}

void QXmppOutgoingClient::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(configuration().user());
    if (plainText)
        authQuery.setPassword(configuration().passwd());
    else
        authQuery.setDigest(d->streamId, configuration().passwd());
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

QAbstractSocket::SocketError QXmppOutgoingClient::socketError()
{
    return d->socketError;
}

QXmppStanza::Error::Condition QXmppOutgoingClient::xmppStreamError()
{
    return d->xmppStreamError;
}

QXmppDiscoveryIq QXmppOutgoingClient::capabilities() const
{
    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);

    // features
    QStringList features;
    features
        << ns_rpc               // XEP-0009: Jabber-RPC
        << ns_disco_info        // XEP-0030: Service Discovery
        << ns_ibb               // XEP-0047: In-Band Bytestreams
        << ns_vcard             // XEP-0054: vcard-temp
        << ns_bytestreams       // XEP-0065: SOCKS5 Bytestreams
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_version           // XEP-0092: Software Version
        << ns_stream_initiation // XEP-0095: Stream Initiation
        << ns_stream_initiation_file_transfer // XEP-0096: SI File Transfer
        << ns_capabilities      // XEP-0115 : Entity Capabilities
        << ns_jingle            // XEP-0166 : Jingle
        << ns_jingle_rtp        // XEP-0167 : Jingle RTP Sessions
        << ns_jingle_rtp_audio
        << ns_jingle_ice_udp    // XEP-0176 : Jingle ICE-UDP Transport Method
        << ns_ping;             // XEP-0199: XMPP Ping
    iq.setFeatures(features);

    // identities
    QList<QXmppDiscoveryIq::Identity> identities;
    QXmppDiscoveryIq::Identity identity;

    identity.setCategory("automation");
    identity.setType("rpc");
    identities.append(identity);

    identity.setCategory("client");
    identity.setType("pc");
    identity.setName(QString("%1 %2").arg(qApp->applicationName(), qApp->applicationVersion()));
    identities.append(identity);

    iq.setIdentities(identities);
    return iq;
}

QXmppElementList QXmppOutgoingClient::presenceExtensions() const
{
    QXmppElementList extensions;

    QXmppElement caps;
    caps.setTagName("c");
    caps.setAttribute("xmlns", ns_capabilities);
    caps.setAttribute("hash", "sha-1");
    caps.setAttribute("node", capabilitiesNode);
    caps.setAttribute("ver", capabilities().verificationString().toBase64());
    extensions << caps;

    return extensions;
}

