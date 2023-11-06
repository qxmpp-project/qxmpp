// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

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
    QXmppPubSubSubscription(QXmppPubSubSubscription &&);
    ~QXmppPubSubSubscription();

    QXmppPubSubSubscription &operator=(const QXmppPubSubSubscription &);
    QXmppPubSubSubscription &operator=(QXmppPubSubSubscription &&);

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
