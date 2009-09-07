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


#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include "QXmppConfiguration.h"
#include "QXmppPresence.h"

class QXmppStream;
class QXmppPresence;
class QXmppMessage;
class QXmppPacket;
class QXmppIq;
class QXmppRoster;
class QXmppReconnectionManager;
class QXmppVCardManager;

class QXmppClient : public QObject
{
    Q_OBJECT

public:
    enum Error
    {
        SocketError,
        XmppStreamError,
        XmppStanzaError
    };

    QXmppClient(QObject *parent = 0);
    ~QXmppClient();
    void connectToServer(const QString& host,
                         const QString& user,
                         const QString& passwd,
                         const QString& domain,
                         int port = 5222,
                         const QXmppPresence& initialPresence =
                         QXmppPresence());
    void connectToServer(const QXmppConfiguration&, 
                         const QXmppPresence& initialPresence = 
                         QXmppPresence());
    void disconnect();
    QXmppRoster& getRoster();
    QXmppConfiguration& getConfiguration();
    QXmppReconnectionManager* getReconnectionManager();
    bool setReconnectionManager(QXmppReconnectionManager*);
    const QXmppPresence& getClientPresence() const;
    QXmppVCardManager& getVCardManager();

signals:
    void connected();
    void disconnected();
    void error(QXmppClient::Error);
    void messageReceived(const QXmppMessage&);
    void presenceReceived(const QXmppPresence&);
    void iqReceived(const QXmppIq&);

public:
    QAbstractSocket::SocketError getSocketError();
//    QXmppStanza::Error getXmppStreamError();

public slots:
    void sendPacket(const QXmppPacket&);
    void sendMessage(const QString& bareJid, const QString& message);

    void setClientPresence(const QXmppPresence&);
    void setClientPresence(const QString& statusText);
    void setClientPresence(QXmppPresence::Type presenceType);
    void setClientPresence(QXmppPresence::Status::Type statusType);

private:
    QXmppStream* m_stream;
    QXmppConfiguration m_config;
    QXmppPresence m_clientPrecence;
    QXmppReconnectionManager* m_reconnectionManager;
};

#endif // QXMPPCLIENT_H
