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
#include "QXmppStream.h"

#include <QDebug>

QXmppArchiveManager::QXmppArchiveManager(QXmppStream *stream, QObject *parent)
    : QObject(parent),
    m_stream(stream)
{
    bool check = QObject::connect(m_stream, SIGNAL(archiveChatIqReceived(const QXmppArchiveChatIq&)),
        this, SLOT(archiveChatIqReceived(const QXmppArchiveChatIq&)));
    Q_ASSERT(check);

    check = QObject::connect(m_stream, SIGNAL(archiveListIqReceived(const QXmppArchiveListIq&)),
        this, SLOT(archiveListIqReceived(const QXmppArchiveListIq&)));
    Q_ASSERT(check);

    check = QObject::connect(m_stream, SIGNAL(archivePrefIqReceived(const QXmppArchivePrefIq&)),
        this, SLOT(archivePrefIqReceived(const QXmppArchivePrefIq&)));
    Q_ASSERT(check);
}

void QXmppArchiveManager::archiveChatIqReceived(const QXmppArchiveChatIq &chatIq)
{
    emit archiveChatReceived(chatIq.chat());
}

void QXmppArchiveManager::archiveListIqReceived(const QXmppArchiveListIq &listIq)
{
    emit archiveListReceived(listIq.chats());
}

void QXmppArchiveManager::archivePrefIqReceived(const QXmppArchivePrefIq &prefIq)
{
    Q_UNUSED(prefIq);
}

void QXmppArchiveManager::listCollections(const QString &jid, const QDateTime &start, const QDateTime &end, int max)
{
    QXmppArchiveListIq packet;
    packet.setMax(max);
    packet.setWith(jid);
    packet.setStart(start);
    packet.setEnd(end);
    m_stream->sendPacket(packet);
}

void QXmppArchiveManager::retrieveCollection(const QString &jid, const QDateTime &start, int max)
{
    QXmppArchiveRetrieveIq packet;
    packet.setMax(max);
    packet.setStart(start);
    packet.setWith(jid);
    m_stream->sendPacket(packet);
}

void QXmppArchiveManager::getPreferences()
{
    QXmppArchivePrefIq packet;
    m_stream->sendPacket(packet);
}
