// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRosterManager.h"

#include "QXmppClient.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"
#include "QXmppUtils.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// \fn QXmppRosterManager::subscriptionRequestReceived
///
/// This signal is emitted when a JID asks to subscribe to the user's presence.
///
/// The user can either accept the request by calling acceptSubscription() or refuse it
/// by calling refuseSubscription().
///
/// \note If QXmppConfiguration::autoAcceptSubscriptions() is set to true, this
/// signal will not be emitted.
///
/// \param subscriberBareJid bare JID that wants to subscribe to the user's presence
/// \param presence presence stanza containing the reason / message (presence.statusText())
///
/// \since QXmpp 1.5
///

class QXmppRosterManagerPrivate
{
public:
    QXmppRosterManagerPrivate();

    void clear();

    // map of bareJid and its rosterEntry
    QMap<QString, QXmppRosterIq::Item> entries;

    // map of resources of the jid and map of resources and presences
    QMap<QString, QMap<QString, QXmppPresence>> presences;

    // flag to store that the roster has been populated
    bool isRosterReceived;

    // id of the initial roster request
    QString rosterReqId;
};

QXmppRosterManagerPrivate::QXmppRosterManagerPrivate()
    : isRosterReceived(false)
{
}

void QXmppRosterManagerPrivate::clear()
{
    entries.clear();
    presences.clear();
    rosterReqId.clear();
    isRosterReceived = false;
}

///
/// Constructs a roster manager.
///
QXmppRosterManager::QXmppRosterManager(QXmppClient *client)
    : d(std::make_unique<QXmppRosterManagerPrivate>())
{
    connect(client, &QXmppClient::connected,
            this, &QXmppRosterManager::_q_connected);

    connect(client, &QXmppClient::disconnected,
            this, &QXmppRosterManager::_q_disconnected);

    connect(client, &QXmppClient::presenceReceived,
            this, &QXmppRosterManager::_q_presenceReceived);
}

QXmppRosterManager::~QXmppRosterManager() = default;

///
/// Accepts an existing subscription request or pre-approves future subscription
/// requests.
///
/// You can call this method in reply to the subscriptionRequest() signal or to
/// create a pre-approved subscription.
///
/// \note Pre-approving subscription requests is only allowed, if the server
/// supports RFC6121 and advertises the 'urn:xmpp:features:pre-approval' stream
/// feature.
///
/// \sa QXmppStreamFeatures::preApprovedSubscriptionsSupported()
///
bool QXmppRosterManager::acceptSubscription(const QString &bareJid, const QString &reason)
{
    QXmppPresence presence;
    presence.setTo(bareJid);
    presence.setType(QXmppPresence::Subscribed);
    presence.setStatusText(reason);
    return client()->sendPacket(presence);
}

///
/// Upon XMPP connection, request the roster.
///
void QXmppRosterManager::_q_connected()
{
    // clear cache if stream has not been resumed
    if (client()->streamManagementState() != QXmppClient::ResumedStream) {
        d->clear();
    }

    if (!d->isRosterReceived) {
        QXmppRosterIq roster;
        roster.setType(QXmppIq::Get);
        roster.setFrom(client()->configuration().jid());

        // TODO: Request MIX annotations only when the server supports MIX-PAM.
        roster.setMixAnnotate(true);

        d->rosterReqId = roster.id();
        if (client()->isAuthenticated()) {
            client()->sendPacket(roster);
        }
    }
}

void QXmppRosterManager::_q_disconnected()
{
    // clear cache if stream cannot be resumed
    if (client()->streamManagementState() == QXmppClient::NoStreamManagement) {
        d->clear();
    }
}

/// \cond
bool QXmppRosterManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "iq" || !QXmppRosterIq::isRosterIq(element)) {
        return false;
    }

    // Security check: only server should send this iq
    // from() should be either empty or bareJid of the user
    const auto fromJid = element.attribute("from");
    if (!fromJid.isEmpty() && QXmppUtils::jidToBareJid(fromJid) != client()->configuration().jidBare()) {
        return false;
    }

    QXmppRosterIq rosterIq;
    rosterIq.parse(element);

    bool isInitial = (d->rosterReqId == rosterIq.id());
    if (isInitial) {
        d->rosterReqId.clear();
    }

    switch (rosterIq.type()) {
    case QXmppIq::Set: {
        // send result iq
        QXmppIq returnIq(QXmppIq::Result);
        returnIq.setId(rosterIq.id());
        client()->sendPacket(returnIq);

        // store updated entries and notify changes
        const auto items = rosterIq.items();
        for (const auto &item : items) {
            const QString bareJid = item.bareJid();
            if (item.subscriptionType() == QXmppRosterIq::Item::Remove) {
                if (d->entries.remove(bareJid)) {
                    // notify the user that the item was removed
                    Q_EMIT itemRemoved(bareJid);
                }
            } else {
                const bool added = !d->entries.contains(bareJid);
                d->entries.insert(bareJid, item);
                if (added) {
                    // notify the user that the item was added
                    Q_EMIT itemAdded(bareJid);
                } else {
                    // notify the user that the item changed
                    Q_EMIT itemChanged(bareJid);
                }
            }
        }
    } break;
    case QXmppIq::Result: {
        const auto items = rosterIq.items();
        for (const auto &item : items) {
            const auto bareJid = item.bareJid();
            d->entries.insert(bareJid, item);
        }
        if (isInitial) {
            d->isRosterReceived = true;
            Q_EMIT rosterReceived();
        }
        break;
    }
    default:
        break;
    }

    return true;
}
/// \endcond

void QXmppRosterManager::_q_presenceReceived(const QXmppPresence &presence)
{
    const auto jid = presence.from();
    const auto bareJid = QXmppUtils::jidToBareJid(jid);
    const auto resource = QXmppUtils::jidToResource(jid);

    if (bareJid.isEmpty()) {
        return;
    }

    switch (presence.type()) {
    case QXmppPresence::Available:
        d->presences[bareJid][resource] = presence;
        Q_EMIT presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Unavailable:
        d->presences[bareJid].remove(resource);
        Q_EMIT presenceChanged(bareJid, resource);
        break;
    case QXmppPresence::Subscribe:
        if (client()->configuration().autoAcceptSubscriptions()) {
            // accept subscription request
            acceptSubscription(bareJid);

            // ask for reciprocal subscription
            subscribe(bareJid);
        } else {
            Q_EMIT subscriptionReceived(bareJid);
            Q_EMIT subscriptionRequestReceived(bareJid, presence);
        }
        break;
    default:
        break;
    }
}

///
/// Adds a new item to the roster without sending any subscription requests.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name Optional name for the item.
/// \param groups Optional groups for the item.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppRosterManager::Result> QXmppRosterManager::addRosterItem(const QString &bareJid, const QString &name, const QSet<QString> &groups)
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setName(name);
    item.setGroups(groups);
    item.setSubscriptionType(QXmppRosterIq::Item::NotSet);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendGenericIq(std::move(iq));
}

///
/// Removes a roster item and cancels subscriptions to and from the contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemRemoved() signal to be emitted.
///
/// \param bareJid
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppRosterManager::Result> QXmppRosterManager::removeRosterItem(const QString &bareJid)
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setSubscriptionType(QXmppRosterIq::Item::Remove);

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendGenericIq(std::move(iq));
}

///
/// Renames a roster item.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppRosterManager::Result> QXmppRosterManager::renameRosterItem(const QString &bareJid, const QString &name)
{
    using Error = QXmppStanza::Error;
    if (!d->entries.contains(bareJid)) {
        return makeReadyTask<Result>(
            QXmppError { QStringLiteral("The roster doesn't contain this user."), {} });
    }

    auto item = d->entries.value(bareJid);
    item.setName(name);

    // If there is a pending subscription, do not include the corresponding attribute in the stanza.
    if (!item.subscriptionStatus().isEmpty()) {
        item.setSubscriptionStatus({});
    }

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendGenericIq(std::move(iq));
}

///
/// Requests a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppRosterManager::subscribeTo(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Subscribe);
    packet.setStatusText(reason);
    return client()->sendSensitive(std::move(packet));
}

///
/// Removes a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppRosterManager::unsubscribeFrom(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Unsubscribe);
    packet.setStatusText(reason);
    return client()->sendSensitive(std::move(packet));
}

///
/// Refuses a subscription request.
///
/// You can call this method in reply to the subscriptionRequest() signal.
///
bool QXmppRosterManager::refuseSubscription(const QString &bareJid, const QString &reason)
{
    QXmppPresence presence;
    presence.setTo(bareJid);
    presence.setType(QXmppPresence::Unsubscribed);
    presence.setStatusText(reason);
    return client()->sendPacket(presence);
}

///
/// Adds a new item  the roster without sending any subscription requests.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name Optotional name for the item.
/// \param groups Optional groups for the item.
///
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

///
/// Removes a roster item and cancels subscriptions to and from the contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemRemoved() signal to be emitted.
///
/// \param bareJid
///
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

///
/// Renames a roster item.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.
///
/// \param bareJid
/// \param name
///
bool QXmppRosterManager::renameItem(const QString &bareJid, const QString &name)
{
    if (!d->entries.contains(bareJid)) {
        return false;
    }

    auto item = d->entries.value(bareJid);
    item.setName(name);

    // If there is a pending subscription, do not include the corresponding attribute in the stanza.
    if (!item.subscriptionStatus().isEmpty()) {
        item.setSubscriptionStatus({});
    }

    QXmppRosterIq iq;
    iq.setType(QXmppIq::Set);
    iq.addItem(item);
    return client()->sendPacket(iq);
}

///
/// Requests a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemAdded() or itemChanged() signal to be emitted.
///
bool QXmppRosterManager::subscribe(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Subscribe);
    packet.setStatusText(reason);
    return client()->sendPacket(packet);
}

///
/// Removes a subscription to the given contact.
///
/// As a result, the server will initiate a roster push, causing the
/// itemChanged() signal to be emitted.
///
bool QXmppRosterManager::unsubscribe(const QString &bareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(bareJid));
    packet.setType(QXmppPresence::Unsubscribe);
    packet.setStatusText(reason);
    return client()->sendPacket(packet);
}

///
/// Function to get all the bareJids present in the roster.
///
/// \return QStringList list of all the bareJids
///
QStringList QXmppRosterManager::getRosterBareJids() const
{
    return d->entries.keys();
}

///
/// Returns the roster entry of the given bareJid. If the bareJid is not in the
/// database and empty QXmppRosterIq::Item will be returned.
///
/// \param bareJid as a QString
///
QXmppRosterIq::Item QXmppRosterManager::getRosterEntry(
    const QString &bareJid) const
{
    // will return blank entry if bareJid doesn't exist
    if (d->entries.contains(bareJid)) {
        return d->entries.value(bareJid);
    }
    return {};
}

///
/// Get all the associated resources with the given bareJid.
///
/// \param bareJid as a QString
/// \return list of associated resources as a QStringList
///
QStringList QXmppRosterManager::getResources(const QString &bareJid) const
{
    if (d->presences.contains(bareJid)) {
        return d->presences[bareJid].keys();
    }
    return {};
}

///
/// Get all the presences of all the resources of the given bareJid. A bareJid
/// can have multiple resources and each resource will have a presence
/// associated with it.
///
/// \param bareJid as a QString
/// \return Map of resource and its respective presence QMap<QString, QXmppPresence>
///
QMap<QString, QXmppPresence> QXmppRosterManager::getAllPresencesForBareJid(
    const QString &bareJid) const
{
    if (d->presences.contains(bareJid)) {
        return d->presences.value(bareJid);
    }
    return {};
}

///
/// Get the presence of the given resource of the given bareJid.
///
/// \param bareJid as a QString
/// \param resource as a QString
/// \return QXmppPresence
///
QXmppPresence QXmppRosterManager::getPresence(const QString &bareJid,
                                              const QString &resource) const
{
    if (d->presences.contains(bareJid) && d->presences[bareJid].contains(resource)) {
        return d->presences[bareJid][resource];
    }

    QXmppPresence presence;
    presence.setType(QXmppPresence::Unavailable);
    return presence;
}

///
/// Function to check whether the roster has been received or not.
///
/// On disconnecting this is reset to false if no stream management is used by
/// the client and so the stream cannot be resumed later.
///
/// \return true if roster received else false
///
bool QXmppRosterManager::isRosterReceived() const
{
    return d->isRosterReceived;
}
