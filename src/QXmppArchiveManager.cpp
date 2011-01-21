/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QDomElement>

#include "QXmppArchiveIq.h"
#include "QXmppArchiveManager.h"
#include "QXmppClient.h"

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

bool QXmppArchiveManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "iq")
        return false;

    // XEP-0136: Message Archiving
    if(QXmppArchiveChatIq::isArchiveChatIq(element))
    {
        QXmppArchiveChatIq archiveIq;
        archiveIq.parse(element);
        archiveChatIqReceived(archiveIq);
        return true;
    }
    else if(QXmppArchiveListIq::isArchiveListIq(element))
    {
        QXmppArchiveListIq archiveIq;
        archiveIq.parse(element);
        archiveListIqReceived(archiveIq);
        return true;
    }
    else if(QXmppArchivePrefIq::isArchivePrefIq(element))
    {
        QXmppArchivePrefIq archiveIq;
        archiveIq.parse(element);
        archivePrefIqReceived(archiveIq);
        return true;
    }

    return false;
}

/// Retrieves the list of available collections. Once the results are
/// received, the archiveListReceived() signal will be emitted.
///
/// \param jid Optional JID if you only want conversations with a specific JID.
/// \param start Optional start time.
/// \param end Optional end time.
/// \param max Optional maximum.
///
void QXmppArchiveManager::listCollections(const QString &jid, const QDateTime &start, const QDateTime &end, int max)
{
    QXmppArchiveListIq packet;
    packet.setMax(max);
    packet.setWith(jid);
    packet.setStart(start);
    packet.setEnd(end);
    client()->sendPacket(packet);
}

/// Retrieves the specified collection. Once the results are received,
/// the archiveChatReceived() will be emitted.
///
/// \param jid The JID of the collection
/// \param start The start time of the collection.
/// \param max Optional maximum number of messages to retrieve.
///
void QXmppArchiveManager::retrieveCollection(const QString &jid, const QDateTime &start, int max)
{
    QXmppArchiveRetrieveIq packet;
    packet.setMax(max);
    packet.setStart(start);
    packet.setWith(jid);
    client()->sendPacket(packet);
}

#if 0
void QXmppArchiveManager::getPreferences()
{
    QXmppArchivePrefIq packet;
    client()->sendPacket(packet);
}
#endif
