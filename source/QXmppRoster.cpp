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


#include "QXmppRoster.h"
#include "utils.h"
#include "QXmppRosterIq.h"
#include "QXmppPresence.h"
#include "QXmppStream.h"

QXmppRoster::QXmppRoster(QXmppStream* stream) : m_stream(stream)
{
}

QXmppRoster::~QXmppRoster()
{

}

void QXmppRoster::presenceReceived(const QXmppPresence& presence)
{
    QString jid = presence.getFrom();
    QString bareJid = jidToBareJid(jid);
    QString resource = jidToResource(jid);

    m_presences[bareJid][resource] = presence;

    emit presenceChanged(bareJid, resource);
}

void QXmppRoster::rosterIqReceived(const QXmppRosterIq& rosterIq)
{
    switch(rosterIq.getType())
    {
    case QXmppIq::Set:
    case QXmppIq::Result:
        {
            QList<QXmppRosterIq::Item> items = rosterIq.getItems();
            for(int i = 0; i < items.count(); ++i)
            {
                QString bareJid = items.at(i).getBareJid();
                m_entries[bareJid].setBareJid(bareJid);
                m_entries[bareJid].setName(items.at(i).getSubscriptionStatus());
                m_entries[bareJid].setSubscriptionType(
                    static_cast<QXmppRosterEntry::SubscriptionType>(items.at(i).getSubscriptionType()));
                m_entries[bareJid].setSubscriptionStatus(items.at(i).getSubscriptionStatus());
                m_entries[bareJid].setGroups(items.at(i).getGroups());
                emit rosterChanged(bareJid);
            }
            if(rosterIq.getType() == QXmppIq::Set) // send result iq
            {
                QXmppIq returnIq(QXmppIq::Result);
                returnIq.setId(rosterIq.getId());
                m_stream->sendPacket(returnIq);
            }
            break;
        }
    }
}

QString QXmppRoster::QXmppRosterEntry::getBareJid() const
{
    return m_bareJid;
}

QString QXmppRoster::QXmppRosterEntry::getName() const
{
    return m_name;
}

QXmppRoster::QXmppRosterEntry::SubscriptionType QXmppRoster::QXmppRosterEntry::getSubscriptionType() const
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

void QXmppRoster::QXmppRosterEntry::setBareJid(const QString& str)
{
    m_bareJid = str;
}

void QXmppRoster::QXmppRosterEntry::setName(const QString& str)
{
    m_name = str;
}

void QXmppRoster::QXmppRosterEntry::setSubscriptionType(QXmppRosterEntry::SubscriptionType type)
{
    m_type = type;
}

void QXmppRoster::QXmppRosterEntry::setSubscriptionStatus(const QString& str)
{
    m_subscriptionStatus = str;
}

void QXmppRoster::QXmppRosterEntry::addGroupEntry(const QString& str)
{
    m_groups << str;
}

void QXmppRoster::QXmppRosterEntry::setGroups(const QSet<QString>& groups)
{
    m_groups = groups;
}

QStringList QXmppRoster::getRosterBareJids() const
{
    return m_entries.keys();
}

QXmppRoster::QXmppRosterEntry QXmppRoster::getRosterEntry(const QString& bareJid) const
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

QMap<QString, QXmppRoster::QXmppRosterEntry> QXmppRoster::getRosterEntries() const
{
    return m_entries;
}

QStringList QXmppRoster::getResources(const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
    {
        return m_presences[bareJid].keys();
    }
    else
        return QStringList();
}

QMap<QString, QXmppPresence> QXmppRoster::getAllPresencesForBareJid(const QString& bareJid) const
{
    if(m_presences.contains(bareJid))
        return m_presences[bareJid];
    else
    {
        qWarning("QXmppRoster::getAllPresences(): invalid bareJid");
        return QMap<QString, QXmppPresence>();
    }
}

QXmppPresence QXmppRoster::getPresence(const QString& bareJid, const QString& resource) const
{
    if(m_presences.contains(bareJid) && m_presences[bareJid].contains(resource))
        return m_presences[bareJid][resource];
    else
    {
        qWarning("QXmppRoster::getPresence(): invalid bareJid");
        return QXmppPresence();
    }
}

QMap<QString, QMap<QString, QXmppPresence> > QXmppRoster::getAllPresences() const
{
    return m_presences;
}
