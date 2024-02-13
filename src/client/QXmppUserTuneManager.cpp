// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUserTuneManager.h"

#include "QXmppConstants_p.h"
#include "QXmppPep_p.h"
#include "QXmppUserTuneItem.h"

using namespace QXmpp::Private;

static QXmppPubSubManager *pubSub(QXmppClient *client)
{
    return client->findExtension<QXmppPubSubManager>();
}

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
/// \ingroup Managers
///

///
/// \typedef QXmppUserTuneManager::Item
///
/// Used pubsub item type.
///

///
/// \typedef QXmppUserTuneManager::GetResult
///
/// Contains the User Tune information or an error.
///

///
/// \typedef QXmppUserTuneManager::PublishResult
///
/// Contains the ID of the published item on success or a stanza error.
///

///
/// \fn QXmppUserTuneManager::itemReceived()
///
/// Emitted whenever a \xep{0118, User Tune} items event arrives.
///

QXmppUserTuneManager::QXmppUserTuneManager()
{
}

QStringList QXmppUserTuneManager::discoveryFeatures() const
{
    return {
        ns_tune.toString(),
        ns_tune_notify.toString(),
    };
}

///
/// Request User Tune information from an account.
///
/// \param jid The account JID to request.
///
auto QXmppUserTuneManager::request(const QString &jid)
    -> QXmppTask<GetResult>
{
    return Pep::request<Item>(pubSub(client()), jid, ns_tune.toString(), this);
}

///
/// Publishes User Tune information on the user's account.
///
/// \param item The User Tune item to be published.
///
auto QXmppUserTuneManager::publish(const QXmppTuneItem &item)
    -> QXmppTask<PublishResult>
{
    return pubSub(client())->publishOwnPepItem(ns_tune.toString(), item);
}

/// \cond
bool QXmppUserTuneManager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    return Pep::handlePubSubEvent<Item>(element, pubSubService, nodeName, ns_tune.toString(), this, &QXmppUserTuneManager::itemReceived);
}
/// \endcond
