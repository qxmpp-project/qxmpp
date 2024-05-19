// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubEvent.h"

#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// \class QXmppPubSubEventBase
///
/// The QXmppPubSubEventBase class is an abstract class used for parsing of
/// generic PubSub event notifications as defined by \xep{0060, Publish-
/// Subscribe}.
///
/// This class cannot be used directly. For a full-featured access to the event
/// notifications, please use the QXmppPubSubEvent class.
///
/// \since QXmpp 1.5
///

///
/// \class QXmppPubSubEvent
///
/// \brief The QXmppPubSubEvent class represents a PubSub event notification as
/// defined by \xep{0060, Publish-Subscribe}.
///
/// This class has a template parameter that can be used to define the type of
/// the contained items.
///
/// You can use QXmppPubSubEvent::isPubSubItem() to check whether an DOM element
/// is a &lt;message/&gt; with a PubSub event notification. If you set a special
/// type as a template parameter, validity of the items will also be checked. To
/// check for an event notification with items from \xep{0118, User Tune} for
/// example, you could use the following:
/// \code
/// QXmppPubSubEvent<QXmppTuneItem>::isPubSubEvent(element);
/// \endcode
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.5
///

constexpr auto PUBSUB_EVENTS = to_array<QStringView>({
    u"configuration",
    u"delete",
    u"items",
    u"items",  // virtual retract type
    u"purge",
    u"subscription",
});

class QXmppPubSubEventPrivate : public QSharedData
{
public:
    QXmppPubSubEventPrivate(QXmppPubSubEventBase::EventType type,
                            const QString &node);

    QXmppPubSubEventBase::EventType eventType;
    QString node;
    QStringList retractIds;
    QString redirectUri;
    std::optional<QXmppPubSubSubscription> subscription;
    std::optional<QXmppDataForm> configurationForm;
};

QXmppPubSubEventPrivate::QXmppPubSubEventPrivate(QXmppPubSubEventBase::EventType type,
                                                 const QString &node)
    : eventType(type), node(node)
{
}

///
/// Constructs a PubSub event.
///
QXmppPubSubEventBase::QXmppPubSubEventBase(EventType type, const QString &node)
    : d(new QXmppPubSubEventPrivate(type, node))
{
    setType(QXmppMessage::Normal);
}

/// Default copy-constructor.
QXmppPubSubEventBase::QXmppPubSubEventBase(const QXmppPubSubEventBase &other) = default;
/// Default move-constructor.
QXmppPubSubEventBase::QXmppPubSubEventBase(QXmppPubSubEventBase &&) = default;
QXmppPubSubEventBase::~QXmppPubSubEventBase() = default;
/// Default assignment operator.
QXmppPubSubEventBase &QXmppPubSubEventBase::operator=(const QXmppPubSubEventBase &other) = default;
/// Default move-assignment operator.
QXmppPubSubEventBase &QXmppPubSubEventBase::operator=(QXmppPubSubEventBase &&) = default;

///
/// Returns the event type of the PubSub event.
///
QXmppPubSubEventBase::EventType QXmppPubSubEventBase::eventType() const
{
    return d->eventType;
}

///
/// Sets the event type of the PubSub event.
///
void QXmppPubSubEventBase::setEventType(EventType type)
{
    d->eventType = type;
}

///
/// Returns the name of the event's node.
///
/// This does not work with Subscription events. In those cases you need to get
/// the node of the subscription.
///
/// \sa subscription()
/// \sa QXmppPubSubSubscription::node()
///
QString QXmppPubSubEventBase::node() const
{
    return d->node;
}

///
/// Sets the name of the event's node.
///
/// This does not work with Subscription events. In those cases you need to set
/// the node of the subscription.
///
/// \sa subscription()
/// \sa QXmppPubSubSubscription::setNode()
///
void QXmppPubSubEventBase::setNode(const QString &node)
{
    d->node = node;
}

///
/// Returns the item IDs that have been retracted.
///
/// This is only used for the Items event type.
///
QStringList QXmppPubSubEventBase::retractIds() const
{
    return d->retractIds;
}

///
/// Sets the item IDs that have been retracted.
///
/// This is only used for the Items event type.
///
void QXmppPubSubEventBase::setRetractIds(const QStringList &retractIds)
{
    d->retractIds = retractIds;
}

///
/// Returns the redirect URI to the new node.
///
/// This can be set for delete notifications to inform subscribers of the new
/// node. Inclusion of this is of course optional.
///
QString QXmppPubSubEventBase::redirectUri() const
{
    return d->redirectUri;
}

///
/// Sets the redirect URI to the new node.
///
/// This can be set for delete notifications to inform subscribers of the new
/// node. Inclusion of this is of course optional.
///
void QXmppPubSubEventBase::setRedirectUri(const QString &redirectUri)
{
    d->redirectUri = redirectUri;
}

///
/// Returns the subscription in case of a Subscription event.
///
std::optional<QXmppPubSubSubscription> QXmppPubSubEventBase::subscription() const
{
    return d->subscription;
}

///
/// Sets the subscription in case of a Subscription event.
///
void QXmppPubSubEventBase::setSubscription(const std::optional<QXmppPubSubSubscription> &subscription)
{
    d->subscription = subscription;
}

///
/// Returns a configuration data form if the event contains one.
///
std::optional<QXmppDataForm> QXmppPubSubEventBase::configurationForm() const
{
    return d->configurationForm;
}

///
/// Sets a configuration data form (or clears it with std::nullopt).
///
void QXmppPubSubEventBase::setConfigurationForm(const std::optional<QXmppDataForm> &configurationForm)
{
    d->configurationForm = configurationForm;
}

/// \cond
bool QXmppPubSubEventBase::isPubSubEvent(const QDomElement &stanza, std::function<bool(const QDomElement &)> isItemValid)
{
    if (stanza.tagName() != u"message") {
        return false;
    }

    // find correct "event" element
    auto event = firstChildElement(stanza, u"event"_s, ns_pubsub_event);
    auto eventTypeElement = event.firstChildElement();

    // check for validity of the event type
    auto eventType = enumFromString<EventType>(PUBSUB_EVENTS, eventTypeElement.tagName());
    if (!eventType) {
        return false;
    }

    // check for "node" attribute when required
    switch (*eventType) {
    case Delete:
    case Items:
    case Retract:
    case Purge:
        if (!eventTypeElement.hasAttribute(u"node"_s)) {
            return false;
        }
        break;
    case Configuration:
    case Subscription:
        break;
    }

    // check individual content
    switch (*eventType) {
    case Delete: {
        if (const auto redirect = eventTypeElement.firstChildElement(u"redirect"_s);
            !redirect.isNull() && !redirect.hasAttribute(u"uri"_s)) {
            return false;
        }
        break;
    }
    case Items:
    case Retract: {
        // check validity of the items using isItemValid()
        for (const auto &itemElement : iterChildElements(eventTypeElement, u"item")) {
            if (!isItemValid(itemElement)) {
                return false;
            }
        }
        break;
    }
    case Subscription: {
        if (!QXmppPubSubSubscription::isSubscription(eventTypeElement)) {
            return false;
        }
    }
    case Configuration:
    case Purge:
        break;
    }

    return true;
}

bool QXmppPubSubEventBase::parseExtension(const QDomElement &eventElement, QXmpp::SceMode sceMode)
{
    if (sceMode & QXmpp::SceSensitive &&
        eventElement.tagName() == u"event" &&
        eventElement.namespaceURI() == ns_pubsub_event) {
        // check that the query type is valid
        const auto eventTypeElement = eventElement.firstChildElement();
        if (auto eventType = enumFromString<EventType>(PUBSUB_EVENTS, eventTypeElement.tagName())) {
            d->eventType = *eventType;
        } else {
            return false;
        }

        // Detect our virtual retract event type
        if (d->eventType == Items) {
            auto child = eventTypeElement.firstChildElement();
            if (!child.isNull()) {
                if (child.tagName() == u"retract") {
                    d->eventType = Retract;
                }
            }
            // Don't support mixed retract/item events.
        }

        // parse "node" attribute
        switch (d->eventType) {
        case Configuration:
        case Delete:
        case Items:
        case Retract:
        case Purge:
            d->node = eventTypeElement.attribute(u"node"_s);
            break;
        case Subscription:
            break;
        }

        // check the items using isItemValid()
        switch (d->eventType) {
        case Delete:
            if (auto redirect = eventTypeElement.firstChildElement(u"redirect"_s);
                !redirect.isNull()) {
                d->redirectUri = redirect.attribute(u"uri"_s);
            }
            break;
        case Items:
            // parse items
            parseItems(eventTypeElement);
            break;
        case Retract:
            // parse retract ids
            for (const auto &retract : iterChildElements(eventTypeElement, u"retract")) {
                d->retractIds << retract.attribute(u"id"_s);
            }
            break;
        case Subscription: {
            QXmppPubSubSubscription subscription;
            subscription.parse(eventTypeElement);
            d->subscription = subscription;
            break;
        }
        case Configuration:
            if (auto formElement = firstChildElement(eventTypeElement, u"x"_s, ns_data);
                !formElement.isNull()) {
                QXmppDataForm form;
                form.parse(formElement);
                d->configurationForm = form;
            }
            break;
        case Purge:
            break;
        }
    } else {
        // handles QXmppMessage default extensions
        return QXmppMessage::parseExtension(eventElement, sceMode);
    }

    return true;
}

void QXmppPubSubEventBase::serializeExtensions(QXmlStreamWriter *writer, QXmpp::SceMode sceMode, const QString &baseNamespace) const
{
    QXmppMessage::serializeExtensions(writer, sceMode, baseNamespace);

    if (!(sceMode & QXmpp::SceSensitive)) {
        return;
    }

    writer->writeStartElement(QSL65("event"));
    writer->writeDefaultNamespace(toString65(ns_pubsub_event));

    if (d->eventType == Subscription && d->subscription) {
        d->subscription->toXml(writer);
    } else {
        writer->writeStartElement(toString65(PUBSUB_EVENTS.at(size_t(d->eventType))));

        // write node attribute
        switch (d->eventType) {
        case Delete:
        case Items:
        case Retract:
        case Purge:
            // node attribute is required
            writer->writeAttribute(QSL65("node"), d->node);
            break;
        case Configuration:
            // node attribute is optional
            writeOptionalXmlAttribute(writer, u"node", d->node);
            break;
        case Subscription:
            break;
        }

        switch (d->eventType) {
        case Configuration:
            if (d->configurationForm) {
                d->configurationForm->toXml(writer);
            }
            break;
        case Delete:
            if (!d->redirectUri.isEmpty()) {
                writer->writeStartElement(QSL65("redirect"));
                writer->writeAttribute(QSL65("uri"), d->redirectUri);
                writer->writeEndElement();
            }
        case Items:
            // serialize items
            serializeItems(writer);

            break;
        case Retract:
            // serialize retract ids
            for (const auto &id : std::as_const(d->retractIds)) {
                writer->writeStartElement(QSL65("retract"));
                writer->writeAttribute(QSL65("id"), id);
                writer->writeEndElement();
            }

            break;
        case Purge:
        case Subscription:
            break;
        }

        writer->writeEndElement();  // close event's type element
    }
    writer->writeEndElement();  // </event>
}
/// \endcond
