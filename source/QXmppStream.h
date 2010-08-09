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


#ifndef QXMPPSTREAM_H
#define QXMPPSTREAM_H

#include <QObject>
#include <QSslSocket>
#include "QXmppClient.h"
#include "QXmppConfiguration.h"
#include "QXmppLogger.h"
#include "QXmppStanza.h"

class QDomElement;
class QTimer;

class QXmppClient;
class QXmppPacket;
class QXmppPresence;
class QXmppIq;
class QXmppBind;
class QXmppRosterIq;
class QXmppVCard;
class QXmppMessage;
class QXmppRpcResponseIq;
class QXmppRpcErrorIq;
class QXmppArchiveChatIq;
class QXmppArchiveListIq;
class QXmppArchivePrefIq;
class QXmppByteStreamIq;
class QXmppDiscoveryIq;
class QXmppIbbCloseIq;
class QXmppIbbDataIq;
class QXmppIbbOpenIq;
class QXmppJingleIq;
class QXmppMucAdminIq;
class QXmppMucOwnerIq;
class QXmppStreamInitiationIq;
class QXmppStreamPrivate;
class QXmppVersionIq;

class QXmppStream : public QObject
{
    Q_OBJECT

public:
    QXmppStream(QObject *parent);
    ~QXmppStream();
    void connect();
    void acceptSubscriptionRequest(const QString& from, bool accept = true);
    void sendSubscriptionRequest(const QString& to);
    void disconnect();
    bool isConnected() const;
    bool sendPacket(const QXmppPacket&);

    QAbstractSocket::SocketError socketError();
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppConfiguration& configuration();

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    QXmppElementList presenceExtensions() const;

signals:
    // socket host found
    void hostFound();

    // socket connected
    void connected();

    // socket disconnected
    void disconnected();

    // xmpp connected
    void xmppConnected();

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

    void error(QXmppClient::Error);
    void elementReceived(const QDomElement &element, bool &handled);
    void presenceReceived(const QXmppPresence&);
    void messageReceived(const QXmppMessage&);
    void iqReceived(const QXmppIq&);
    void rosterIqReceived(const QXmppRosterIq&);
    void vCardIqReceived(const QXmppVCard&);

    void rpcCallInvoke(const QXmppRpcInvokeIq &invoke);
    void rpcCallResponse(const QXmppRpcResponseIq& result);
    void rpcCallError(const QXmppRpcErrorIq &err);

    void archiveChatIqReceived(const QXmppArchiveChatIq&);
    void archiveListIqReceived(const QXmppArchiveListIq&);
    void archivePrefIqReceived(const QXmppArchivePrefIq&);

    void discoveryIqReceived(const QXmppDiscoveryIq&);

    void byteStreamIqReceived(const QXmppByteStreamIq&);
    void ibbCloseIqReceived(const QXmppIbbCloseIq&);
    void ibbDataIqReceived(const QXmppIbbDataIq&);
    void ibbOpenIqReceived(const QXmppIbbOpenIq&);
    void streamInitiationIqReceived(const QXmppStreamInitiationIq&);

    // XEP-0045: Multi-User Chat
    void mucAdminIqReceived(const QXmppMucAdminIq&);
    void mucOwnerIqReceived(const QXmppMucOwnerIq&);

    // XEP-0166: Jingle
    void jingleIqReceived(const QXmppJingleIq&);

private slots:
    void socketHostFound();
    void socketReadReady();
    void socketEncrypted();
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError>&);

    void pingStart();
    void pingStop();
    void pingSend();
    void pingTimeout();

private:
    QXmppConfiguration m_config;    ///< This object provides the configuration
                                    ///< required for connecting to the XMPP server.
    QXmppLogger* m_logger;
    QString m_sessionId;
    QString m_bindId;
    QByteArray m_dataBuffer;
    bool m_sessionAvailable;
    QString m_streamId;
    QString m_nonSASLAuthId;
    QString m_XMPPVersion;
    QXmppStanza::Error::Condition m_xmppStreamError;
    QTimer *m_pingTimer;
    QTimer *m_timeoutTimer;

    int m_authStep;

    QXmppDiscoveryIq capabilities() const;
    void debug(const QString&);
    void info(const QString&);
    void warning(const QString&);
    void parser(const QByteArray&);
    void sendStartStream();
    void sendEndStream();
    void sendNonSASLAuth(bool);
    void sendNonSASLAuthQuery( const QString &to );
    void sendAuthDigestMD5ResponseStep1(const QString& challenge);
    void sendAuthDigestMD5ResponseStep2();
    void sendBindIQ();
    void sendSessionIQ();
    void sendRosterRequest();
    bool sendToServer(const QByteArray&);
    bool hasStartStreamElement(const QByteArray&);
    bool hasEndStreamElement(const QByteArray&);

    void flushDataBuffer();

    QXmppStreamPrivate * const d;
};

#endif // QXMPPSTREAM_H
