// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubSubAuthorization.h"

#include "StringLiterals.h"

const auto FORM_TYPE_SUBSCRIBE_AUTHORIZATION = u"http://jabber.org/protocol/pubsub#subscribe_authorization"_s;
const auto ALLOW_SUBSCRIPTION = u"pubsub#allow"_s;
const auto NODE = u"pubsub#node"_s;
const auto SUBSCRIBER_JID = u"pubsub#subscriber_jid"_s;
const auto SUBID = u"pubsub#subid"_s;

///
/// \class QXmppPubSubSubAuthorization
///
/// This class represents a PubSub subscribe authorization data form.
///
/// \since QXmpp 1.5
///

class QXmppPubSubSubAuthorizationPrivate : public QSharedData
{
public:
    std::optional<bool> allowSubscription;
    QString node;
    QString subscriberJid;
    QString subid;
};

///
/// Tries to parse a PubSub subscribe authorization form from a plain data form.
///
/// \returns The parsed data form on success.
///
std::optional<QXmppPubSubSubAuthorization> QXmppPubSubSubAuthorization::fromDataForm(const QXmppDataForm &form)
{
    if (auto parsed = QXmppPubSubSubAuthorization();
        QXmppDataFormBase::fromDataForm(form, parsed)) {
        return parsed;
    }
    return std::nullopt;
}

QXmppPubSubSubAuthorization::QXmppPubSubSubAuthorization()
    : d(new QXmppPubSubSubAuthorizationPrivate)
{
}

/// Copy-constructor.
QXmppPubSubSubAuthorization::QXmppPubSubSubAuthorization(const QXmppPubSubSubAuthorization &) = default;
/// Move-constructor.
QXmppPubSubSubAuthorization::QXmppPubSubSubAuthorization(QXmppPubSubSubAuthorization &&) = default;
QXmppPubSubSubAuthorization::~QXmppPubSubSubAuthorization() = default;
/// Assignment operator.
QXmppPubSubSubAuthorization &QXmppPubSubSubAuthorization::operator=(const QXmppPubSubSubAuthorization &) = default;
/// Move-assignment operator.
QXmppPubSubSubAuthorization &QXmppPubSubSubAuthorization::operator=(QXmppPubSubSubAuthorization &&) = default;

///
/// Returns whether the subscription is allowed.
///
std::optional<bool> QXmppPubSubSubAuthorization::allowSubscription() const
{
    return d->allowSubscription;
}

///
/// Sets whether the subscription is allowed.
///
void QXmppPubSubSubAuthorization::setAllowSubscription(std::optional<bool> allowSubscription)
{
    d->allowSubscription = allowSubscription;
}

///
/// Returns the node name of the relevant node.
///
QString QXmppPubSubSubAuthorization::node() const
{
    return d->node;
}

///
/// Sets the node name of the relevant node.
///
void QXmppPubSubSubAuthorization::setNode(const QString &node)
{
    d->node = node;
}

///
/// Returns the JID of the user requesting to subscribe.
///
QString QXmppPubSubSubAuthorization::subscriberJid() const
{
    return d->subscriberJid;
}

///
/// Sets the JID of the user requesting to subscribe.
///
void QXmppPubSubSubAuthorization::setSubscriberJid(const QString &subscriberJid)
{
    d->subscriberJid = subscriberJid;
}

///
/// Returns the subscription ID associated with the subscription request.
///
QString QXmppPubSubSubAuthorization::subid() const
{
    return d->subid;
}

///
/// Sets the subscription ID associated with the subscription request.
///
void QXmppPubSubSubAuthorization::setSubid(const QString &subid)
{
    d->subid = subid;
}

QString QXmppPubSubSubAuthorization::formType() const
{
    return FORM_TYPE_SUBSCRIBE_AUTHORIZATION;
}

bool QXmppPubSubSubAuthorization::parseField(const QXmppDataForm::Field &field)
{
    // ignore hidden fields
    using Type = QXmppDataForm::Field::Type;
    if (field.type() == Type::HiddenField) {
        return false;
    }

    const auto key = field.key();
    const auto value = field.value();

    if (key == ALLOW_SUBSCRIPTION) {
        d->allowSubscription = parseBool(value);
    } else if (key == NODE) {
        d->node = value.toString();
    } else if (key == SUBID) {
        d->subid = value.toString();
    } else if (key == SUBSCRIBER_JID) {
        d->subscriberJid = value.toString();
    } else {
        return false;
    }
    return true;
}

void QXmppPubSubSubAuthorization::serializeForm(QXmppDataForm &form) const
{
    using Type = QXmppDataForm::Field::Type;

    serializeOptional(form, Type::BooleanField, ALLOW_SUBSCRIPTION, d->allowSubscription);
    serializeNullable(form, Type::TextSingleField, NODE, d->node);
    serializeNullable(form, Type::TextSingleField, SUBID, d->subid);
    serializeNullable(form, Type::JidSingleField, SUBSCRIBER_JID, d->subscriberJid);
}
