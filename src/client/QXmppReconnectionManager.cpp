/*
 * Copyright (C) 2008-2011 The QXmpp developers
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


#include "QXmppReconnectionManager.h"
#include "QXmppClient.h"
#include "QXmppLogger.h"
#include "QXmppUtils.h"

QXmppReconnectionManager::QXmppReconnectionManager(QXmppClient* client) :
        QObject(client),
        m_receivedConflict(false),
        m_reconnectionTries(0),
        m_timer(this),
        m_client(client)
{
    m_timer.setSingleShot(true);
    bool check = connect(&m_timer, SIGNAL(timeout()), SLOT(reconnect()));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

void QXmppReconnectionManager::connected()
{
    m_receivedConflict = false;
    m_reconnectionTries = 0;
}

void QXmppReconnectionManager::error(QXmppClient::Error error)
{
    if(m_client && error == QXmppClient::XmppStreamError)
    {
        // if we receive a resource conflict, inhibit reconnection
        if(m_client->xmppStreamError() == QXmppStanza::Error::Conflict)
            m_receivedConflict = true;
    }
    else if(m_client && error == QXmppClient::SocketError && !m_receivedConflict)
    {
        int time = getNextReconnectingInTime();

        // time is in sec
        m_timer.start(time*1000);
        emit reconnectingIn(time);
    }
    else if (m_client && error == QXmppClient::KeepAliveError)
    {
        // if we got a keepalive error, reconnect in one second
        m_timer.start(1000);
    }
}

int QXmppReconnectionManager::getNextReconnectingInTime()
{
    int reconnectingIn;
    if(m_reconnectionTries < 5)
        reconnectingIn = 10;
    else if(m_reconnectionTries < 10)
        reconnectingIn = 20;
    else if(m_reconnectionTries < 15)
        reconnectingIn = 40;
    else
        reconnectingIn = 60;

    return reconnectingIn;
}

void QXmppReconnectionManager::reconnect()
{
    if(m_client)
    {
        emit reconnectingNow();
        m_client->connectToServer(m_client->configuration(), m_client->clientPresence());
    }
}

void QXmppReconnectionManager::cancelReconnection()
{
    m_timer.stop();
    m_receivedConflict = false;
    m_reconnectionTries = 0;
}
