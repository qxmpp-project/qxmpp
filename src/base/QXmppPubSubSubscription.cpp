// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubSubscription.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDateTime>
#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

///
/// \class QXmppPubSubSubscription
///
/// The QXmppPubSubSubscription class represents a PubSub subscription contained
/// in event notifications and IQ requests, as defined in \xep{0060, Publish-
/// Subscribe}.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.5
///

constexpr auto SUBSCRIPTION_STATES = to_array<QStringView>({
    {},
    u"none",
    u"pending",
    u"subscribed",
    u"unconfigured",
});

class QXmppPubSubSubscriptionPrivate : public QSharedData
{
public:
    QXmppPubSubSubscriptionPrivate(const QString &jid,
                                   const QString &node,
                                   const QString &subId,
                                   QXmppPubSubSubscription::State state,
                                   QXmppPubSubSubscription::ConfigurationSupport configurationSupport,
                                   const QDateTime &expiry);

    QString jid;
    QString node;
    QString subId;
    QDateTime expiry;
    QXmppPubSubSubscription::State state;
    QXmppPubSubSubscription::ConfigurationSupport configurationSupport;
};

QXmppPubSubSubscriptionPrivate::QXmppPubSubSubscriptionPrivate(const QString &jid,
                                                               const QString &node,
                                                               const QString &subId,
                                                               QXmppPubSubSubscription::State state,
                                                               QXmppPubSubSubscription::ConfigurationSupport configurationSupport,
                                                               const QDateTime &expiry)
    : jid(jid),
      node(node),
      subId(subId),
      expiry(expiry),
      state(state),
      configurationSupport(configurationSupport)
{
}

///
/// Converts a subscription state to string.
///
QString QXmppPubSubSubscription::stateToString(State state)
{
    return SUBSCRIPTION_STATES.at(size_t(state)).toString();
}

///
/// Converts a string with a subscription state to the enum value.
///
QXmppPubSubSubscription::State QXmppPubSubSubscription::stateFromString(const QString &str)
{
    return enumFromString<State>(SUBSCRIPTION_STATES, str).value_or(Invalid);
}

///
/// Creates a new QXmppPubSubSubscription.
///
/// \param jid
/// \param node
/// \param subId
/// \param state
/// \param configurationSupport
/// \param expiry
///
QXmppPubSubSubscription::QXmppPubSubSubscription(const QString &jid,
                                                 const QString &node,
                                                 const QString &subId,
                                                 State state,
                                                 ConfigurationSupport configurationSupport,
                                                 const QDateTime &expiry)
    : d(new QXmppPubSubSubscriptionPrivate(jid, node, subId, state, configurationSupport, expiry))
{
}

/// Copy contructor.
QXmppPubSubSubscription::QXmppPubSubSubscription(const QXmppPubSubSubscription &) = default;
/// Move contructor.
QXmppPubSubSubscription::QXmppPubSubSubscription(QXmppPubSubSubscription &&) = default;
QXmppPubSubSubscription::~QXmppPubSubSubscription() = default;
/// Copy assignment operator.
QXmppPubSubSubscription &QXmppPubSubSubscription::operator=(const QXmppPubSubSubscription &) = default;
/// Move-assignment operator.
QXmppPubSubSubscription &QXmppPubSubSubscription::operator=(QXmppPubSubSubscription &&) = default;

///
/// Returns the JID of the user of this subscription.
///
QString QXmppPubSubSubscription::jid() const
{
    return d->jid;
}

///
/// Sets the JID of the user of this subscription.
///
void QXmppPubSubSubscription::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns the node name of this subscription.
///
QString QXmppPubSubSubscription::node() const
{
    return d->node;
}

///
/// Sets the node name of this subscription.
///
void QXmppPubSubSubscription::setNode(const QString &node)
{
    d->node = node;
}

///
/// Returns the subscription ID (may be empty).
///
QString QXmppPubSubSubscription::subId() const
{
    return d->subId;
}

///
/// Sets the subscription ID (may be empty).
///
void QXmppPubSubSubscription::setSubId(const QString &subId)
{
    d->subId = subId;
}

///
/// Returns the state of the subscription.
///
QXmppPubSubSubscription::State QXmppPubSubSubscription::state() const
{
    return d->state;
}

///
/// Sets the state of the subscription.
///
void QXmppPubSubSubscription::setState(State state)
{
    d->state = state;
}

///
/// Returns the expiry date of the subscription.
///
/// If this timestamp is valid, the subscription is going to be cancelled at
/// this date.
///
QDateTime QXmppPubSubSubscription::expiry() const
{
    return d->expiry;
}

///
/// Sets the expiry date of the subscription.
///
/// If this timestamp is valid, the subscription is going to be cancelled at
/// this date.
///
void QXmppPubSubSubscription::setExpiry(const QDateTime &expiry)
{
    d->expiry = expiry;
}

///
/// Returns the availability of a subscription configuration.
///
QXmppPubSubSubscription::ConfigurationSupport QXmppPubSubSubscription::configurationSupport() const
{
    return d->configurationSupport;
}

///
/// Sets the availability of a subscription configuration.
///
void QXmppPubSubSubscription::setConfigurationSupport(ConfigurationSupport support)
{
    d->configurationSupport = support;
}

///
/// Returns whether a configuration of the subscription is possible.
///
bool QXmppPubSubSubscription::isConfigurationSupported() const
{
    return d->configurationSupport > Unavailable;
}

///
/// Returns whether configuration of the subscription required before event
/// notifications are going to be sent to the user.
///
bool QXmppPubSubSubscription::isConfigurationRequired() const
{
    return d->configurationSupport == Required || d->state == Unconfigured;
}

///
/// Returns true, if the element is a PubSub subscription element.
///
bool QXmppPubSubSubscription::isSubscription(const QDomElement &element)
{
    if (element.tagName() != u"subscription") {
        return false;
    }

    if (element.hasAttribute(u"subscription"_s)) {
        const auto subStateStr = element.attribute(u"subscription"_s);
        if (!enumFromString<State>(SUBSCRIPTION_STATES, subStateStr)) {
            return false;
        }
    }

    if (element.namespaceURI() == ns_pubsub || element.namespaceURI() == ns_pubsub_event) {
        return element.hasAttribute(u"jid"_s);
    }
    if (element.namespaceURI() == ns_pubsub_owner) {
        return element.hasAttribute(u"jid"_s) &&
            element.hasAttribute(u"subscription"_s);
    }
    return false;
}

/// \cond
void QXmppPubSubSubscription::parse(const QDomElement &element)
{
    bool isPubSub = element.namespaceURI() == ns_pubsub;
    bool isPubSubEvent = !isPubSub && element.namespaceURI() == ns_pubsub_event;

    d->jid = element.attribute(u"jid"_s);
    d->state = stateFromString(element.attribute(u"subscription"_s));

    if (isPubSub || isPubSubEvent) {
        d->node = element.attribute(u"node"_s);
        d->subId = element.attribute(u"subid"_s);

        if (isPubSubEvent) {
            if (element.hasAttribute(u"expiry"_s)) {
                d->expiry = QXmppUtils::datetimeFromString(
                    element.attribute(u"expiry"_s));
            }
        } else if (isPubSub) {
            auto options = element.firstChildElement(u"subscribe-options"_s);
            if (options.isNull()) {
                d->configurationSupport = Unavailable;
            } else {
                if (!options.firstChildElement(u"required"_s).isNull()) {
                    d->configurationSupport = Required;
                } else {
                    d->configurationSupport = Available;
                }
            }
        }
    }
}

void QXmppPubSubSubscription::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("subscription"));

    // jid is required
    writer->writeAttribute(QSL65("jid"), d->jid);
    writeOptionalXmlAttribute(writer, u"node", d->node);
    writeOptionalXmlAttribute(writer, u"subscription", stateToString(d->state));
    writeOptionalXmlAttribute(writer, u"subid", d->subId);
    if (d->expiry.isValid()) {
        writer->writeAttribute(QSL65("expiry"),
                               QXmppUtils::datetimeToString(d->expiry));
    }

    if (d->configurationSupport > Unavailable) {
        writer->writeStartElement(QSL65("subscribe-options"));
        if (d->configurationSupport == Required) {
            writer->writeEmptyElement(u"required"_s);
        }
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
