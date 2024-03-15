// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppArchiveManager.h"

#include "QXmppArchiveIq.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"

#include <QDomElement>

/// \cond
QStringList QXmppArchiveManager::discoveryFeatures() const
{
    // XEP-0036: Message Archiving
    return { ns_archive.toString() };
}

bool QXmppArchiveManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != u"iq") {
        return false;
    }

    // XEP-0136: Message Archiving
    if (QXmppArchiveChatIq::isArchiveChatIq(element)) {
        QXmppArchiveChatIq archiveIq;
        archiveIq.parse(element);
        Q_EMIT archiveChatReceived(archiveIq.chat(), archiveIq.resultSetReply());
        return true;
    } else if (QXmppArchiveListIq::isArchiveListIq(element)) {
        QXmppArchiveListIq archiveIq;
        archiveIq.parse(element);
        Q_EMIT archiveListReceived(archiveIq.chats(), archiveIq.resultSetReply());
        return true;
    } else if (QXmppArchivePrefIq::isArchivePrefIq(element)) {
        // TODO: handle preference iq
        QXmppArchivePrefIq archiveIq;
        archiveIq.parse(element);
        return true;
    }

    return false;
}
/// \endcond

///
/// Retrieves the list of available collections. Once the results are
/// received, the archiveListReceived() signal will be emitted.
///
/// \param jid JID you want conversations with.
/// \param start Optional start time.
/// \param end Optional end time.
/// \param rsm Optional Result Set Management query
///
void QXmppArchiveManager::listCollections(const QString &jid, const QDateTime &start,
                                          const QDateTime &end, const QXmppResultSetQuery &rsm)
{
    QXmppArchiveListIq packet;
    packet.setResultSetQuery(rsm);
    packet.setWith(jid);
    packet.setStart(start);
    packet.setEnd(end);
    client()->sendPacket(packet);
}

/// \overload
///
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

///
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

///
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
///
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
