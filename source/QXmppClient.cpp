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


#include "QXmppClient.h"
#include "QXmppStream.h"
#include "QXmppRoster.h"
#include "QXmppMessage.h"
#include "QXmppReconnectionManager.h"

QXmppClient::QXmppClient(QObject *parent)
    : QObject(parent), m_stream(0), m_clientPrecence(QXmppPresence::Available),
    m_reconnectionManager(0)
{
    m_stream = new QXmppStream(this);

    bool check = connect(m_stream, SIGNAL(messageReceived(const QXmppMessage&)),
                         this, SIGNAL(messageReceived(const QXmppMessage&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(presenceReceived(const QXmppPresence&)),
                    this, SIGNAL(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(iqReceived(const QXmppIq&)), this,
        SIGNAL(iqReceived(const QXmppIq&)));

    check = connect(m_stream, SIGNAL(disconnected()), this,
        SIGNAL(disconnected()));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(xmppConnected()), this,
        SIGNAL(connected()));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(error(QXmppClient::Error)), this,
        SIGNAL(error(QXmppClient::Error)));
    Q_ASSERT(check);

    check = setReconnectionManager(new QXmppReconnectionManager(this));
    Q_ASSERT(check);
}

QXmppClient::~QXmppClient()
{
}

QXmppConfiguration& QXmppClient::getConfiguration()
{
    return m_config;
}

void QXmppClient::connectToServer(const QString& host, const QString& user,
                                  const QString& passwd, const QString& domain,
                                  int port,
                                  const QXmppPresence& initialPresence)
{
    m_config.setHost(host);
    m_config.setUser(user);
    m_config.setPasswd(passwd);
    m_config.setDomain(domain);
    m_config.setPort(port);

    m_clientPrecence = initialPresence;

    m_stream->connect();
}

void QXmppClient::connectToServer(const QXmppConfiguration& config,
                                  const QXmppPresence& initialPresence)
{
    m_config = config;

    m_clientPrecence = initialPresence;

    m_stream->connect();
}

void QXmppClient::sendPacket(const QXmppPacket& packet)
{
    if(m_stream)
    {
        m_stream->sendPacket(packet);
    }
}

void QXmppClient::disconnect()
{
    m_clientPrecence.setType(QXmppPresence::Unavailable);
    m_clientPrecence.getStatus().setType(QXmppPresence::Status::Online);
    m_clientPrecence.getStatus().setStatusText("Logged out");
    sendPacket(m_clientPrecence);
    if(m_stream)
        m_stream->disconnect();
}

QXmppRoster& QXmppClient::getRoster()
{
    if(m_stream)
        return m_stream->getRoster();
}

void QXmppClient::sendMessage(const QString& bareJid, const QString& message)
{
    QStringList resources = getRoster().getResources(bareJid);
    for(int i = 0; i < resources.size(); ++i)
    {
        sendPacket(QXmppMessage("", bareJid + "/" + resources.at(i), message));
    }
}

// sets the new presence of the connected client
void QXmppClient::setClientPresence(const QXmppPresence& presence)
{
    m_clientPrecence = presence;
    sendPacket(m_clientPrecence);
}

// overloaded function, changes the status text
void QXmppClient::setClientPresence(const QString& statusText)
{
    m_clientPrecence.getStatus().setStatusText(statusText);
    sendPacket(m_clientPrecence);
}

// overloaded function, changes the presence type
void QXmppClient::setClientPresence(QXmppPresence::Type presenceType)
{
    if(presenceType == QXmppPresence::Unavailable)
    {
        disconnect();
    }
    else
    {
        m_clientPrecence.setType(presenceType);
        sendPacket(m_clientPrecence);
    }
}

// overloaded function, changes the status type
void QXmppClient::setClientPresence(QXmppPresence::Status::Type statusType)
{
    m_clientPrecence.getStatus().setType(statusType);
    sendPacket(m_clientPrecence);
}

// returnsn the referece to client presence object
const QXmppPresence& QXmppClient::getClientPresence() const
{
    return m_clientPrecence;
}

QXmppReconnectionManager* QXmppClient::getReconnectionManager()
{
    return m_reconnectionManager;
}

bool QXmppClient::setReconnectionManager(QXmppReconnectionManager*
                                         reconnectionManager)
{
    if(m_reconnectionManager)
        delete m_reconnectionManager;

    m_reconnectionManager = reconnectionManager;

    bool check = connect(this, SIGNAL(connected()), m_reconnectionManager,
                         SLOT(connected()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(error(QXmppClient::Error)),
                    m_reconnectionManager, SLOT(error(QXmppClient::Error)));
    Q_ASSERT(check);

    return true;
}

QAbstractSocket::SocketError QXmppClient::getSocketError()
{
    return m_stream->getSocketError();
}

QXmppVCardManager& QXmppClient::getVCardManager()
{
    return m_stream->getVCardManager();
}
