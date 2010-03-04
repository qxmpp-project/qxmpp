/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
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


#include "QXmppRoster.h"
#include "QXmppUtils.h"
#include "QXmppRosterIq.h"
#include "QXmppPresence.h"
#include "QXmppStream.h"

QXmppRoster::QXmppRoster(QXmppStream* stream) : m_stream(stream),
                                m_isRosterReceived(false)
{
}

QXmppRoster::~QXmppRoster()
{

}

void QXmppRoster::disconnected()
{
    m_entries = QMap<QString, QXmppRoster::QXmppRosterEntry>();
    m_presences = QMap<QString, QMap<QString, QXmppPresence> >();
    m_isRosterReceived = false;
}

void QXmppRoster::presenceReceived(const QXmppPresence& presence)
{
    QString jid = presence.from();
    QString bareJid = jidToBareJid(jid);
    QString resource = jidToResource(jid);

    if (presence.getType() == QXmppPresence::Available)
        m_presences[bareJid][resource] = presence;
    else if (presence.getType() == QXmppPresence::Unavailable)
        m_presences[bareJid].remove(resource);
    else
        return;

    emit presenceChanged(bareJid, resource);
}

void QXmppRoster::rosterIqReceived(const QXmppRosterIq& rosterIq)
{
    switch(rosterIq.type())
    {
    case QXmppIq::Set:
    case QXmppIq::Result:
        {
            QList<QXmppRosterIq::Item> items = rosterIq.getItems();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).getBareJid();
                m_entries[bareJid].setBareJid(bareJid);
                m_entries[bareJid].setName(items.at(i).getName());
                m_entries[bareJid].setSubscriptionType(
                    static_cast<QXmppRosterEntry::SubscriptionType>(
                            items.at(i).getSubscriptionType()));
                m_entries[bareJid].setSubscriptionStatus(
                        items.at(i).getSubscriptionStatus());
                m_entries[bareJid].setGroups(items.at(i).getGroups());
                emit rosterChanged(bareJid);
            }
            if(rosterIq.type() == QXmppIq::Set) // send result iq
            {
                QXmppIq returnIq(QXmppIq::Result);
                returnIq.setId(rosterIq.id());
                m_stream->sendPacket(returnIq);
            }
            break;
        }
    default:
        break;
    }
}

void QXmppRoster::rosterRequestIqReceived(const QXmppRosterIq& rosterIq)
{
    switch(rosterIq.type())
    {
    case QXmppIq::Set:
    case QXmppIq::Result:
        {
            QList<QXmppRosterIq::Item> items = rosterIq.getItems();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).getBareJid();
                m_entries[bareJid].setBareJid(bareJid);
                m_entries[bareJid].setName(items.at(i).getName());
                m_entries[bareJid].setSubscriptionType(
                    static_cast<QXmppRosterEntry::SubscriptionType>(
                            items.at(i).getSubscriptionType()));
                m_entries[bareJid].setSubscriptionStatus(
                        items.at(i).getSubscriptionStatus());
                m_entries[bareJid].setGroups(items.at(i).getGroups());
            }
            if(rosterIq.type() == QXmppIq::Set) // send result iq
            {
                QXmppIq returnIq(QXmppIq::Result);
                returnIq.setId(rosterIq.id());
                m_stream->sendPacket(returnIq);
            }
            m_isRosterReceived = true;
            emit rosterReceived();
            break;
        }
    default:
        break;
    }
}

/// Returns the bareJid of the roster entry.
///
/// \return bareJid as a QString
///

QString QXmppRoster::QXmppRosterEntry::bareJid() const
{
    return m_bareJid;
}

/// Returns the name of the roster entry.
///
/// \return name as a QString
///

QString QXmppRoster::QXmppRosterEntry::name() const
{
    return m_name;
}

/// Returns the subscription type of the roster entry.
///
/// \return QXmppRosterEntry::SubscriptionType
///

QXmppRoster::QXmppRosterEntry::SubscriptionType
        QXmppRoster::QXmppRosterEntry::subscriptionType() const
{
    return m_type;
}

/// Sets the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \return subscription status as a QString
///
///

QString QXmppRoster::QXmppRosterEntry::subscriptionStatus() const
{
    return m_subscriptionStatus;
}

/// Returns the groups of the roster entry.
///
/// \return QSet<QString> list of all the groups
///

QSet<QString> QXmppRoster::QXmppRosterEntry::groups() const
{
    return m_groups;
}

/// Sets the bareJid of the roster entry.
///
/// \param bareJid as a QString
///

void QXmppRoster::QXmppRosterEntry::setBareJid(const QString& bareJid )
{
    m_bareJid = bareJid ;
}

/// Sets the name of the roster entry.
///
/// \param name as a QString
///

void QXmppRoster::QXmppRosterEntry::setName(const QString& name)
{
    m_name = name;
}

/// Sets the subscription type of the roster entry.
///
/// \param type as a QXmppRosterEntry::SubscriptionType
///

void QXmppRoster::QXmppRosterEntry::setSubscriptionType(
        QXmppRosterEntry::SubscriptionType type)
{
    m_type = type;
}

/// Sets the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \param status as a QString
///

void QXmppRoster::QXmppRosterEntry::setSubscriptionStatus(const QString& status)
{
    m_subscriptionStatus = status;
}

/// Adds the group entry of the roster entry.
///
/// \param group name as a QString
///

void QXmppRoster::QXmppRosterEntry::addGroupEntry(const QString& group)
{
    m_groups << group;
}

/// Sets the groups of the roster entry.
///
/// \param groups list of all the groups as a QSet<QString>
///

void QXmppRoster::QXmppRosterEntry::setGroups(const QSet<QString>& groups)
{
    m_groups = groups;
}

/// Function to get all the bareJids present in the roster.
///
/// \return QStringList list of all the bareJids
///

QStringList QXmppRoster::getRosterBareJids() const
{
    return m_entries.keys();
}

/// Returns the roster entry of the given bareJid. If the bareJid is not in the
/// database and empty QXmppRoster::QXmppRosterEntry will be returned.
///
/// \param bareJid as a QString
/// \return QXmppRoster::QXmppRosterEntry
///

QXmppRoster::QXmppRosterEntry QXmppRoster::getRosterEntry(
        const QString& bareJid) const
{
    // will return blank entry if bareJid does'nt exist
    if(m_entries.contains(bareJid))
        return m_entries.value(bareJid);
    else
    {
        qWarning("QXmppRoster::getRosterEntry(): bareJid doesn't exist in roster db");
        return QXmppRoster::QXmppRosterEntry();
    }
}

/// [OBSOLETE] Returns all the roster entries in the database.
///
/// \return Map of bareJid and its respective QXmppRoster::QXmppRosterEntry
///
/// \note This function is obsolete, use getRosterBareJids() and
/// getRosterEntry() to get all the roster entries.
///

QMap<QString, QXmppRoster::QXmppRosterEntry>
        QXmppRoster::getRosterEntries() const
{
    return m_entries;
}

/// Get all the associated resources with the given bareJid.
///
/// \param bareJid as a QString
/// \return list of associated resources as a QStringList
///

QStringList QXmppRoster::getResources(const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
    {
        return m_presences[bareJid].keys();
    }
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

QMap<QString, QXmppPresence> QXmppRoster::getAllPresencesForBareJid(
        const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
        return m_presences[bareJid];
    else
    {
        qWarning("QXmppRoster::getAllPresences(): invalid bareJid");
        return QMap<QString, QXmppPresence>();
    }
}

/// Get the presence of the given resource of the given bareJid.
///
/// \param bareJid as a QString
/// \param resource as a QString
/// \return QXmppPresence
///

QXmppPresence QXmppRoster::getPresence(const QString& bareJid,
                                       const QString& resource) const
{
    if(m_presences.contains(bareJid) && m_presences[bareJid].contains(resource))
        return m_presences[bareJid][resource];
    else
    {
        qWarning("QXmppRoster::getPresence(): invalid bareJid");
        return QXmppPresence();
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

QMap<QString, QMap<QString, QXmppPresence> > QXmppRoster::getAllPresences() const
{
    return m_presences;
}

/// Function to check whether the roster has been received or not.
///
/// \return true if roster received else false

bool QXmppRoster::isRosterReceived()
{
    return m_isRosterReceived;
}

QString QXmppRoster::QXmppRosterEntry::getBareJid() const
{
    return m_bareJid;
}

QString QXmppRoster::QXmppRosterEntry::getName() const
{
    return m_name;
}

QXmppRoster::QXmppRosterEntry::SubscriptionType
        QXmppRoster::QXmppRosterEntry::getSubscriptionType() const
{
    return m_type;
}

QString QXmppRoster::QXmppRosterEntry::getSubscriptionStatus() const
{
    return m_subscriptionStatus;
}

QSet<QString> QXmppRoster::QXmppRosterEntry::getGroups() const
{
    return m_groups;
}
