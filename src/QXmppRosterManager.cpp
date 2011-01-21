/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#include "QXmppClient.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"

/// Constructs a roster manager.

QXmppRosterManager::QXmppRosterManager(QXmppClient* client)
    : m_isRosterReceived(false)
{
    bool check = QObject::connect(client, SIGNAL(connected()),
        this, SLOT(connected()));
    Q_ASSERT(check);

    check = QObject::connect(client, SIGNAL(disconnected()),
        this, SLOT(disconnected()));
    Q_ASSERT(check);

    check = QObject::connect(client, SIGNAL(presenceReceived(const QXmppPresence&)),
        this, SLOT(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);
}

/// Upon XMPP connection, request the roster.
///
void QXmppRosterManager::connected()
{
    QXmppRosterIq roster;
    roster.setType(QXmppIq::Get);
    roster.setFrom(client()->configuration().jid());
    m_rosterReqId = roster.id();
    client()->sendPacket(roster);
}

void QXmppRosterManager::disconnected()
{
    m_entries.clear();
    m_presences.clear();
    m_isRosterReceived = false;
}

bool QXmppRosterManager::handleStanza(const QDomElement &element)
{
    if(element.tagName() == "iq" && QXmppRosterIq::isRosterIq(element))
    {
        QXmppRosterIq rosterIq;
        rosterIq.parse(element);

        // Security check: only server should send this iq
        // from() should be either empty or bareJid of the user
        QString fromJid = rosterIq.from();
        if(fromJid.isEmpty() ||
           jidToBareJid(fromJid) == client()->configuration().jidBare())
        {
            rosterIqReceived(rosterIq);
            return true;
        }
    }

    return false;
}

void QXmppRosterManager::presenceReceived(const QXmppPresence& presence)
{
    QString jid = presence.from();
    QString bareJid = jidToBareJid(jid);
    QString resource = jidToResource(jid);

    if (bareJid.isEmpty())
        return;

    switch(presence.type())
    {
    case QXmppPresence::Available:
        m_presences[bareJid][resource] = presence;
        emit presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Unavailable:
        m_presences[bareJid].remove(resource);
        emit presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Subscribe:
        if (client()->configuration().autoAcceptSubscriptions())
        {
            // accept subscription request
            QXmppPresence presence;
            presence.setTo(jid);
            presence.setType(QXmppPresence::Subscribed);
            client()->sendPacket(presence);

            // ask for reciprocal subscription
            presence.setTo(bareJid);
            presence.setType(QXmppPresence::Subscribe);
            client()->sendPacket(presence);
        }
        break;
    default:
        break;
    }
}

/// Removes a roster entry and cancels subscriptions to and from the contact.
///
/// As a result, the server will initiate a roster push, causing the
/// rosterChanged() signal to be emitted.
///
/// \param bareJid

void QXmppRosterManager::removeRosterEntry(const QString &bareJid)
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setSubscriptionType(QXmppRosterIq::Item::Remove);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    client()->sendPacket(iq);
}

void QXmppRosterManager::rosterIqReceived(const QXmppRosterIq& rosterIq)
{
    bool isInitial = (m_rosterReqId == rosterIq.id());

    switch(rosterIq.type())
    {
    case QXmppIq::Set:
        {
            // send result iq
            QXmppIq returnIq(QXmppIq::Result);
            returnIq.setId(rosterIq.id());
            client()->sendPacket(returnIq);

            // store updated entries and notify changes
            const QList<QXmppRosterIq::Item> items = rosterIq.items();
            for (int i = 0; i < items.count(); i++)
            {
                QString bareJid = items.at(i).bareJid();
                m_entries[bareJid] = items.at(i);
                emit rosterChanged(bareJid);
            }
        }
        break;
    case QXmppIq::Result:
        {
            QList<QXmppRosterIq::Item> items = rosterIq.items();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).bareJid();
                m_entries[bareJid] = items.at(i);
                if (!isInitial)
                    emit rosterChanged(bareJid);
            }
            if (isInitial)
            {
                m_isRosterReceived = true;
                emit rosterReceived();
            }
            break;
        }
    default:
        break;
    }
}

/// Function to get all the bareJids present in the roster.
///
/// \return QStringList list of all the bareJids
///

QStringList QXmppRosterManager::getRosterBareJids() const
{
    return m_entries.keys();
}

/// Returns the roster entry of the given bareJid. If the bareJid is not in the
/// database and empty QXmppRosterIq::Item will be returned.
///
/// \param bareJid as a QString
///

QXmppRosterIq::Item QXmppRosterManager::getRosterEntry(
        const QString& bareJid) const
{
    // will return blank entry if bareJid does'nt exist
    if(m_entries.contains(bareJid))
        return m_entries.value(bareJid);
    else
        return QXmppRosterIq::Item();
}

/// [OBSOLETE] Returns all the roster entries in the database.
///
/// \return Map of bareJid and its respective QXmppRosterIq::Item
///
/// \note This function is obsolete, use getRosterBareJids() and
/// getRosterEntry() to get all the roster entries.
///

QMap<QString, QXmppRosterIq::Item>
        QXmppRosterManager::getRosterEntries() const
{
    return m_entries;
}

/// Get all the associated resources with the given bareJid.
///
/// \param bareJid as a QString
/// \return list of associated resources as a QStringList
///

QStringList QXmppRosterManager::getResources(const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
        return m_presences[bareJid].keys();
    else
        return QStringList();
}

/// Get all the presences of all the resources of the given bareJid. A bareJid
/// can have multiple resources and each resource will have a presence
/// associated with it.
///
/// \param bareJid as a QString
/// \return Map of resource and its respective presence QMap<QString, QXmppPresence>
///

QMap<QString, QXmppPresence> QXmppRosterManager::getAllPresencesForBareJid(
        const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
        return m_presences[bareJid];
    else
        return QMap<QString, QXmppPresence>();
}

/// Get the presence of the given resource of the given bareJid.
///
/// \param bareJid as a QString
/// \param resource as a QString
/// \return QXmppPresence
///

QXmppPresence QXmppRosterManager::getPresence(const QString& bareJid,
                                       const QString& resource) const
{
    if(m_presences.contains(bareJid) && m_presences[bareJid].contains(resource))
        return m_presences[bareJid][resource];
    else
    {
        QXmppPresence presence;
        presence.setType(QXmppPresence::Unavailable);
        presence.setStatus(QXmppPresence::Status::Offline);
        return presence;
    }
}

/// [OBSOLETE] Returns all the presence entries in the database.
///
/// \return Map of bareJid and map of resource and its presence that is
/// QMap<QString, QMap<QString, QXmppPresence> >
///
/// \note This function is obsolete, use getRosterBareJids(), getResources()
/// and getPresence() or getAllPresencesForBareJid()
/// to get all the presence entries.

QMap<QString, QMap<QString, QXmppPresence> > QXmppRosterManager::getAllPresences() const
{
    return m_presences;
}

/// Function to check whether the roster has been received or not.
///
/// \return true if roster received else false

bool QXmppRosterManager::isRosterReceived()
{
    return m_isRosterReceived;
}

