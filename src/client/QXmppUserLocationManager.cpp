/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppUserLocationManager.h"

#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppGeolocItem.h"
#include "QXmppPubSubEvent.h"
#include "QXmppPubSubManager.h"

using namespace QXmpp::Private;

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

///
/// \typedef QXmppUserLocationManager::LocationResult
///
/// Contains the User Location information or an error.
///

///
/// \typedef QXmppUserLocationManager::PublishResult
///
/// Contains the ID of the published item on success or a stanza error.
///

///
/// \fn QXmppUserLocationManager::userLocationChanged()
///
/// Emitted whenever an \xep{0080, User Location} items event arrives.
///

QXmppUserLocationManager::QXmppUserLocationManager()
{
}

QStringList QXmppUserLocationManager::discoveryFeatures() const
{
    return {
        ns_geoloc,
        ns_geoloc_notify,
    };
}

///
/// Request User Location information from an account.
///
/// \param jid The account JID to request.
///
QFuture<QXmppUserLocationManager::LocationResult> QXmppUserLocationManager::request(const QString &jid)
{
    using PubSub = QXmppPubSubManager;
    using Error = QXmppStanza::Error;

    return chain<LocationResult>(pubSub()->requestItems<QXmppGeolocItem>(jid, ns_geoloc), this,
                                 [](PubSub::ItemsResult<QXmppGeolocItem> &&result) -> LocationResult {
                                     if (const auto items = std::get_if<PubSub::Items<QXmppGeolocItem>>(&result)) {
                                         if (!items->items.isEmpty()) {
                                             return items->items.constFirst();
                                         }
                                         return Error(Error::Cancel, Error::ItemNotFound, QStringLiteral("No location available."));
                                     } else {
                                         return std::get<QXmppStanza::Error>(result);
                                     }
                                 });
}

///
/// Publishes User Location information on the user's account.
///
/// \param item The User Location item to be published.
///
QFuture<QXmppUserLocationManager::PublishResult> QXmppUserLocationManager::publish(const QXmppGeolocItem &item)
{
    return pubSub()->publishPepItem(ns_geoloc, item);
}

/// \cond
bool QXmppUserLocationManager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    if (nodeName == ns_geoloc && QXmppPubSubEvent<QXmppGeolocItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppGeolocItem> event;
        event.parse(element);

        if (event.eventType() == QXmppPubSubEventBase::Items) {
            if (!event.items().isEmpty()) {
                emit userLocationChanged(pubSubService, event.items().constFirst());
            } else {
                emit userLocationChanged(pubSubService, {});
            }
            return true;
        }
    }
    return false;
}
/// \endcond
