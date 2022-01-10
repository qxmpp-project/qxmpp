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

#ifndef QXMPPUSERLOCATIONMANAGER_H
#define QXMPPUSERLOCATIONMANAGER_H

#include "QXmppPubSubEventManager.h"

#include <variant>

class QXmppGeolocItem;

class QXMPP_EXPORT QXmppUserLocationManager : public QXmppPubSubEventManager
{
    Q_OBJECT

public:
    using LocationResult = std::variant<QXmppGeolocItem, QXmppStanza::Error>;
    using PublishResult = std::variant<QString, QXmppStanza::Error>;

    QXmppUserLocationManager();

    QStringList discoveryFeatures() const override;

    QFuture<LocationResult> request(const QString &jid);
    QFuture<PublishResult> publish(const QXmppGeolocItem &);

    Q_SIGNAL void userLocationChanged(const QString &jid, const QXmppGeolocItem &);

protected:
    /// \cond
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond
};

#endif  // QXMPPUSERLOCATIONMANAGER_H
