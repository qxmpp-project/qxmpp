// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRosterManager.h"

#include "QXmppAccountMigrationManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMovedManager.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Algorithms.h"
#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp;
using namespace QXmpp::Private;

namespace QXmpp::Private {

struct RosterData {
    using Items = QList<QXmppRosterIq::Item>;

    Items items;

    static std::variant<RosterData, QXmppError> fromDom(const QDomElement &el)
    {
        if (el.tagName() != u"roster" && el.namespaceURI() != ns_qxmpp_export) {
            return QXmppError { u"Invalid element."_s, {} };
        }

        RosterData d;

        for (const auto &itemEl : iterChildElements(el, u"item", ns_roster)) {
            QXmppRosterIq::Item item;
            item.parse(itemEl);
            d.items.push_back(std::move(item));
        }

        return d;
    }

    void toXml(QXmlStreamWriter &writer) const
    {
        writer.writeStartElement(QSL65("roster"));
        for (const auto &item : items) {
            item.toXml(&writer, true);
        }
        writer.writeEndElement();
    }
};

static void serializeRosterData(const RosterData &d, QXmlStreamWriter &writer)
{
    d.toXml(writer);
}

}  // namespace QXmpp::Private

///
/// \fn QXmppRosterManager::subscriptionRequestReceived
///
/// This signal is emitted when a JID asks to subscribe to the user's presence.
///
/// The user can either accept the request by calling acceptSubscription() or refuse it
/// by calling refuseSubscription().
///
/// \note If QXmppConfiguration::autoAcceptSubscriptions() is set to true or the subscription
/// request is automatically accepted by the QXmppMovedManager, this signal will not be emitted.
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
};

QXmppRosterManagerPrivate::QXmppRosterManagerPrivate()
    : isRosterReceived(false)
{
}

void QXmppRosterManagerPrivate::clear()
{
    entries.clear();
    presences.clear();
    isRosterReceived = false;
}

///
/// Constructs a roster manager.
///
QXmppRosterManager::QXmppRosterManager(QXmppClient *client)
    : d(std::make_unique<QXmppRosterManagerPrivate>())
{
    QXmppExportData::registerExtension<RosterData, RosterData::fromDom, serializeRosterData>(u"roster", ns_qxmpp_export);

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

    if (!d->isRosterReceived && client()->isAuthenticated()) {
        requestRoster().then(this, [this](auto &&result) {
            if (auto *rosterIq = std::get_if<QXmppRosterIq>(&result)) {
                // reset entries
                d->entries.clear();
                const auto items = rosterIq->items();
                for (const auto &item : items) {
                    d->entries.insert(item.bareJid(), item);
                }

                // notify
                d->isRosterReceived = true;
                Q_EMIT rosterReceived();
            }
        });
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
    if (element.tagName() != u"iq" || !QXmppRosterIq::isRosterIq(element)) {
        return false;
    }

    // Security check: only server should send this iq
    // from() should be either empty or bareJid of the user
    const auto fromJid = element.attribute(u"from"_s);
    if (!fromJid.isEmpty() && QXmppUtils::jidToBareJid(fromJid) != client()->configuration().jidBare()) {
        return false;
    }

    QXmppRosterIq rosterIq;
    rosterIq.parse(element);

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
    case QXmppPresence::Subscribe: {
        handleSubscriptionRequest(bareJid, presence);
        break;
    }
    default:
        break;
    }
}

void QXmppRosterManager::handleSubscriptionRequest(const QString &bareJid, const QXmppPresence &presence)
{
    auto notifyOnSubscriptionRequest = [this, bareJid](const QXmppPresence &presence) {
        Q_EMIT subscriptionReceived(bareJid);
        Q_EMIT subscriptionRequestReceived(bareJid, presence);
    };

    // Automatically accept all incoming subscription requests if enabled.
    if (client()->configuration().autoAcceptSubscriptions()) {
        acceptSubscription(bareJid);
        subscribe(bareJid);
        return;
    }

    // check for XEP-0283: Moved subscription requests and verify them
    if (auto *movedManager = client()->findExtension<QXmppMovedManager>(); movedManager && !presence.oldJid().isEmpty()) {
        movedManager->processSubscriptionRequest(presence).then(this, [this, notifyOnSubscriptionRequest](QXmppPresence &&presence) mutable {
            notifyOnSubscriptionRequest(presence);
        });
    } else {
        notifyOnSubscriptionRequest(presence);
    }
}

QXmppTask<QXmppRosterManager::RosterResult> QXmppRosterManager::requestRoster()
{
    QXmppRosterIq iq;
    iq.setType(QXmppIq::Get);
    iq.setFrom(client()->configuration().jid());

    // TODO: Request MIX annotations only when the server supports MIX-PAM.
    iq.setMixAnnotate(true);

    return chainIq<RosterResult>(client()->sendIq(std::move(iq)), this);
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
    if (!d->entries.contains(bareJid)) {
        return makeReadyTask<Result>(
            QXmppError { u"The roster doesn't contain this user."_s, {} });
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

void QXmppRosterManager::onRegistered(QXmppClient *client)
{
    // data import/export
    if (auto manager = client->findExtension<QXmppAccountMigrationManager>()) {
        using ImportResult = std::variant<Success, QXmppError>;
        auto importData = [this, client, manager](const RosterData &data) -> QXmppTask<ImportResult> {
            if (data.items.isEmpty()) {
                return makeReadyTask<ImportResult>(Success());
            }

            QXmppPromise<ImportResult> promise;
            auto counter = std::make_shared<int>(data.items.size());

            for (const auto &item : std::as_const(data.items)) {
                Q_ASSERT(!item.isMixChannel());

                QXmppRosterIq iq;
                iq.addItem(item);
                iq.setType(QXmppIq::Set);

                client->sendGenericIq(std::move(iq)).then(this, [promise, counter](auto &&result) mutable {
                    if (promise.task().isFinished()) {
                        return;
                    }

                    if (auto error = std::get_if<QXmppError>(&result); error) {
                        return promise.finish(std::move(*error));
                    }

                    if ((--(*counter)) == 0) {
                        return promise.finish(Success());
                    }
                });
            }

            return promise.task();
        };
        auto exportData = [this]() {
            return chainMapSuccess(requestRoster(), this, [](QXmppRosterIq &&iq) -> RosterData {
                const auto items = transformFilter<RosterData::Items>(iq.items(), [](const auto &item) -> std::optional<QXmppRosterIq::Item> {
                    if (item.isMixChannel()) {
                        return {};
                    }

                    auto fixed = item;

                    // We don't want this to be sent while importing.
                    // See https://datatracker.ietf.org/doc/html/rfc6121#section-2.1.2.2
                    fixed.setSubscriptionStatus({});

                    return fixed;
                });

                return { items };
            });
        };

        manager->registerExportData<RosterData>(importData, exportData);
    }
}

void QXmppRosterManager::onUnregistered(QXmppClient *client)
{
    if (auto manager = client->findExtension<QXmppAccountMigrationManager>()) {
        manager->unregisterExportData<RosterData>();
    }
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
