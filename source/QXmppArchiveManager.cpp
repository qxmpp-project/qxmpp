/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
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

#include "QXmppArchiveIq.h"
#include "QXmppArchiveManager.h"
#include "QXmppClient.h"

#include <QDebug>

QXmppArchiveManager::QXmppArchiveManager(QXmppClient *client) :
        QObject(client), m_client(client)
{
}

void QXmppArchiveManager::archiveChatIqReceived(const QXmppArchiveChatIq &chatIq)
{
    emit archiveChatReceived(chatIq.getChat());
}

void QXmppArchiveManager::archiveListIqReceived(const QXmppArchiveListIq &listIq)
{
    emit archiveListReceived(listIq.getChats());
}

void QXmppArchiveManager::archivePrefIqReceived(const QXmppArchivePrefIq &prefIq)
{
    qDebug() << "got archive preferences";
}

void QXmppArchiveManager::listCollections(const QString &jid, const QDateTime &start, const QDateTime &end, int max)
{
    QXmppArchiveListIq packet;
    packet.setMax(max);
    packet.setWith(jid);
    packet.setStart(start);
    packet.setEnd(end);
    m_client->sendPacket(packet);
}

void QXmppArchiveManager::retrieveCollection(const QString &jid, const QDateTime &start, int max)
{
    QXmppArchiveRetrieveIq packet;
    packet.setMax(max);
    packet.setStart(start);
    packet.setWith(jid);
    m_client->sendPacket(packet);
}

void QXmppArchiveManager::getPreferences()
{
    QXmppArchivePrefIq packet;
    m_client->sendPacket(packet);
}
