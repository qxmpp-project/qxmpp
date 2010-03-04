/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#include "QXmppStream.h"
#include "QXmppPacket.h"
#include "QXmppUtils.h"
#include "QXmppClient.h"
#include "QXmppRoster.h"
#include "QXmppPresence.h"
#include "QXmppIq.h"
#include "QXmppBind.h"
#include "QXmppSession.h"
#include "QXmppRosterIq.h"
#include "QXmppMessage.h"
#include "QXmppConstants.h"
#include "QXmppVCard.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppInformationRequestResult.h"
#include "QXmppIbbIq.h"
#include "QXmppRpcIq.h"
#include "QXmppArchiveIq.h"
#include "QXmppByteStreamIq.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppPingIq.h"
#include "QXmppLogger.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppTransferManager.h"
#include "QXmppVersionIq.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QStringList>
#include <QRegExp>
#include <QHostAddress>
#include <QXmlStreamWriter>

static const QByteArray streamRootElementStart = "<?xml version=\"1.0\"?><stream:stream xmlns:stream=\"http://etherx.jabber.org/streams\" version=\"1.0\" xmlns=\"jabber:client\" xml:lang=\"en\" xmlns:xml=\"http://www.w3.org/XML/1998/namespace\">\n";
static const QByteArray streamRootElementEnd = "</stream:stream>";

QXmppStream::QXmppStream(QXmppClient* client)
    : QObject(client), m_client(client), m_roster(this),
    m_sessionAvaliable(false),
    m_archiveManager(m_client),
    m_transferManager(m_client),
    m_vCardManager(m_client),
    m_authStep(0)
{
    // Make sure the random number generator is seeded
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    bool check = QObject::connect(&m_socket, SIGNAL(hostFound()),
                                  this, SLOT(socketHostFound()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(connected()),
                             this, SLOT(socketConnected()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(disconnected()),
                             this, SLOT(socketDisconnected()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(readyRead()),
                             this, SLOT(socketReadReady()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket, SIGNAL(encrypted()),
                             this, SLOT(socketEncrypted()));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket,
                             SIGNAL(sslErrors(const QList<QSslError>&)), this,
                             SLOT(socketSslErrors(const QList<QSslError>&)));
    Q_ASSERT(check);
    check = QObject::connect(&m_socket,
                             SIGNAL(error(QAbstractSocket::SocketError)), this,
                             SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    check = QObject::connect(this,
                            SIGNAL(disconnected()),
                            &m_roster,
                            SLOT(disconnected()));
    Q_ASSERT(check);

    check = QObject::connect(this,
                            SIGNAL(presenceReceived(const QXmppPresence&)),
                            &m_roster,
                            SLOT(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);
    
    check = QObject::connect(this, SIGNAL(rosterIqReceived(const QXmppRosterIq&)), 
        &m_roster, SLOT(rosterIqReceived(const QXmppRosterIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(rosterRequestIqReceived(const QXmppRosterIq&)),
        &m_roster, SLOT(rosterRequestIqReceived(const QXmppRosterIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(vCardIqReceived(const QXmppVCard&)),
        &m_vCardManager, SLOT(vCardIqReceived(const QXmppVCard&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(archiveChatIqReceived(const QXmppArchiveChatIq&)),
        &m_archiveManager, SLOT(archiveChatIqReceived(const QXmppArchiveChatIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(archiveListIqReceived(const QXmppArchiveListIq&)),
        &m_archiveManager, SLOT(archiveListIqReceived(const QXmppArchiveListIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(archivePrefIqReceived(const QXmppArchivePrefIq&)),
        &m_archiveManager, SLOT(archivePrefIqReceived(const QXmppArchivePrefIq&)));
    Q_ASSERT(check);

    // XEP-0047: In-Band Bytestreams
    check = QObject::connect(this, SIGNAL(iqReceived(const QXmppIq&)),
        &m_transferManager, SLOT(iqReceived(const QXmppIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(ibbCloseIqReceived(const QXmppIbbCloseIq&)),
        &m_transferManager, SLOT(ibbCloseIqReceived(const QXmppIbbCloseIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(ibbDataIqReceived(const QXmppIbbDataIq&)),
        &m_transferManager, SLOT(ibbDataIqReceived(const QXmppIbbDataIq&)));
    Q_ASSERT(check);

    check = QObject::connect(this, SIGNAL(ibbOpenIqReceived(const QXmppIbbOpenIq&)),
        &m_transferManager, SLOT(ibbOpenIqReceived(const QXmppIbbOpenIq&)));
    Q_ASSERT(check);

    // XEP-0065: SOCKS5 Bytestreams
    check = QObject::connect(this, SIGNAL(byteStreamIqReceived(const QXmppByteStreamIq&)),
        &m_transferManager, SLOT(byteStreamIqReceived(const QXmppByteStreamIq&)));
    Q_ASSERT(check);

    // XEP-0095: Stream Initiation
    check = QObject::connect(this, SIGNAL(streamInitiationIqReceived(const QXmppStreamInitiationIq&)),
        &m_transferManager, SLOT(streamInitiationIqReceived(const QXmppStreamInitiationIq&)));
    Q_ASSERT(check);
}

QXmppStream::~QXmppStream()
{

}

QXmppConfiguration& QXmppStream::getConfiguration()
{
    return m_client->getConfiguration();
}

void QXmppStream::connect()
{
    log(QString("Connecting to: %1:%2").arg(getConfiguration().
            host()).arg(getConfiguration().port()));

    // prepare for connection
    m_authStep = 0;

    m_socket.setProxy(getConfiguration().networkProxy());
    m_socket.connectToHost(getConfiguration().
                           host(), getConfiguration().port());
}

void QXmppStream::socketSslErrors(const QList<QSslError> & error)
{
    log(QString("SSL errors"));
    for(int i = 0; i< error.count(); ++i)
        log(error.at(i).errorString());

    if (getConfiguration().ignoreSslErrors())
        m_socket.ignoreSslErrors();
}

void QXmppStream::socketHostFound()
{
    log(QString("Host found"));
    emit hostFound();
}

void QXmppStream::socketConnected()
{
    flushDataBuffer();
    log(QString("Connected"));
    emit connected();
    sendStartStream();
}

void QXmppStream::socketDisconnected()
{
    flushDataBuffer();
    log(QString("Disconnected"));
    emit disconnected();
}

void QXmppStream::socketEncrypted()
{
    log(QString("Encrypted"));
    sendStartStream();
}

void QXmppStream::socketError(QAbstractSocket::SocketError ee)
{
    m_socketError = ee;
    emit error(QXmppClient::SocketError);
    log(QString("Socket error: " + m_socket.errorString()));
}

void QXmppStream::socketReadReady()
{
    QByteArray data = m_socket.readAll();
    log("SERVER [COULD BE PARTIAL DATA]:" + data.left(20));
    parser(data);
}

void QXmppStream::sendNonSASLAuthQuery( const QString &to )
{
    QXmppNonSASLAuthTypesRequestIq authQuery;
    authQuery.setTo(to);
    authQuery.setUsername(getConfiguration().user());

    sendPacket(authQuery);
}

void QXmppStream::parser(const QByteArray& data)
{
    QDomDocument doc;
    QByteArray completeXml;

    m_dataBuffer = m_dataBuffer + data;

    if(hasStartStreamElement(m_dataBuffer))
    {
        completeXml = m_dataBuffer + streamRootElementEnd;
    }
    else if(hasEndStreamElement(data))
    {
        completeXml = streamRootElementStart + m_dataBuffer;
    }
    else
    {
        completeXml = streamRootElementStart + m_dataBuffer + streamRootElementEnd;
    }
    
    if(doc.setContent(completeXml, true))
    {
        log("SERVER:" + m_dataBuffer);
        flushDataBuffer();

        QDomElement nodeRecv = doc.documentElement().firstChildElement();

        if(nodeRecv.isNull())
        {
            QDomElement streamElement = doc.documentElement();
            if(m_streamId.isEmpty())
                m_streamId = streamElement.attribute("id");
            if(m_XMPPVersion.isEmpty())
            {
                m_XMPPVersion = streamElement.attribute("version");
                if(m_XMPPVersion.isEmpty())
                {
                    // no version specified, signals XMPP Version < 1.0.
                    // switch to old auth mechanism
                    sendNonSASLAuthQuery(doc.documentElement().attribute("from"));
                }
            }
        }
        else
        {
            //TODO: Make a login error here.
        }
        while(!nodeRecv.isNull())
        {

            QString ns = nodeRecv.namespaceURI();
            log("Namespace: " + ns + " Tag: " + nodeRecv.tagName() );
            if(m_client->handleStreamElement(nodeRecv))
            {
                // already handled by client, do nothing
            }
            else if(ns == ns_stream && nodeRecv.tagName() == "features")
            {
                bool nonSaslAvailable = nodeRecv.firstChildElement("auth").
                                         namespaceURI() == ns_authFeature;
                bool saslAvailable = nodeRecv.firstChildElement("mechanisms").
                                     namespaceURI() == ns_sasl;
                bool useSasl = getConfiguration().useSASLAuthentication();

                if(nodeRecv.firstChildElement("starttls").namespaceURI()
                    == ns_tls && !m_socket.isEncrypted())
                {
                    if(nodeRecv.firstChildElement("starttls").
                                     firstChildElement().tagName() == "required")
                    {
                        // TLS is must from the server side
                        sendStartTls();
                        return;
                    }
                    else
                    {
                        // TLS is optional from the server side
                        switch(getConfiguration().streamSecurityMode())
                        {
                        case QXmppConfiguration::TLSEnabled:
                        case QXmppConfiguration::TLSRequired:
                            sendStartTls();
                            return;
                        case QXmppConfiguration::TLSDisabled:
                            break;
                        }
                    }
                }
                else if(!m_socket.isEncrypted())    // TLS not supported by server
                {
                    if(getConfiguration().streamSecurityMode() ==
                       QXmppConfiguration::TLSRequired)
                    {
                        // disconnect as the for client TLS is compulsory but
                        // not available on the server
                        //
                        log(QString("Disconnecting as TLS not available at the server"));
                        disconnect();
                        return;
                    }
                }

                if((saslAvailable && nonSaslAvailable && !useSasl) ||
                   (!saslAvailable && nonSaslAvailable))
                {
                    sendNonSASLAuthQuery(doc.documentElement().attribute("from"));
                }
                else if(saslAvailable)
                {
                    // SASL Authentication
                    QDomElement element = nodeRecv.firstChildElement("mechanisms");
                    log(QString("Mechanisms:"));
                    QDomElement subElement = element.firstChildElement();
                    QStringList mechanisms;
                    while(!subElement.isNull())
                    {
                        if(subElement.tagName() == "mechanism")
                        {
                            log(subElement.text());
                            mechanisms << subElement.text();
                        }
                        subElement = subElement.nextSiblingElement();
                    }

                    switch(getConfiguration().sASLAuthMechanism())
                    {
                    case QXmppConfiguration::SASLPlain:
                        if(mechanisms.contains("PLAIN"))
                        {
                            sendAuthPlain();
                            break;
                        }
                    case QXmppConfiguration::SASLDigestMD5:
                        if(mechanisms.contains("DIGEST-MD5"))
                        {
                            sendAuthDigestMD5();
                            break;
                        }
                    default:
                        log(QString("Desired SASL Auth mechanism not available trying the available ones"));
                        if(mechanisms.contains("DIGEST-MD5"))
                            sendAuthDigestMD5();
                        else if(mechanisms.contains("PLAIN"))
                            sendAuthPlain();
                        else
                        {
                            log(QString("SASL Auth mechanism not available"));
                            disconnect();
                            return;
                        }
                        break;
                    }
                }

                if(nodeRecv.firstChildElement("bind").
                                     namespaceURI() == ns_bind)
                {
                    sendBindIQ();
                }

                if(nodeRecv.firstChildElement("session").
                                     namespaceURI() == ns_session)
                {
                    m_sessionAvaliable = true;
                }
            }
            else if(ns == ns_stream && nodeRecv.tagName() == "error")
            {
                if (!nodeRecv.firstChildElement("conflict").isNull())
                    m_xmppStreamError = QXmppClient::ConflictStreamError;
                else
                    m_xmppStreamError = QXmppClient::UnknownStreamError;
                emit error(QXmppClient::XmppStreamError);
            }
            else if(ns == ns_tls)
            {
                if(nodeRecv.tagName() == "proceed")
                {
                    log(QString("Starting encryption"));
                    m_socket.startClientEncryption();
                    return;
                }
            }
            else if(ns == ns_sasl)
            {
                if(nodeRecv.tagName() == "success")
                {
                    log(QString("Authenticated"));
                    sendStartStream();
                }
                else if(nodeRecv.tagName() == "challenge")
                {
                    // TODO: Track which mechanism was used for when other SASL protocols which use challenges are supported
                    m_authStep++;
                    switch (m_authStep)
                    {
                    case 1 :
                        sendAuthDigestMD5ResponseStep1(nodeRecv.text());
                        break;
                    case 2 :
                        sendAuthDigestMD5ResponseStep2();
                        break;
                    default :
                        disconnect();
                        log(QString("Too many authentication steps"));
                        break;
                    }
                }
                else if(nodeRecv.tagName() == "failure")
                {
                    disconnect();
                    log(QString("Authentication failure")); 
                }
            }
            else if(ns == ns_client)
            {

                if(nodeRecv.tagName() == "iq")
                {
                    QDomElement element = nodeRecv.firstChildElement();
                    QString id = nodeRecv.attribute("id");
                    QString to = nodeRecv.attribute("to");
                    QString from = nodeRecv.attribute("from");
                    QString type = nodeRecv.attribute("type");
                    if(type.isEmpty())
                        qWarning("QXmppStream: iq type can't be empty");
                    QXmppIq iqPacket;    // to emit

                    if( QXmppRpcInvokeIq::isRpcInvokeIq( nodeRecv ) )
                    {
                        QXmppRpcInvokeIq rpcIqPacket;
                        rpcIqPacket.parse(nodeRecv);
                        m_client->invokeInterfaceMethod(rpcIqPacket);
                    }
                    else if ( QXmppRpcResponseIq::isRpcResponseIq( nodeRecv ) )
                    {
                        QXmppRpcResponseIq rpcResponseIq;
                        rpcResponseIq.parse(nodeRecv);
                        emit rpcCallResponse( rpcResponseIq );
                    }
                    else if ( QXmppRpcErrorIq::isRpcErrorIq( nodeRecv ) )
                    {
                        QXmppRpcErrorIq rpcErrorIq;
                        rpcErrorIq.parse(nodeRecv);
                        emit rpcCallError( rpcErrorIq );
                    }
                    else if(id == m_sessionId)
                    {
                        // get back add configuration whether to send
                        // roster and intial presence in beginning
                        // process SessionIq

                        // xmpp connection made
                        emit xmppConnected();

                        sendRosterRequest();
                        sendInitialPresence();

                        QXmppBind session(type);
                        session.setId(id);
                        session.setTo(to);
                        session.setFrom(from);
                        iqPacket = session;
                    }
                    else if(id == m_bindId)
                    {
                        QXmppBind bind(type);
                        QString jid = nodeRecv.firstChildElement("bind").
                                      firstChildElement("jid").text();
                        bind.setResource(jidToResource(jid));
                        bind.setJid(jidToBareJid(jid));
                        bind.setId(id);
                        bind.setTo(to);
                        bind.setFrom(from);
                        processBindIq(bind);
                        iqPacket = bind;
                    }
                    else if(nodeRecv.firstChildElement("query").
                            namespaceURI() == ns_roster)
                    {
                        QDomElement itemElement = nodeRecv.
                                                  firstChildElement("query").
                                                  firstChildElement("item");
                        QXmppRosterIq rosterIq(nodeRecv.attribute("type"));
                        rosterIq.setId(id);
                        rosterIq.setTo(to);
                        rosterIq.setFrom(from);
                        while(!itemElement.isNull())
                        {
                            QXmppRosterIq::Item item;
                            item.setName(itemElement.attribute("name"));
                            item.setBareJid(itemElement.attribute("jid"));
                            item.setSubscriptionTypeFromStr(
                                    itemElement.attribute("subscription"));
                            item.setSubscriptionStatus(
                                    itemElement.attribute("ask"));
                            item.addGroup(
                                    itemElement.firstChildElement("group").firstChildElement().text());
                            rosterIq.addItem(item);
                            itemElement = itemElement.nextSiblingElement();
                        }
                        processRosterIq(rosterIq);
                        iqPacket = rosterIq;
                    }
                    // extensions

                    // XEP-0030: Service Discovery
                    else if(QXmppDiscoveryIq::isDiscoveryIq(nodeRecv))
                    {
                        QXmppDiscoveryIq discoIq;
                        discoIq.parse(nodeRecv);

                        if (discoIq.type() == QXmppIq::Get &&
                            discoIq.queryType() == QXmppDiscoveryIq::InfoQuery)
                        {
                            // respond to info query
                            QXmppInformationRequestResult qxmppFeatures;
                            qxmppFeatures.setId(id);
                            qxmppFeatures.setTo(from);
                            qxmppFeatures.setFrom(to);
                            sendPacket(qxmppFeatures);
                        } else {
                            emit discoveryIqReceived(discoIq);
                        }

                        iqPacket = discoIq;
                    }
                    // XEP-0047 In-Band Bytestreams
                    else if(QXmppIbbCloseIq::isIbbCloseIq(nodeRecv))
                    {
                        QXmppIbbCloseIq ibbCloseIq;
                        ibbCloseIq.parse(nodeRecv);
                        emit ibbCloseIqReceived(ibbCloseIq);
                        iqPacket = ibbCloseIq;
                    }
                    else if(QXmppIbbDataIq::isIbbDataIq(nodeRecv))
                    {
                        QXmppIbbDataIq ibbDataIq;
                        ibbDataIq.parse(nodeRecv);
                        emit ibbDataIqReceived(ibbDataIq);
                        iqPacket = ibbDataIq;
                    }
                    else if(QXmppIbbOpenIq::isIbbOpenIq(nodeRecv))
                    {
                        QXmppIbbOpenIq ibbOpenIq;
                        ibbOpenIq.parse(nodeRecv);
                        emit ibbOpenIqReceived(ibbOpenIq);
                        iqPacket = ibbOpenIq;
                    }
                    // XEP-0054: vcard-temp
                    else if(nodeRecv.firstChildElement("vCard").
                            namespaceURI() == ns_vcard)
                    {
                        QXmppVCard vcardIq;
                        vcardIq.parse(nodeRecv);
                        emit vCardIqReceived(vcardIq);
                        iqPacket = vcardIq;
                    }
                    // XEP-0065: SOCKS5 Bytestreams
                    else if(QXmppByteStreamIq::isByteStreamIq(nodeRecv))
                    {
                        QXmppByteStreamIq byteStreamIq;
                        byteStreamIq.parse(nodeRecv);
                        emit byteStreamIqReceived(byteStreamIq);
                        iqPacket = byteStreamIq;
                    }
                    // XEP-0078: Non-SASL Authentication
                    else if(id == m_nonSASLAuthId && type == "result")
                    {
                        // successful Non-SASL Authentication
                        log(QString("Authenticated (Non-SASL)"));

                        emit xmppConnected();

                        sendRosterRequest();
                        sendInitialPresence();
                    }
                    else if(nodeRecv.firstChildElement("query").
                            namespaceURI() == ns_auth)
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
                                if(getConfiguration().nonSASLAuthMechanism() ==
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
                                //TODO Login error
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
                            responseIq.setId(id);
                            responseIq.setTo(from);
                            responseIq.setName(qApp->applicationName());
                            responseIq.setVersion(qApp->applicationVersion());
                            sendPacket(responseIq);
                        } else {
                            emit versionIqReceived(versionIq);
                        }

                        iqPacket = versionIq;
                    }
                    // XEP-0095: Stream Initiation
                    else if(QXmppStreamInitiationIq::isStreamInitiationIq(nodeRecv))
                    {
                        QXmppStreamInitiationIq siIq;
                        siIq.parse(nodeRecv);
                        emit streamInitiationIqReceived(siIq);
                        iqPacket = siIq;
                    }
                    // XEP-0136: Message Archiving
                    else if(QXmppArchiveChatIq::isArchiveChatIq(nodeRecv))
                    {
                        QXmppArchiveChatIq archiveIq;
                        archiveIq.parse(nodeRecv);
                        emit archiveChatIqReceived(archiveIq);
                        iqPacket = archiveIq;
                    }
                    else if(QXmppArchiveListIq::isArchiveListIq(nodeRecv))
                    {
                        QXmppArchiveListIq archiveIq;
                        archiveIq.parse(nodeRecv);
                        emit archiveListIqReceived(archiveIq);
                        iqPacket = archiveIq;
                    }
                    else if(QXmppArchivePrefIq::isArchivePrefIq(nodeRecv))
                    {
                        QXmppArchivePrefIq archiveIq;
                        archiveIq.parse(nodeRecv);
                        emit archivePrefIqReceived(archiveIq);
                        iqPacket = archiveIq;
                    }
                    // XEP-0199: XMPP Ping
                    else if(QXmppPingIq::isPingIq(nodeRecv))
                    {
                        QXmppIq iq(QXmppIq::Result);
                        iq.setId(id);
                        iq.setTo(from);
                        iq.setFrom(to);
                        sendPacket(iq);
                    }
                    else
                    {
                        // if we didn't understant the iq, reply with error
                        // except for "result" and "error" iqs
                        if (type != "result" && type != "error")
                        {
                            QXmppIq iq(QXmppIq::Error);
                            iq.setId(id);
                            iq.setTo(from);
                            iq.setFrom(to);
                            QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                                QXmppStanza::Error::FeatureNotImplemented);
                            iq.setError(error);
                            sendPacket(iq);
                        }

                        iqPacket.parse(nodeRecv);
                    }

                    processIq(iqPacket);
                }
                else if(nodeRecv.tagName() == "presence")
                {
                    QXmppPresence presence;
                    presence.parse(nodeRecv);

                    processPresence(presence);
                }
                else if(nodeRecv.tagName() == "message")
                {
                    QXmppMessage message;
                    message.parse(nodeRecv);

                    processMessage(message);
                }
            }
            nodeRecv = nodeRecv.nextSiblingElement();
        }
    }
    else
    {
        //wait for complete packet
    }
}


void QXmppStream::sendStartStream()
{
    QByteArray data = "<?xml version='1.0'?><stream:stream to='";
    data.append(getConfiguration().domain());
    data.append("' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>");
    sendToServer(data);
}

void QXmppStream::sendToServer(const QByteArray& packet)
{
    log("CLIENT: " + packet);
    m_socket.write( packet );
}

bool QXmppStream::hasStartStreamElement(const QByteArray& data)
{
    QString str(data);
    QRegExp regex("(<\\?xml.*\\?>)?\\s*<stream:stream.*>");
    regex.setMinimal(true);
    if(str.contains(regex))
        return true;
    else
        return false;
}

bool QXmppStream::hasEndStreamElement(const QByteArray& data)
{
    QString str(data);
    QRegExp regex("</stream:stream>");
    regex.setMinimal(true);
    if(str.contains(regex))
        return true;
    else
        return false;
}

void QXmppStream::sendStartTls()
{
    sendToServer("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
}

void QXmppStream::sendNonSASLAuth(bool plainText)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setUsername(getConfiguration().user());
    authQuery.setPassword(getConfiguration().passwd());
    authQuery.setResource(getConfiguration().resource());
    authQuery.setStreamId(m_streamId);
    authQuery.setUsePlainText(plainText);
    m_nonSASLAuthId = authQuery.id();
    sendPacket(authQuery);
}

void QXmppStream::sendAuthPlain()
{
    QByteArray data = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>";
    QString userPass('\0' + getConfiguration().user() +
                     '\0' + getConfiguration().passwd());
    data += userPass.toUtf8().toBase64();
    data += "</auth>";
    sendToServer(data);
}

void QXmppStream::sendAuthDigestMD5()
{
    QByteArray packet = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='DIGEST-MD5'/>";
    sendToServer(packet);
}

// challenge is BASE64 encoded string
void QXmppStream::sendAuthDigestMD5ResponseStep1(const QString& challenge)
{
    QByteArray ba = QByteArray::fromBase64(challenge.toUtf8());

    //log(ba);

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
                    log(key + ":" + value);
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
        disconnect();
        log(QString("sendAuthDigestMD5ResponseStep1: Invalid input"));
        return;
    }

    QByteArray user = getConfiguration().user().toUtf8();
    QByteArray passwd = getConfiguration().passwd().toUtf8();
    QByteArray domain = getConfiguration().domain().toUtf8();
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

    log(response);
    QByteArray packet = "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
                        + response.toBase64() + "</response>";
    sendToServer(packet);
}

void QXmppStream::sendAuthDigestMD5ResponseStep2()
{
    sendToServer("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
}

void QXmppStream::sendBindIQ()
{
    QXmppBind bind(QXmppIq::Set);
    bind.setResource(getConfiguration().resource());
    m_bindId = bind.id();
    sendPacket(bind);
}

void QXmppStream::sendSessionIQ()
{
    QXmppSession session(QXmppIq::Set);
    session.setTo(getConfiguration().domain());
    m_sessionId = session.id();
    sendPacket(session);
}

void QXmppStream::sendInitialPresence()
{
    if(m_client)
        sendPacket(m_client->getClientPresence());
}

void QXmppStream::acceptSubscriptionRequest(const QString& from, bool accept)
{
    QXmppPresence presence;
    presence.setTo(from);
    if(accept)
        presence.setType(QXmppPresence::Subscribed);
    else
        presence.setType(QXmppPresence::Unsubscribed);

    sendPacket(presence);
}

void QXmppStream::sendSubscriptionRequest(const QString& to)
{
    if(to.isEmpty())
        return;

    QXmppPresence presence;
    presence.setTo(to);
    presence.setType(QXmppPresence::Subscribe);
    sendPacket(presence);
}

void QXmppStream::sendRosterRequest()
{
    QXmppRosterIq roster(QXmppIq::Get);
    roster.setFrom(getConfiguration().jid());
    m_rosterReqId = roster.id();
    sendPacket(roster);
}

void QXmppStream::disconnect()
{
    m_authStep = 0;
    sendEndStream();
    m_socket.flush();
    m_socket.disconnectFromHost();
}

QXmppRoster& QXmppStream::getRoster()
{
    return m_roster;
}

void QXmppStream::sendPacket(const QXmppPacket& packet)
{
    if(QXmppLogger::getLogger()->getLoggingType() != QXmppLogger::NONE)
    {
        QByteArray logPacket;
        QXmlStreamWriter xmlStreamLog(&logPacket);
        packet.toXml(&xmlStreamLog);
        log("CLIENT: "+ logPacket);
    }

    QXmlStreamWriter xmlStream(&m_socket);
    packet.toXml(&xmlStream);
}

void QXmppStream::processPresence(const QXmppPresence& presence)
{
    switch(presence.getType())
    {
    case QXmppPresence::Error:
        break;
    case QXmppPresence::Available:
        break;
    case QXmppPresence::Unavailable:
        break;
    case QXmppPresence::Subscribe:
        if(!presence.from().isEmpty())
        {
            if(getConfiguration().autoAcceptSubscriptions())
                acceptSubscriptionRequest(presence.from());
            emit subscriptionRequestReceived(presence.from());
        }
        break;
    case QXmppPresence::Unsubscribe:
        break;
    case QXmppPresence::Unsubscribed:
        break;
    case QXmppPresence::Probe:
        break;
    default:
        break;
    }
    emit presenceReceived(presence);
}

void QXmppStream::processMessage(const QXmppMessage& message)
{
    emit messageReceived(message);
}

void QXmppStream::processIq(const QXmppIq& iq)
{
    emit iqReceived(iq);
}

void QXmppStream::sendEndStream()
{
    sendToServer(streamRootElementEnd);
}

void QXmppStream::processBindIq(const QXmppBind& bind)
{
    switch(bind.type())
    {
    case QXmppIq::Result:
        if(!bind.resource().isEmpty())
            getConfiguration().setResource(bind.resource());
        if(m_sessionAvaliable)
            sendSessionIQ();
        break;
    default:
        break;
    }
}

void QXmppStream::processRosterIq(const QXmppRosterIq& rosterIq)
{
    if(m_rosterReqId == rosterIq.id())
        emit rosterRequestIqReceived(rosterIq);
    else
        emit rosterIqReceived(rosterIq);

    switch(rosterIq.type())
    {
    case QXmppIq::Set:
        // when contact subscribes user...user sends 'subscribed' presence 
        // then after recieving following iq user requests contact for subscription
        
        // check thet "from" is newly added in the roster...and remove this ask thing...and do this for all items
        if(rosterIq.getItems().at(0).getSubscriptionType() ==
           QXmppRosterIq::Item::From && rosterIq.getItems().at(0).
           getSubscriptionStatus().isEmpty())
            sendSubscriptionRequest(rosterIq.getItems().at(0).getBareJid());
        break;
    default:
        break;
    }
}

QAbstractSocket::SocketError QXmppStream::getSocketError()
{
    return m_socketError;
}

QXmppClient::StreamError QXmppStream::getXmppStreamError()
{
    return m_xmppStreamError;
}

QXmppVCardManager& QXmppStream::getVCardManager()
{
    return m_vCardManager;
}

void QXmppStream::flushDataBuffer()
{
    m_dataBuffer.clear();
}

QXmppArchiveManager& QXmppStream::getArchiveManager()
{
    return m_archiveManager;
}

QXmppTransferManager& QXmppStream::getTransferManager()
{
    return m_transferManager;
}
