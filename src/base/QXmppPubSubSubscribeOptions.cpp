// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubSubscribeOptions.h"

#include <QDateTime>

const auto SUBSCRIBE_OPTIONS_FORM_TYPE = QStringLiteral("http://jabber.org/protocol/pubsub#subscribe_options");

const auto NOTIFICATIONS_ENABLED = QStringLiteral("pubsub#deliver");
const auto DIGESTS_ENABLED = QStringLiteral("pubsub#digest");
const auto DIGEST_FREQUENCY_MS = QStringLiteral("pubsub#digest_frequency");
const auto BODY_INCLUDED = QStringLiteral("pubsub#include_body");
const auto EXPIRE = QStringLiteral("pubsub#expire");
const auto NOTIFICATION_RULES = QStringLiteral("pubsub#show-values");
const auto SUBSCRIPTION_TYPE = QStringLiteral("pubsub#subscription_type");
const auto SUBSCRIPTION_DEPTH = QStringLiteral("pubsub#subscription_depth");

class QXmppPubSubSubscribeOptionsPrivate : public QSharedData
{
public:
    std::optional<bool> notificationsEnabled;
    std::optional<bool> digestsEnabled;
    std::optional<quint32> digestFrequencyMs;
    std::optional<bool> bodyIncluded;
    QDateTime expire;
    QXmppPubSubSubscribeOptions::PresenceStates notificationRules;
    std::optional<QXmppPubSubSubscribeOptions::SubscriptionType> subscriptionType;
    std::optional<QXmppPubSubSubscribeOptions::SubscriptionDepth> subscriptionDepth;
};

QXmppPubSubSubscribeOptions::PresenceStates QXmppPubSubSubscribeOptions::presenceStatesFromStringList(const QStringList &values)
{
    PresenceStates states;
    if (values.contains(QStringLiteral("away")))
        states |= Away;
    if (values.contains(QStringLiteral("chat")))
        states |= Chat;
    if (values.contains(QStringLiteral("dnd")))
        states |= DoNotDisturb;
    if (values.contains(QStringLiteral("online")))
        states |= Online;
    if (values.contains(QStringLiteral("xa")))
        states |= ExtendedAway;
    return states;
}

QStringList QXmppPubSubSubscribeOptions::presenceStatesToStringList(PresenceStates states)
{
    QStringList output;
    if (states & Away)
        output << QStringLiteral("away");
    if (states & Chat)
        output << QStringLiteral("chat");
    if (states & DoNotDisturb)
        output << QStringLiteral("dnd");
    if (states & Online)
        output << QStringLiteral("online");
    if (states & ExtendedAway)
        output << QStringLiteral("xa");
    return output;
}

std::optional<QXmppPubSubSubscribeOptions> QXmppPubSubSubscribeOptions::fromDataForm(const QXmppDataForm &form)
{
    if (form.formType() == SUBSCRIBE_OPTIONS_FORM_TYPE) {
        QXmppPubSubSubscribeOptions options;
        options.parseForm(form);
        return options;
    }
    return std::nullopt;
}

QXmppPubSubSubscribeOptions::QXmppPubSubSubscribeOptions()
    : d(new QXmppPubSubSubscribeOptionsPrivate())
{
}

QXmppPubSubSubscribeOptions::QXmppPubSubSubscribeOptions(const QXmppPubSubSubscribeOptions &) = default;
QXmppPubSubSubscribeOptions::QXmppPubSubSubscribeOptions(QXmppPubSubSubscribeOptions &&) = default;
QXmppPubSubSubscribeOptions::~QXmppPubSubSubscribeOptions() = default;
QXmppPubSubSubscribeOptions &QXmppPubSubSubscribeOptions::operator=(const QXmppPubSubSubscribeOptions &) = default;
QXmppPubSubSubscribeOptions &QXmppPubSubSubscribeOptions::operator=(QXmppPubSubSubscribeOptions &&) = default;

std::optional<bool> QXmppPubSubSubscribeOptions::notificationsEnabled() const
{
    return d->notificationsEnabled;
}

void QXmppPubSubSubscribeOptions::setNotificationsEnabled(std::optional<bool> enabled)
{
    d->notificationsEnabled = enabled;
}

std::optional<bool> QXmppPubSubSubscribeOptions::digestsEnabled() const
{
    return d->digestsEnabled;
}

void QXmppPubSubSubscribeOptions::setDigestsEnabled(std::optional<bool> digestsEnabled)
{
    d->digestsEnabled = digestsEnabled;
}

std::optional<quint32> QXmppPubSubSubscribeOptions::digestFrequencyMs() const
{
    return d->digestFrequencyMs;
}

void QXmppPubSubSubscribeOptions::setDigestFrequencyMs(std::optional<quint32> digestFrequencyMs)
{
    d->digestFrequencyMs = digestFrequencyMs;
}

QDateTime QXmppPubSubSubscribeOptions::expire() const
{
    return d->expire;
}

void QXmppPubSubSubscribeOptions::setExpire(const QDateTime &expire)
{
    d->expire = expire;
}

std::optional<bool> QXmppPubSubSubscribeOptions::bodyIncluded() const
{
    return d->bodyIncluded;
}

void QXmppPubSubSubscribeOptions::setBodyIncluded(std::optional<bool> bodyIncluded)
{
    d->bodyIncluded = bodyIncluded;
}

QXmppPubSubSubscribeOptions::PresenceStates QXmppPubSubSubscribeOptions::notificationRules() const
{
    return d->notificationRules;
}

void QXmppPubSubSubscribeOptions::setNotificationRules(PresenceStates notificationRules)
{
    d->notificationRules = notificationRules;
}

std::optional<QXmppPubSubSubscribeOptions::SubscriptionType> QXmppPubSubSubscribeOptions::subscriptionType() const
{
    return d->subscriptionType;
}

void QXmppPubSubSubscribeOptions::setSubscriptionType(std::optional<SubscriptionType> subscriptionType)
{
    d->subscriptionType = subscriptionType;
}

std::optional<QXmppPubSubSubscribeOptions::SubscriptionDepth> QXmppPubSubSubscribeOptions::subscriptionDepth() const
{
    return d->subscriptionDepth;
}

void QXmppPubSubSubscribeOptions::setSubscriptionDepth(std::optional<SubscriptionDepth> subscriptionDepth)
{
    d->subscriptionDepth = subscriptionDepth;
}

QString QXmppPubSubSubscribeOptions::formType() const
{
    return SUBSCRIBE_OPTIONS_FORM_TYPE;
}

bool QXmppPubSubSubscribeOptions::parseField(const QXmppDataForm::Field &field)
{
    const auto subscriptionTypeFromString = [](const QString &value) -> std::optional<SubscriptionType> {
        if (value == QStringLiteral("items")) {
            return Items;
        }
        if (value == QStringLiteral("nodes")) {
            return Nodes;
        }
        return {};
    };

    const auto subscriptionDepthFromString = [](const QString &value) -> std::optional<SubscriptionDepth> {
        if (value == QChar(u'1')) {
            return TopLevelOnly;
        }
        if (value == QStringLiteral("all")) {
            return Recursive;
        }
        return {};
    };

    // ignore hidden fields
    if (field.type() == QXmppDataForm::Field::Type::HiddenField) {
        return false;
    }

    const auto key = field.key();
    const auto value = field.value();

    if (key == NOTIFICATIONS_ENABLED) {
        d->notificationsEnabled = parseBool(value);
    } else if (key == DIGESTS_ENABLED) {
        d->digestsEnabled = parseBool(value);
    } else if (key == DIGEST_FREQUENCY_MS) {
        d->digestFrequencyMs = parseUInt(value);
    } else if (key == BODY_INCLUDED) {
        d->bodyIncluded = parseBool(value);
    } else if (key == EXPIRE) {
        d->expire = QDateTime::fromString(value.toString(), Qt::ISODate);
    } else if (key == NOTIFICATION_RULES) {
        d->notificationRules = presenceStatesFromStringList(value.toStringList());
    } else if (key == SUBSCRIPTION_TYPE) {
        d->subscriptionType = subscriptionTypeFromString(value.toString());
    } else if (key == SUBSCRIPTION_DEPTH) {
        d->subscriptionDepth = subscriptionDepthFromString(value.toString());
    } else {
        return false;
    }
    return true;
}

void QXmppPubSubSubscribeOptions::serializeForm(QXmppDataForm &form) const
{
    using Type = QXmppDataForm::Field::Type;

    const auto numberToString = [](quint32 value) {
        return QString::number(value);
    };

    const auto subscriptionTypeToString = [](SubscriptionType type) -> QString {
        switch (type) {
        case Items:
            return QStringLiteral("items");
        case Nodes:
            return QStringLiteral("nodes");
        }
        return {};
    };

    const auto subscriptionDepthToString = [](SubscriptionDepth depth) -> QString {
        switch (depth) {
        case TopLevelOnly:
            return QStringLiteral("1");
        case Recursive:
            return QStringLiteral("all");
        };
        return {};
    };

    serializeOptional(form, Type::BooleanField, NOTIFICATIONS_ENABLED, d->notificationsEnabled);
    serializeOptional(form, Type::BooleanField, DIGESTS_ENABLED, d->digestsEnabled);
    serializeOptional(form, Type::TextSingleField, DIGEST_FREQUENCY_MS, d->digestFrequencyMs, numberToString);
    serializeDatetime(form, EXPIRE, d->expire);
    serializeOptional(form, Type::BooleanField, BODY_INCLUDED, d->bodyIncluded);
    serializeEmptyable(form, Type::ListMultiField, NOTIFICATION_RULES, presenceStatesToStringList(d->notificationRules));
    serializeOptional(form, Type::ListSingleField, SUBSCRIPTION_TYPE, d->subscriptionType, subscriptionTypeToString);
    serializeOptional(form, Type::ListSingleField, SUBSCRIPTION_DEPTH, d->subscriptionDepth, subscriptionDepthToString);
}
