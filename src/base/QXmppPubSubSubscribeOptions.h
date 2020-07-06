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

#ifndef QXMPPPUBSUBSUBSCRIBEOPTIONS_H
#define QXMPPPUBSUBSUBSCRIBEOPTIONS_H

#include "QXmppDataForm.h"
#include "QXmppDataFormBase.h"
#include "QXmppGlobal.h"

#include <QFlags>
#include <QSharedDataPointer>

class QDateTime;
class QDomElement;
class QXmlStreamWriter;
class QXmppDataForm;
class QXmppPubSubSubscribeOptionsPrivate;

class QXMPP_EXPORT QXmppPubSubSubscribeOptions : public QXmppExtensibleDataFormBase
{
public:
    enum PresenceState : uint8_t {
        Unset = 0x00,
        Online = 0x01,
        Away = 0x02,
        Chat = 0x04,
        DoNotDisturb = 0x08,
        ExtendedAway = 0x10
    };
    Q_DECLARE_FLAGS(PresenceStates, PresenceState)
    static PresenceStates presenceStatesFromStringList(const QStringList &);
    static QStringList presenceStatesToStringList(PresenceStates);

    enum SubscriptionType : uint8_t {
        Items,
        Nodes
    };

    enum SubscriptionDepth : uint8_t {
        TopLevelOnly,
        Recursive
    };

    static std::optional<QXmppPubSubSubscribeOptions> fromDataForm(const QXmppDataForm &form);

    QXmppPubSubSubscribeOptions();
    QXmppPubSubSubscribeOptions(const QXmppPubSubSubscribeOptions &);
    virtual ~QXmppPubSubSubscribeOptions();

    QXmppPubSubSubscribeOptions &operator=(const QXmppPubSubSubscribeOptions &);

    std::optional<bool> notificationsEnabled() const;
    void setNotificationsEnabled(std::optional<bool> notifying);

    std::optional<bool> digestsEnabled() const;
    void setDigestsEnabled(std::optional<bool> digestsEnabled);

    std::optional<quint32> digestFrequencyMs() const;
    void setDigestFrequencyMs(std::optional<quint32> digestFrequencyMs);

    QDateTime expire() const;
    void setExpire(const QDateTime &expire);

    std::optional<bool> bodyIncluded() const;
    void setBodyIncluded(std::optional<bool> bodyIncluded);

    PresenceStates notificationRules() const;
    void setNotificationRules(PresenceStates notificationRules);

    std::optional<SubscriptionType> subscriptionType() const;
    void setSubscriptionType(std::optional<SubscriptionType> subscriptionType);

    std::optional<SubscriptionDepth> subscriptionDepth() const;
    void setSubscriptionDepth(std::optional<SubscriptionDepth> subscriptionDepth);

protected:
    QString formType() const override;
    bool parseField(const QXmppDataForm::Field &) override;
    void serializeForm(QXmppDataForm &) const override;

private:
    QSharedDataPointer<QXmppPubSubSubscribeOptionsPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppPubSubSubscribeOptions::PresenceStates)
Q_DECLARE_METATYPE(QXmppPubSubSubscribeOptions)

#endif  // QXMPPPUBSUBSUBSCRIBEOPTIONS_H
