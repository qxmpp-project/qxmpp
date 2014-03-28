/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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
#include "QXmppConstants.h"

/// \cond
QStringList QXmppArchiveManager::discoveryFeatures() const
{
    // XEP-0036: Message Archiving
    return QStringList() << ns_archive;
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
        emit archiveChatReceived(archiveIq.chat(), archiveIq.resultSetReply());
        return true;
    }
    else if(QXmppArchiveListIq::isArchiveListIq(element))
    {
        QXmppArchiveListIq archiveIq;
        archiveIq.parse(element);
        emit archiveListReceived(archiveIq.chats(), archiveIq.resultSetReply());
        return true;
    }
    else if(QXmppArchivePrefIq::isArchivePrefIq(element))
    {
        // TODO: handle preference iq
        QXmppArchivePrefIq archiveIq;
        archiveIq.parse(element);
        return true;
    }

    return false;
}
/// \endcond

/// Retrieves the list of available collections. Once the results are
/// received, the archiveListReceived() signal will be emitted.
///
/// \param jid JID you want conversations with.
/// \param start Optional start time.
/// \param end Optional end time.
/// \param rsm Optional Result Set Management query
///
void QXmppArchiveManager::listCollections(const QString& jid, const QDateTime& start,
                                          const QDateTime& end, const QXmppResultSetQuery &rsm)
{
    QXmppArchiveListIq packet;
    packet.setResultSetQuery(rsm);
    packet.setWith(jid);
    packet.setStart(start);
    packet.setEnd(end);
    client()->sendPacket(packet);
}

/// \overload
/// Retrieves the list of available collections. Once the results are
/// received, the archiveListReceived() signal will be emitted.
///
/// \param jid JID you want conversations with.
/// \param start Start time.
/// \param end End time.
/// \param max Maximum number of collections to list.
///
void QXmppArchiveManager::listCollections(const QString &jid, const QDateTime &start, const QDateTime &end, int max)
{
    QXmppResultSetQuery rsm;
    rsm.setMax(max);
    listCollections(jid, start, end, rsm);
}


/// Removes the specified collection(s).
///
/// \param jid The JID of the collection
/// \param start Optional start time.
/// \param end Optional end time.
///
void QXmppArchiveManager::removeCollections(const QString &jid, const QDateTime &start, const QDateTime &end)
{
    QXmppArchiveRemoveIq packet;
    packet.setType(QXmppIq::Set);
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
/// \param rsm Optional Result Set Management query
///
void QXmppArchiveManager::retrieveCollection(const QString &jid, const QDateTime &start, const QXmppResultSetQuery &rsm)
{
    QXmppArchiveRetrieveIq packet;
    packet.setResultSetQuery(rsm);
    packet.setStart(start);
    packet.setWith(jid);
    client()->sendPacket(packet);
}

/// \overload
/// Retrieves the specified collection. Once the results are received,
/// the archiveChatReceived() will be emitted.
///
/// \param jid The JID of the collection
/// \param start The start time of the collection.
/// \param max Maximum number of messages to retrieve.
///
void QXmppArchiveManager::retrieveCollection(const QString &jid, const QDateTime &start, int max)
{
    QXmppResultSetQuery rsm;
    rsm.setMax(max);
    retrieveCollection(jid, start, rsm);
}

#if 0
void QXmppArchiveManager::getPreferences()
{
    QXmppArchivePrefIq packet;
    client()->sendPacket(packet);
}
#endif
