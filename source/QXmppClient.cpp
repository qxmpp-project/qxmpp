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

QXmppClient::QXmppClient(QObject *parent)
    : QObject(parent), m_stream(0)
{
    m_stream = new QXmppStream(this);

    bool check = connect(m_stream, SIGNAL(messageReceived(const QXmppMessage&)), this,
        SIGNAL(messageReceived(const QXmppMessage&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(presenceReceived(const QXmppPresence&)), this,
        SIGNAL(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(iqReceived(const QXmppIq&)), this,
        SIGNAL(iqReceived(const QXmppIq&)));
    Q_ASSERT(check);
}

QXmppClient::~QXmppClient()
{
}

QXmppConfiguration& QXmppClient::getConfigurgation()
{
    return m_config;
}

void QXmppClient::connectToServer(const QString& host, const QString& user, const QString& passwd,
                     const QString& domain, int port)
{
    m_config.setHost(host);
    m_config.setUser(user);
    m_config.setPasswd(passwd);
    m_config.setDomain(domain);
    m_config.setPort(port);

    m_stream->connect();
}

void QXmppClient::connectToServer(const QXmppConfiguration& config)
{
    m_config = config;
    
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
    if(m_stream)
        m_stream->disconnect();
}

QXmppRoster& QXmppClient::getRoster()
{
    return m_stream->getRoster();
}
