/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#include "QXmppClient.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"

class QXmppRosterManagerPrivate
{
public:
    QXmppRosterManagerPrivate(QXmppRosterManager *qq);

    // map of bareJid and its rosterEntry
    QMap<QString, QXmppRosterIq::Item> entries;

    // map of resources of the jid and map of resources and presences
    QMap<QString, QMap<QString, QXmppPresence> > presences;

    // flag to store that the roster has been populated
    bool isRosterReceived;

    // id of the initial roster request
    QString rosterReqId;

private:
    QXmppRosterManager *q;
};

QXmppRosterManagerPrivate::QXmppRosterManagerPrivate(QXmppRosterManager *qq)
    : isRosterReceived(false),
    q(qq)
{
}

/// Constructs a roster manager.

QXmppRosterManager::QXmppRosterManager(QXmppClient* client)
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppRosterManagerPrivate(this);

    check = connect(client, SIGNAL(connected()),
                    this, SLOT(_q_connected()));
    Q_ASSERT(check);

    check = connect(client, SIGNAL(disconnected()),
                    this, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    check = connect(client, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SLOT(_q_presenceReceived(QXmppPresence)));
    Q_ASSERT(check);
}

QXmppRosterManager::~QXmppRosterManager()
{
    delete d;
}

/// Accepts a subscription request.
///
/// You can call this method in reply to the subscriptionRequest() signal.

bool QXmppRosterManager::acceptSubscription(const QString &bareJid, const QString &reason)
{
    QXmppPresence presence;
    presence.setTo(bareJid);
    presence.setType(QXmppPresence::Subscribed);
    presence.setStatusText(reason);
    return client()->sendPacket(presence);
}

/// Upon XMPP connection, request the roster.
///
void QXmppRosterManager::_q_connected()
{
    QXmppRosterIq roster;
    roster.setType(QXmppIq::Get);
    roster.setFrom(client()->configuration().jid());
    d->rosterReqId = roster.id();
    if (client()->isAuthenticated())
        client()->sendPacket(roster);
}

void QXmppRosterManager::_q_disconnected()
{
    d->entries.clear();
    d->presences.clear();
    d->isRosterReceived = false;
}

/// \cond
bool QXmppRosterManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "iq" || !QXmppRosterIq::isRosterIq(element))
        return false;

    // Security check: only server should send this iq
    // from() should be either empty or bareJid of the user
    const QString fromJid = element.attribute("from");
    if (!fromJid.isEmpty() && QXmppUtils::jidToBareJid(fromJid) != client()->configuration().jidBare())
        return false;

    QXmppRosterIq rosterIq;
    rosterIq.parse(element);

    bool isInitial = (d->rosterReqId == rosterIq.id());
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
            foreach (const QXmppRosterIq::Item &item, items) {
                const QString bareJid = item.bareJid();
                if (item.subscriptionType() == QXmppRosterIq::Item::Remove) {
                    if (d->entries.remove(bareJid)) {
                        // notify the user that the item was removed
                        emit itemRemoved(bareJid);
                    }
                } else {
                    const bool added = !d->entries.contains(bareJid);
                    d->entries.insert(bareJid, item);
                    if (added) {
                        // notify the user that the item was added
                        emit itemAdded(bareJid);
                    } else {
                        // notify the user that the item changed
                        emit itemChanged(bareJid);
                    }
                }
            }
        }
        break;
    case QXmppIq::Result:
        {
            const QList<QXmppRosterIq::Item> items = rosterIq.items();
            foreach (const QXmppRosterIq::Item &item, items) {
                const QString bareJid = item.bareJid();
                d->entries.insert(bareJid, item);
            }
            if (isInitial)
            {
                d->isRosterReceived = true;
                emit rosterReceived();
            }
            break;
        }
    default:
        break;
    }

    return true;
}
/// \endcond

void QXmppRosterManager::_q_presenceReceived(const QXmppPresence& presence)
{
    const QString jid = presence.from();
    const QString bareJid = QXmppUtils::jidToBareJid(jid);
    const QString resource = QXmppUtils::jidToResource(jid);

    if (bareJid.isEmpty())
        return;

    switch(presence.type())
    {
    case QXmppPresence::Available:
        d->presences[bareJid][resource] = presence;
        emit presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Unavailable:
        d->presences[bareJid].remove(resource);
        emit presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Subscribe:
        if (client()->configuration().autoAcceptSubscriptions())
        {
            // accept subscription request
            acceptSubscription(bareJid);

            // ask for reciprocal subscription
            subscribe(bareJid);
        } else {
            emit subscriptionReceived(bareJid);
        }
        break;
    default:
        break;
    }
}

/// Refuses a subscription request.
///
/// You can call this method in reply to the subscriptionRequest() signal.

bool QXmppRosterManager::refuseSubscription(const QString &bareJid, const QString &reason)
{
    QXmppPresence presence;
    presence.setTo(bareJid);
    presence.setType(QXmppPresence::Unsubscribed);
    presence.setStatusText(reason);
    return client()->sendPacket(presence);
}

/// Adds a new item to the roster without sending any subscription requests.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name Optional name for the item.
/// \param groups Optional groups for the item.

bool QXmppRosterManager::addItem(const QString &bareJid, const QString &name, const QSet<QString> &groups)
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setName(name);
    item.setGroups(groups);
    item.setSubscriptionType(QXmppRosterIq::Item::NotSet);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendPacket(iq);
}

/// Removes a roster item and cancels subscriptions to and from the contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemRemoved() signal to be emitted.
///
/// \param bareJid

bool QXmppRosterManager::removeItem(const QString &bareJid)
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setSubscriptionType(QXmppRosterIq::Item::Remove);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendPacket(iq);
}

/// Renames a roster item.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name

bool QXmppRosterManager::renameItem(const QString &bareJid, const QString &name)
{
    if (!d->entries.contains(bareJid))
        return false;

    QXmppRosterIq::Item item = d->entries.value(bareJid);
    item.setName(name);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendPacket(iq);
}

/// Requests a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.

bool QXmppRosterManager::subscribe(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Subscribe);
    packet.setStatusText(reason);
    return client()->sendPacket(packet);
}

/// Removes a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.

bool QXmppRosterManager::unsubscribe(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Unsubscribe);
    packet.setStatusText(reason);
    return client()->sendPacket(packet);
}

/// Function to get all the bareJids present in the roster.
///
/// \return QStringList list of all the bareJids
///

QStringList QXmppRosterManager::getRosterBareJids() const
{
    return d->entries.keys();
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
    if(d->entries.contains(bareJid))
        return d->entries.value(bareJid);
    else
        return QXmppRosterIq::Item();
}

/// Get all the associated resources with the given bareJid.
///
/// \param bareJid as a QString
/// \return list of associated resources as a QStringList
///

QStringList QXmppRosterManager::getResources(const QString& bareJid) const
{
    if(d->presences.contains(bareJid))
        return d->presences[bareJid].keys();
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
    if(d->presences.contains(bareJid))
        return d->presences[bareJid];
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
    if(d->presences.contains(bareJid) && d->presences[bareJid].contains(resource))
        return d->presences[bareJid][resource];
    else
    {
        QXmppPresence presence;
        presence.setType(QXmppPresence::Unavailable);
        return presence;
    }
}

/// Function to check whether the roster has been received or not.
///
/// \return true if roster received else false

bool QXmppRosterManager::isRosterReceived() const
{
    return d->isRosterReceived;
}
