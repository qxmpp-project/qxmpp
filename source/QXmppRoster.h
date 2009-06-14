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


#ifndef QXMPPROSTER_H
#define QXMPPROSTER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QStringList>

#include "QXmppClient.h"

class QXmppRosterIq;
class QXmppPresence;

class QXmppRoster : public QObject
{
    Q_OBJECT

public:
    class QXmppRosterEntry
    {
    public:
        enum SubscriptionType
        {
            None = 0,
            Both,
            From,
            To,
            Remove
        };

        QString getBareJid() const;
        QString getName() const;
        QXmppRosterEntry::SubscriptionType getSubscriptionType() const;
        QString getSubscriptionStatus() const;
        QSet<QString> getGroups() const;

        void setBareJid(const QString&);
        void setName(const QString&);
        void setSubscriptionType(QXmppRosterEntry::SubscriptionType);
        void setSubscriptionStatus(const QString&);
        void setGroups(const QSet<QString>&);
        void addGroupEntry(const QString&);

    private:
        QString m_bareJid;
        SubscriptionType m_type;
        QString m_name;
        QString m_subscriptionStatus;  // can be subscribe/unsubscribe (attribute "ask")
        QSet<QString> m_groups;
    };

    QXmppRoster(QXmppStream* stream);
    ~QXmppRoster();
    
    QStringList getRosterBareJids() const;
    QXmppRoster::QXmppRosterEntry getRosterEntry(const QString& bareJid) const;
    QMap<QString, QXmppRoster::QXmppRosterEntry> getRosterEntries() const;
    
    QStringList getResources(const QString& bareJid) const;
    QMap<QString, QMap<QString, QXmppPresence> > getAllPresences() const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(const QString& bareJid) const;
    QXmppPresence getPresence(const QString& bareJid, const QString& resource) const;

signals:
    void presenceChanged(const QString& bareJid, const QString& resource);
    void rosterChanged(const QString& bareJid);

private:
    QXmppStream* m_stream; //reverse pointer to stream
    QMap<QString, QXmppRoster::QXmppRosterEntry> m_entries;  //map of bareJid and its rosterEntry 
    QMap<QString, QMap<QString, QXmppPresence> > m_presences;    // map of resources of the jid and map of resouces and presences

private slots:
    void presenceReceived(const QXmppPresence&);
    void rosterIqReceived(const QXmppRosterIq&);
};

#endif // QXMPPROSTER_H
