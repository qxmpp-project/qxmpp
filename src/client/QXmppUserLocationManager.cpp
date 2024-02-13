// SPDX-FileCopyrightText: 2022 Cochise César <cochisecesar@zoho.com>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUserLocationManager.h"

#include "QXmppConstants_p.h"
#include "QXmppGeolocItem.h"
#include "QXmppPep_p.h"

using namespace QXmpp::Private;

static QXmppPubSubManager *pubSub(QXmppClient *client)
{
    return client->findExtension<QXmppPubSubManager>();
}

///
/// \class QXmppUserLocationManager
///
/// The QXmppUserLocationManager implements \xep{0080, User Location}. You'll receive
/// location updates from all presence subscriptions. You can publish location
/// information on the user's account (publish()) and request location information
/// from specific accounts (request()).
///
/// The manager needs to be added to the client first and also requires the
/// QXmppPubSubManager.
/// \code
/// QXmppClient client;
/// auto *pubSubManager = client.addNewExtension<QXmppPubSubManager>();
/// auto *locationManager = client.addNewExtension<QXmppUserLocationManager>();
/// \endcode
///
/// \since QXmpp 1.5
///
/// \ingroup Managers
///

///
/// \typedef QXmppUserLocationManager::Item
///
/// Used pubsub item type.
///

///
/// \typedef QXmppUserLocationManager::GetResult
///
/// Contains the User Location information or an error.
///

///
/// \typedef QXmppUserLocationManager::PublishResult
///
/// Contains the ID of the published item on success or a stanza error.
///

///
/// \fn QXmppUserLocationManager::itemReceived()
///
/// Emitted whenever a \xep{0080, User Location} items event arrives.
///

QXmppUserLocationManager::QXmppUserLocationManager() = default;

QStringList QXmppUserLocationManager::discoveryFeatures() const
{
    return {
        ns_geoloc.toString(),
        ns_geoloc_notify.toString(),
    };
}

///
/// Request User Location information from an account.
///
/// \param jid The account JID to request.
///
auto QXmppUserLocationManager::request(const QString &jid)
    -> QXmppTask<GetResult>
{
    return Pep::request<Item>(pubSub(client()), jid, ns_geoloc.toString(), this);
}

///
/// Publishes User Location information on the user's account.
///
/// \param item The User Location item to be published.
///
auto QXmppUserLocationManager::publish(const QXmppGeolocItem &item)
    -> QXmppTask<PublishResult>
{
    return pubSub(client())->publishOwnPepItem(ns_geoloc.toString(), item);
}

/// \cond
bool QXmppUserLocationManager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    return Pep::handlePubSubEvent<Item>(element, pubSubService, nodeName, ns_geoloc.toString(), this, &QXmppUserLocationManager::itemReceived);
}
/// \endcond
