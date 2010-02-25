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


/// \class QXmppRoster
/// \brief Class for handling the roster of the connected client.
///
/// \note It's object should not be created using it's constructor. Instead
/// QXmppClient::getRoster() should be used to get the reference of instantiated
/// object this class.
///
/// It stores all the Roster and Presence details of all the roster entries (that
/// is all the bareJids) in the client's friend's list. It provides the
/// functionality to get all the bareJids in the client's roster and Roster and
/// Presence details of the same.
///
/// After the sucessfull xmpp connection that after the signal QXmppClient::connected()
/// is emitted QXmpp requests for getting the roster. Once QXmpp receives the roster
/// the signal QXmppRoster::rosterReceived() is emitted and after that user can
/// use the functions of this class to get roster entries.
///
/// Function QXmppRoster::isRosterReceived() tells whether the roster has been
/// received or not.
///
/// Signals presenceChanged() or rosterChanged() are emitted whenever presence
/// or roster changes respectively.
///

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
        /// An enumeration for type of subscription with the bareJid in the roster.
        enum SubscriptionType
        {
            None = 1,   ///< the user does not have a subscription to the
                        ///< contact's presence information, and the contact does
                        ///< not have a subscription to the user's presence information
            Both,   ///< both the user and the contact have subscriptions to each
                    ///< other's presence information
            From,   ///< the contact has a subscription to the user's presence information,
                    ///< but the user does not have a subscription to the contact's presence information
            To,     ///< the user has a subscription to the contact's presence information,
                    ///< but the contact does not have a subscription to the user's presence information
            Remove  ///< to delete a roster item
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
        // can be subscribe/unsubscribe (attribute "ask")
        QString m_subscriptionStatus;
        QSet<QString> m_groups;
    };

    QXmppRoster(QXmppStream* stream);
    ~QXmppRoster();
    
    bool isRosterReceived();
    QStringList getRosterBareJids() const;
    QXmppRoster::QXmppRosterEntry getRosterEntry(const QString& bareJid) const;
    QMap<QString, QXmppRoster::QXmppRosterEntry> getRosterEntries() const;
    
    QStringList getResources(const QString& bareJid) const;
    QMap<QString, QMap<QString, QXmppPresence> > getAllPresences() const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(
            const QString& bareJid) const;
    QXmppPresence getPresence(const QString& bareJid,
                              const QString& resource) const;

signals:
    /// This signal is emitted when the Roster IQ is received after a successful
    /// connection.
    void rosterReceived();

    /// This signal is emitted when the presence of a particular bareJid and resource changes.
    void presenceChanged(const QString& bareJid, const QString& resource);

    /// This signal is emitted when the roster entry of a particular bareJid changes.
    void rosterChanged(const QString& bareJid);

private:
    //reverse pointer to stream
    QXmppStream* m_stream;
    //map of bareJid and its rosterEntry
    QMap<QString, QXmppRoster::QXmppRosterEntry> m_entries;
    // map of resources of the jid and map of resouces and presences
    QMap<QString, QMap<QString, QXmppPresence> > m_presences;
    // flag to store that QXmppRoster has been populated
    bool m_isRosterReceived;

private slots:
    void disconnected();
    void presenceReceived(const QXmppPresence&);
    void rosterIqReceived(const QXmppRosterIq&);
    void rosterRequestIqReceived(const QXmppRosterIq&);
};

#endif // QXMPPROSTER_H
