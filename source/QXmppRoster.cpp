/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Authors:
 *	Manjeet Dahiya
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
            QList<QXmppRosterIq::Item> items = rosterIq.items();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).bareJid();
                m_entries[bareJid] = items.at(i);
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
            QList<QXmppRosterIq::Item> items = rosterIq.items();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).bareJid();
                m_entries[bareJid] = items.at(i);
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

QMap<QString, QXmppPresence> QXmppRoster::getAllPresencesForBareJid(
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

