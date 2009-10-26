/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
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
#include "QXmppConfiguration.h"
#include "QXmppRoster.h"
#include "QXmppStanza.h"
#include "QXmppVCardManager.h"

class QDomElement;

class QXmppRoster;
class QXmppClient;
class QXmppPacket;
class QXmppPresence;
class QXmppIq;
class QXmppBind;
class QXmppRosterIq;
class QXmppVCard;
class QXmppMessage;

class QXmppStream : public QObject
{
    Q_OBJECT

public:
    QXmppStream(QXmppClient* client);
    ~QXmppStream();
    void connect();
    void acceptSubscriptionRequest(const QString& from, bool accept = true);
    void sendSubscriptionRequest(const QString& to);
    void disconnect();
    QXmppRoster& getRoster();
    QXmppVCardManager& getVCardManager();
    void sendPacket(const QXmppPacket&);

    QAbstractSocket::SocketError getSocketError();

signals:
    // socket host found
    void hostFound();

    // socket connected
    void connected();

    // socket disconnected
    void disconnected();

    // xmpp connected
    void xmppConnected();

    void error(QXmppClient::Error);
    void subscriptionRequestReceived(const QString& from);
    void presenceReceived(const QXmppPresence&);
    void messageReceived(const QXmppMessage&);
    void iqReceived(const QXmppIq&);
    void rosterIqReceived(const QXmppRosterIq&);
    void vCardIqReceived(const QXmppVCard&);

private slots:
    void socketHostFound();
    void socketReadReady();
    void socketEncrypted();
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError>&);

private:
    QXmppClient* m_client; // reverse pointer
    QXmppRoster m_roster;
    QString m_sessionId;
    QString m_bindId;
    QString m_rosterReqId;
    QByteArray m_dataBuffer;
    QSslSocket m_socket;
    bool m_sessionAvaliable;
    QAbstractSocket::SocketError m_socketError;
    QString m_streamId;
    QString m_nonSASLAuthId;
    QString m_XMPPVersion;
//    m_xmppStreamError;
//    m_xmppStanzaError;


    QXmppVCardManager m_vCardManager;

    QXmppConfiguration& getConfiguration();
    void parser(const QByteArray&);
    void sendStartStream();
    void sendEndStream();
    void sendStartTls();
    void sendNonSASLAuth(bool);
    void sendNonSASLAuthQuery( const QString &to );
    void sendAuthPlain();
    void sendAuthDigestMD5();
    void sendAuthDigestMD5Response(const QString& challenge);
    void sendBindIQ();
    void sendSessionIQ();
    void sendInitialPresence();
    void sendRosterRequest();
    void sendToServer(const QByteArray&);
    bool hasStartStreamElement(const QByteArray&);
    bool hasEndStreamElement(const QByteArray&);
    QXmppStanza::Error parseStanzaError(QDomElement & errorElement);

    void processPresence(const QXmppPresence&);
    void processMessage(const QXmppMessage&);
    void processIq(const QXmppIq&);
    void processBindIq(const QXmppBind&);
    void processRosterIq(const QXmppRosterIq&);

    void flushDataBuffer();
};

#endif // QXMPPSTREAM_H
