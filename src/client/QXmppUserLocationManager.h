// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Cochise César <cochisecesar@zoho.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUSERLOCATIONMANAGER_H
#define QXMPPUSERLOCATIONMANAGER_H

#include "QXmppPubSubEventManager.h"

#include <variant>

class QXmppGeolocItem;

class QXMPP_EXPORT QXmppUserLocationManager : public QXmppPubSubEventManager
{
    Q_OBJECT

public:
    using Item = QXmppGeolocItem;
    using GetResult = std::variant<Item, QXmppStanza::Error>;
    using PublishResult = std::variant<QString, QXmppStanza::Error>;

    QXmppUserLocationManager();

    QStringList discoveryFeatures() const override;

    QFuture<GetResult> request(const QString &jid);
    QFuture<PublishResult> publish(const Item &);

    Q_SIGNAL void itemReceived(const QString &jid, const QXmppGeolocItem &);

protected:
    /// \cond
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond
};

#endif  // QXMPPUSERLOCATIONMANAGER_H
