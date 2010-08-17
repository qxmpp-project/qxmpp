/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#include "QXmppConfiguration.h"
#include "QXmppConstants.h"
#include "QXmppIq.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPacket.h"
#include "QXmppPresence.h"
#include "QXmppStream.h"
#include "QXmppStreamFeatures.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppUtils.h"

// IQ types
#include "QXmppArchiveIq.h"
#include "QXmppBind.h"
#include "QXmppByteStreamIq.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppIbbIq.h"
#include "QXmppJingleIq.h"
#include "QXmppMucIq.h"
#include "QXmppPingIq.h"
#include "QXmppRpcIq.h"
#include "QXmppRosterIq.h"
#include "QXmppSession.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppVCard.h"
#include "QXmppVersionIq.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>
#include <QTimer>

static bool randomSeeded = false;
static const QString capabilitiesNode = "http://code.google.com/p/qxmpp";
static const QByteArray streamRootElementEnd = "</stream:stream>";

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    QByteArray dataBuffer;
    QXmppLogger* logger;
    QSslSocket* socket;
    QAbstractSocket::SocketError socketError;
    QXmppStanza::Error::Condition xmppStreamError;

    // State data
    int authStep;
    QString bindId;
    QString sessionId;
    bool sessionAvailable;
    QString streamId;
    QString streamFrom;
    QByteArray streamStart;
    QString streamVersion;
    QString nonSASLAuthId;

    // Timers
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

QXmppStreamPrivate::QXmppStreamPrivate()
    : logger(0),
    authStep(0),
    sessionAvailable(false)
{
}

QXmppStream::QXmppStream(QSslSocket *socket, QObject *parent)
    : QObject(parent),
    d(new QXmppStreamPrivate)
{
    // Make sure the random number generator is seeded
    if (!randomSeeded)
    {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        randomSeeded = true;
    }

    // initialise logger
    setLogger(QXmppLogger::getLogger());

    // create socket
    d->socket = socket;

    bool check = QObject::connect(d->socket, SIGNAL(hostFound()),
                                  this, SLOT(socketHostFound()));
    Q_ASSERT(check);
    check = QObject::connect(d->socket, SIGNAL(connected()),
                             this, SLOT(socketConnected()));
    Q_ASSERT(check);
    check = QObject::connect(d->socket, SIGNAL(disconnected()),
                             this, SLOT(socketDisconnected()));
    Q_ASSERT(check);
    check = QObject::connect(d->socket, SIGNAL(readyRead()),
                             this, SLOT(socketReadReady()));
    Q_ASSERT(check);
    check = QObject::connect(d->socket, SIGNAL(encrypted()),
                             this, SLOT(socketEncrypted()));
    Q_ASSERT(check);
    check = QObject::connect(d->socket,
                             SIGNAL(sslErrors(const QList<QSslError>&)), this,
                             SLOT(socketSslErrors(const QList<QSslError>&)));
    Q_ASSERT(check);
    check = QObject::connect(d->socket,
                             SIGNAL(error(QAbstractSocket::SocketError)), this,
                             SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    // XEP-0199: XMPP Ping
    d->pingTimer = new QTimer(this);
    check = QObject::connect(d->pingTimer, SIGNAL(timeout()), this, SLOT(pingSend()));
    Q_ASSERT(check);

    d->timeoutTimer = new QTimer(this);
    d->timeoutTimer->setSingleShot(true);
    check = QObject::connect(d->timeoutTimer, SIGNAL(timeout()), this, SLOT(pingTimeout()));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(xmppConnected()), this, SLOT(pingStart()));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(disconnected()), this, SLOT(pingStop()));
    Q_ASSERT(check);
}

QXmppStream::~QXmppStream()
{
    delete d;
}

QXmppConfiguration& QXmppStream::configuration()
{
    return d->config;
}

void QXmppStream::connect()
{
    info(QString("Connecting to: %1:%2").arg(configuration().
            host()).arg(configuration().port()));

    // prepare for connection
    d->authStep = 0;

    d->socket->setProxy(configuration().networkProxy());
    d->socket->connectToHost(configuration().
                           host(), configuration().port());
}

void QXmppStream::socketSslErrors(const QList<QSslError> & error)
{
    warning("SSL errors");
    for(int i = 0; i< error.count(); ++i)
        warning(error.at(i).errorString());

    if (configuration().ignoreSslErrors())
        d->socket->ignoreSslErrors();
}

void QXmppStream::socketHostFound()
{
    debug("Host found");
    emit hostFound();
}

void QXmppStream::socketConnected()
{
    flushDataBuffer();
    info("Connected");
    emit connected();
    sendStartStream();
}

void QXmppStream::socketDisconnected()
{
    flushDataBuffer();
    info("Disconnected");
    emit disconnected();
}

void QXmppStream::socketEncrypted()
{
    debug("Encrypted");
    sendStartStream();
}

void QXmppStream::socketError(QAbstractSocket::SocketError ee)
{
    d->socketError = ee;
    emit error(QXmppClient::SocketError);
    warning(QString("Socket error: " + d->socket->errorString()));
}

void QXmppStream::socketReadReady()
{
    const QByteArray data = d->socket->readAll();
    //debug("SERVER [COULD BE PARTIAL DATA]:" + data.left(20));
    parser(data);
}

/// Returns the QXmppLogger associated with the current QXmppStream.

QXmppLogger *QXmppStream::logger()
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the current QXmppStream.

void QXmppStream::setLogger(QXmppLogger *logger)
{
    if (d->logger)
        QObject::disconnect(this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
                            d->logger, SLOT(log(QXmppLogger::MessageType, QString)));
    d->logger = logger;
    if (d->logger)
        QObject::connect(this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
                         d->logger, SLOT(log(QXmppLogger::MessageType, QString)));
}

void QXmppStream::debug(const QString &data)
{
    emit logMessage(QXmppLogger::DebugMessage, data);
}

void QXmppStream::info(const QString &data)
{
    emit logMessage(QXmppLogger::InformationMessage, data);
}

void QXmppStream::warning(const QString &data)
{
    emit logMessage(QXmppLogger::WarningMessage, data);
}

void QXmppStream::parser(const QByteArray& data)
{
    d->dataBuffer.append(data);

    // FIXME : maybe these QRegExps could be static?
    QRegExp startStreamRegex("^(<\\?xml.*\\?>)?\\s*<stream:stream.*>");
    startStreamRegex.setMinimal(true);
    QRegExp endStreamRegex("</stream:stream>$");
    endStreamRegex.setMinimal(true);

    // check whether we need to add stream start / end elements
    QByteArray completeXml;
    const QString strData = QString::fromUtf8(d->dataBuffer);
    bool streamStart = false;
    if(strData.contains(startStreamRegex))
    {
        completeXml = d->dataBuffer + streamRootElementEnd;
        streamStart = true;
        d->streamStart = startStreamRegex.cap(0).toUtf8();
    }
    else if(strData.contains(endStreamRegex))
        completeXml = d->streamStart + d->dataBuffer;
    else
        completeXml = d->streamStart + d->dataBuffer + streamRootElementEnd;

    // check whether we have a valid XML document
    QDomDocument doc;
    if(!doc.setContent(completeXml, true))
        return;

    // remove data from buffer
    emit logMessage(QXmppLogger::ReceivedMessage, strData);
    flushDataBuffer();

    // process stream start
    QDomElement nodeRecv = doc.documentElement().firstChildElement();
    if (streamStart)
    {
        QDomElement streamElement = doc.documentElement();
        handleStream(streamElement);
    }

    // process stanzas
    while(!nodeRecv.isNull())
    {
        // if we receive any kind of data, stop the timeout timer
        d->timeoutTimer->stop();

        handleStanza(nodeRecv);
        nodeRecv = nodeRecv.nextSiblingElement();
    }
}

void QXmppStream::handleStream(const QDomElement &streamElement)
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

void QXmppStream::handleStanza(const QDomElement &nodeRecv)
{
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

        const bool nonSaslAvailable = features.isNonSaslAuthAvailable();
        const bool saslAvailable = !features.authMechanisms().isEmpty();
        const bool useSasl = configuration().useSASLAuthentication();

        if (!d->socket->isEncrypted())
        {
            // determine TLS mode to use
            const QXmppConfiguration::StreamSecurityMode localSecurity = configuration().streamSecurityMode();
            const QXmppConfiguration::StreamSecurityMode remoteSecurity = features.securityMode();
            if (!d->socket->supportsSsl() &&
                (localSecurity == QXmppConfiguration::TLSRequired ||
                 remoteSecurity == QXmppConfiguration::TLSRequired))
            {
                warning("Disconnecting as TLS is required, but SSL support is not available");
                disconnect();
                return;
            }
            if (localSecurity == QXmppConfiguration::TLSRequired &&
                remoteSecurity == QXmppConfiguration::TLSDisabled)
            {
                warning("Disconnecting as TLS is required, but not supported by the server");
                disconnect();
                return;
            }

            if (d->socket->supportsSsl() &&
                remoteSecurity != QXmppConfiguration::TLSDisabled && 
                localSecurity != QXmppConfiguration::TLSDisabled)
            {
                // enable TLS as it is support by both parties
                sendData("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
                return;
            }
        }

        // handle authentication
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
                disconnect();
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
            sendBindIQ();

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
            d->socket->startClientEncryption();
            return;
        }
    }
    else if(ns == ns_sasl)
    {
        if(nodeRecv.tagName() == "success")
        {
            debug("Authenticated");
            sendStartStream();
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
                disconnect();
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
            disconnect();
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
                QXmppSession session;
                session.parse(nodeRecv);

                // get back add configuration whether to send
                // roster and intial presence in beginning
                // process SessionIq

                // xmpp connection made
                emit xmppConnected();
            }
            else if(QXmppBind::isBind(nodeRecv) && id == d->bindId)
            {
                QXmppBind bind;
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
                        sendSessionIQ();
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
                emit xmppConnected();
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
                        disconnect();
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

bool QXmppStream::sendStartStream()
{
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(configuration().domain());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    return sendData(data);
}

bool QXmppStream::sendData(const QByteArray& packet)
{
    emit logMessage(QXmppLogger::SentMessage, QString::fromUtf8(packet));
    if (!isConnected())
        return false;
    return d->socket->write( packet ) == packet.size();
}

void QXmppStream::sendNonSASLAuth(bool plainText)
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

void QXmppStream::sendNonSASLAuthQuery()
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(d->streamFrom);
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(configuration().user());
    sendPacket(authQuery);
}

// challenge is BASE64 encoded string
void QXmppStream::sendAuthDigestMD5ResponseStep1(const QString& challenge)
{
    QByteArray ba = QByteArray::fromBase64(challenge.toUtf8());

    QMap<QByteArray, QByteArray> map;

    QByteArray key;
    QByteArray value;
    bool parsingValue = false;
    int startindex = 0;
    for(int i = 0; i < ba.length(); i++)
    {
        char next = ba.at(i);
        switch (next) {
            case '=':
                if (!parsingValue)
                {
                    // Trim the key, but do not trim the value as it is in delimiters
                    key = ba.mid(startindex, i - startindex).trimmed();
                    // Skip the equals and delimiter
                    startindex = i + 2;
                }
                break;
            case '"':
                // Ignore the opening delimiter
                if (startindex != (i + 1))
                {
                    value = ba.mid(startindex, i - startindex);
                    map[key] = value;
                    debug(key + ":" + value);
                    // Skip the comma
                    i += 2;
                    startindex = i;
                    parsingValue = false;
                }
                else {
                    parsingValue = true;
                }

                break;

            default:
                break;
        }
    }

    if (!map.contains("nonce"))
    {
        warning("sendAuthDigestMD5ResponseStep1: Invalid input");
        disconnect();
        return;
    }

    QByteArray user = configuration().user().toUtf8();
    QByteArray passwd = configuration().passwd().toUtf8();
    QByteArray domain = configuration().domain().toUtf8();
    QByteArray realm;
    if(map.contains("realm"))
        realm = map["realm"];

    QByteArray response;

    QByteArray cnonce(32, 'm');
    for(int n = 0; n < cnonce.size(); ++n)
            cnonce[n] = (char)(256.0*qrand()/(RAND_MAX+1.0));

    // The random data can the '=' char is not valid as it is a delimiter,
    // so to be safe, base64 the nonce
    cnonce = cnonce.toBase64();

    QByteArray nc = "00000001";
    QByteArray digest_uri = "xmpp/" + domain;

    QByteArray a1 = user + ':' + realm + ':' + passwd;
    QByteArray ha1 = QCryptographicHash::hash(a1, QCryptographicHash::Md5);
    ha1 += ':' + map["nonce"] + ':' + cnonce;

    if(map.contains("authzid"))
        ha1 += ':' + map["authzid"];

    QByteArray A1(ha1);
    QByteArray A2 = "AUTHENTICATE:" + digest_uri;
    QByteArray HA1 = QCryptographicHash::hash(A1, QCryptographicHash::Md5).toHex();
    QByteArray HA2 = QCryptographicHash::hash(A2, QCryptographicHash::Md5).toHex();
    QByteArray KD = HA1 + ':' + map["nonce"] + ':' + nc + ':' + cnonce + ':'
                    + "auth" + ':' + HA2;
    QByteArray Z = QCryptographicHash::hash(KD, QCryptographicHash::Md5).toHex();

    response += "username=\"" + user + "\",";

    if(!realm.isEmpty())
        response += "realm=\"" + realm + "\",";

    response += "nonce=\"" + map["nonce"] + "\",";
    response += "cnonce=\"" + cnonce + "\",";
    response += "nc=" + nc + ",";
    response += "qop=auth,";
    response += "digest-uri=\"" + digest_uri + "\",";
    response += "response=" + Z + ",";
    if(map.contains("authzid"))
        response += "authzid=\"" + map["authzid"] + "\",";
    response += "charset=utf-8";

    debug(response);
    QByteArray packet = "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
                        + response.toBase64() + "</response>";
    sendData(packet);
}

void QXmppStream::sendAuthDigestMD5ResponseStep2()
{
    sendData("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
}

void QXmppStream::sendBindIQ()
{
    QXmppBind bind(QXmppIq::Set);
    bind.setResource(configuration().resource());
    d->bindId = bind.id();
    sendPacket(bind);
}

void QXmppStream::sendSessionIQ()
{
    QXmppSession session(QXmppIq::Set);
    session.setTo(configuration().domain());
    d->sessionId = session.id();
    sendPacket(session);
}

void QXmppStream::disconnect()
{
    d->authStep = 0;
    sendEndStream();
    d->socket->flush();
    d->socket->disconnectFromHost();
}

bool QXmppStream::isConnected() const
{
    return d->socket->state() == QAbstractSocket::ConnectedState;
}

bool QXmppStream::sendPacket(const QXmppPacket& packet)
{
    // prepare packet
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    packet.toXml(&xmlStream);

    // send packet
    return sendData(data);
}

bool QXmppStream::sendEndStream()
{
    return sendData(streamRootElementEnd);
}

void QXmppStream::pingStart()
{
    const int interval = configuration().keepAliveInterval();
    // start ping timer
    if (interval > 0)
    {
        d->pingTimer->setInterval(interval * 1000);
        d->pingTimer->start();
    }
}

void QXmppStream::pingStop()
{
    // stop all timers
    d->pingTimer->stop();
    d->timeoutTimer->stop();
}

void QXmppStream::pingSend()
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

void QXmppStream::pingTimeout()
{
    warning("Ping timeout");
    disconnect();
    emit error(QXmppClient::KeepAliveError);
}

QAbstractSocket::SocketError QXmppStream::socketError()
{
    return d->socketError;
}

QXmppStanza::Error::Condition QXmppStream::xmppStreamError()
{
    return d->xmppStreamError;
}

void QXmppStream::flushDataBuffer()
{
    d->dataBuffer.clear();
}

QXmppDiscoveryIq QXmppStream::capabilities() const
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

QXmppElementList QXmppStream::presenceExtensions() const
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

