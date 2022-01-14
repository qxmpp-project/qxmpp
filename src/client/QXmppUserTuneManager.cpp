// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUserTuneManager.h"

#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppPubSubEvent.h"
#include "QXmppPubSubManager.h"
#include "QXmppTuneItem.h"

using namespace QXmpp::Private;

///
/// \class QXmppUserTuneManager
///
/// The QXmppUserTuneManager implements \xep{0118, User Tune}. You'll receive
/// tune updates from all presence subscriptions. You can publish tune
/// information on the user's account (publish()) and request tune information
/// from specific accounts (request()).
///
/// The manager needs to be added to the client first and also requires the
/// QXmppPubSubManager.
/// \code
/// QXmppClient client;
/// auto *pubSubManager = client.addNewExtension<QXmppPubSubManager>();
/// auto *tuneManager = client.addNewExtension<QXmppUserTuneManager>();
/// \endcode
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppUserTuneManager::TuneResult
///
/// Contains the User Tune information or an error.
///

///
/// \typedef QXmppUserTuneManager::PublishResult
///
/// Contains the ID of the published item on success or a stanza error.
///

///
/// \fn QXmppUserTuneManager::userTuneChanged()
///
/// Emitted whenever an \xep{0118, User Tune} items event arrives.
///

QXmppUserTuneManager::QXmppUserTuneManager()
{
}

QStringList QXmppUserTuneManager::discoveryFeatures() const
{
    return {
        ns_tune,
        ns_tune_notify,
    };
}

///
/// Request User Tune information from an account.
///
/// \param jid The account JID to request.
///
QFuture<QXmppUserTuneManager::TuneResult> QXmppUserTuneManager::request(const QString &jid)
{
    using PubSub = QXmppPubSubManager;
    using Error = QXmppStanza::Error;

    return chain<TuneResult>(pubSub()->requestItems<QXmppTuneItem>(jid, ns_tune), this,
                             [](PubSub::ItemsResult<QXmppTuneItem> &&result) -> TuneResult {
                                 if (const auto items = std::get_if<PubSub::Items<QXmppTuneItem>>(&result)) {
                                     if (!items->items.isEmpty()) {
                                         return items->items.constFirst();
                                     }
                                     return Error(Error::Cancel, Error::ItemNotFound, QStringLiteral("No tune available."));
                                 } else {
                                     return std::get<QXmppStanza::Error>(result);
                                 }
                             });
}

///
/// Publishes User Tune information on the user's account.
///
/// \param item The User Tune item to be published.
///
QFuture<QXmppUserTuneManager::PublishResult> QXmppUserTuneManager::publish(const QXmppTuneItem &item)
{
    return pubSub()->publishPepItem(ns_tune, item);
}

/// \cond
bool QXmppUserTuneManager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    if (nodeName == ns_tune && QXmppPubSubEvent<QXmppTuneItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppTuneItem> event;
        event.parse(element);

        if (event.eventType() == QXmppPubSubEventBase::Items) {
            if (!event.items().isEmpty()) {
                emit userTuneChanged(pubSubService, event.items().constFirst());
            } else {
                emit userTuneChanged(pubSubService, {});
            }
            return true;
        }
    }
    return false;
}
/// \endcond
