/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#ifndef QXMPPPUBSUBSUBSCRIPTION_H
#define QXMPPPUBSUBSUBSCRIPTION_H

#include "QXmppGlobal.h"

#include <QDateTime>
#include <QMetaType>
#include <QSharedDataPointer>

class QXmppPubSubSubscriptionPrivate;
class QXmlStreamWriter;
class QDomElement;

class QXMPP_EXPORT QXmppPubSubSubscription
{
public:
    ///
    /// The State enum describes the state of a subscription.
    ///
    enum State : uint8_t {
        /// No state information is included.
        Invalid,
        /// There is no subscription with the node.
        None,
        /// A subscription is pending.
        Pending,
        /// The user is subscribed to the node.
        Subscribed,
        /// The subscription requires configuration before it becomes active.
        Unconfigured,
    };
    static QString stateToString(State);
    static State stateFromString(const QString &);

    ///
    /// The SubscribeOptionsSupport enum describes whether the availability of a
    /// subscription configuration. This is also known as
    /// &lt;subscribe-options/&gt;.
    ///
    enum ConfigurationSupport : uint8_t {
        /// A subscription configuration is not advertised.
        Unavailable,
        /// Configuration of the subscription is possible, but not required.
        Available,
        /// Configuration of the subscription is required. No event
        /// notifications are going to be sent until the subscription has been
        /// configured.
        Required,
    };

    QXmppPubSubSubscription(const QString &jid = {},
                            const QString &node = {},
                            const QString &subId = {},
                            State state = Invalid,
                            ConfigurationSupport configurationSupport = Unavailable,
                            const QDateTime &expiry = {});
    QXmppPubSubSubscription(const QXmppPubSubSubscription &);
    ~QXmppPubSubSubscription();

    QXmppPubSubSubscription &operator=(const QXmppPubSubSubscription &);

    QString jid() const;
    void setJid(const QString &jid);

    QString node() const;
    void setNode(const QString &node);

    QString subId() const;
    void setSubId(const QString &subId);

    QDateTime expiry() const;
    void setExpiry(const QDateTime &expiry);

    State state() const;
    void setState(State state);

    ConfigurationSupport configurationSupport() const;
    void setConfigurationSupport(ConfigurationSupport support);
    bool isConfigurationSupported() const;
    bool isConfigurationRequired() const;

    static bool isSubscription(const QDomElement &);

    /// \cond
    void parse(const QDomElement &);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubSubscriptionPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppPubSubSubscription, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(QXmppPubSubSubscription)
Q_DECLARE_METATYPE(QXmppPubSubSubscription::State)
Q_DECLARE_METATYPE(QXmppPubSubSubscription::ConfigurationSupport)

#endif  // QXMPPPUBSUBSUBSCRIPTION_H
