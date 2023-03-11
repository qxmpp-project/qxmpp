// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppPubSubAffiliation.h"
#include "QXmppPubSubIq_p.h"
#include "QXmppPubSubSubscription.h"
#include "QXmppResultSet.h"
#include "QXmppUtils.h"

#include <QSharedData>

using namespace QXmpp::Private;

/// \cond

///
/// \class QXmppPubSubIqBase
///
/// \brief The QXmppPubSubIqBase class is an abstract class used for parsing of
/// generic PubSub IQs as defined by \xep{0060, Publish-Subscribe}.
///
/// This class does not handle queries working with items. For a full-featured
/// PubSub-IQ, please use QXmppPubSubIq<T> with your needed item class.
///
/// \since QXmpp 1.5
///

///
/// \class QXmppPubSubIq
///
/// The QXmppPubSubIq class represents an IQ used for the publish-subscribe
/// mechanisms defined by \xep{0060, Publish-Subscribe}.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppPubSubIq<T>::items()
///
/// Returns the IQ's items.
///

///
/// \fn QXmppPubSubIq<T>::setItems()
///
/// Sets the IQ's items.
///
/// \param items
///

///
/// \fn QXmppPubSubIq<T>::isPubSubIq()
///
/// Returns true, if the element is a valid PubSub IQ stanza. The payload of the
/// &lt;item/&gt; is also checked.
///

static const QStringList PUBSUB_QUERIES = {
    QStringLiteral("affiliations"),
    QStringLiteral("affiliations"),
    QStringLiteral("configure"),
    QStringLiteral("create"),
    QStringLiteral("default"),
    QStringLiteral("default"),
    QStringLiteral("delete"),
    QStringLiteral("items"),
    QStringLiteral("options"),
    QStringLiteral("publish"),
    QStringLiteral("purge"),
    QStringLiteral("retract"),
    QStringLiteral("subscribe"),
    QStringLiteral("subscription"),
    QStringLiteral("subscriptions"),
    QStringLiteral("subscriptions"),
    QStringLiteral("unsubscribe"),
};

namespace QXmpp::Private {

class PubSubIqPrivate : public QSharedData
{
public:
    PubSubIqBase::QueryType queryType = PubSubIqBase::Items;
    QString queryJid;
    QString queryNode;
    QString subscriptionId;
    QVector<QXmppPubSubSubscription> subscriptions;
    QVector<QXmppPubSubAffiliation> affiliations;
    uint32_t maxItems = 0;
    std::optional<QXmppDataForm> dataForm;
    std::optional<QXmppResultSetReply> itemsContinuation;
};

}  // namespace QXmpp::Private

///
/// Constructs a PubSub IQ.
///
PubSubIqBase::PubSubIqBase()
    : d(new PubSubIqPrivate)
{
}

/// Default copy-constructor
PubSubIqBase::PubSubIqBase(const PubSubIqBase &iq) = default;

PubSubIqBase::~PubSubIqBase() = default;

/// Default assignment operator
PubSubIqBase &PubSubIqBase::operator=(const PubSubIqBase &iq) = default;

///
/// Returns the PubSub query type for this IQ.
///
PubSubIqBase::QueryType PubSubIqBase::queryType() const
{
    return d->queryType;
}

///
/// Sets the PubSub query type for this IQ.
///
/// \param queryType
///
void PubSubIqBase::setQueryType(PubSubIqBase::QueryType queryType)
{
    d->queryType = queryType;
}

///
/// Returns the JID being queried.
///
QString PubSubIqBase::queryJid() const
{
    return d->queryJid;
}

///
/// Sets the JID being queried.
///
/// \param queryJid
///
void PubSubIqBase::setQueryJid(const QString &queryJid)
{
    d->queryJid = queryJid;
}

///
/// Returns the name of the node being queried.
///
QString PubSubIqBase::queryNode() const
{
    return d->queryNode;
}

///
/// Sets the name of the node being queried.
///
/// \param queryNodeName
///
void PubSubIqBase::setQueryNode(const QString &queryNodeName)
{
    d->queryNode = queryNodeName;
}

///
/// Returns the subscription ID for the request.
///
/// This does not work for SubscriptionQuery IQs, use subscription() instead.
///
QString PubSubIqBase::subscriptionId() const
{
    return d->subscriptionId;
}

///
/// Sets the subscription ID for the request.
///
/// This does not work for SubscriptionQuery IQs, use setSubscription() instead.
///
void PubSubIqBase::setSubscriptionId(const QString &subscriptionId)
{
    d->subscriptionId = subscriptionId;
}

///
/// Returns the included subscriptions.
///
QVector<QXmppPubSubSubscription> PubSubIqBase::subscriptions() const
{
    return d->subscriptions;
}

///
/// Sets the included subscriptions.
///
void PubSubIqBase::setSubscriptions(const QVector<QXmppPubSubSubscription> &subscriptions)
{
    d->subscriptions = subscriptions;
}

///
/// Returns the subscription.
///
/// This is a utility function for subscriptions(). It returns the first
/// subscription if existant. This can be used for both query types,
/// Subscription and Subscriptions.
///
std::optional<QXmppPubSubSubscription> PubSubIqBase::subscription() const
{
    if (d->subscriptions.isEmpty()) {
        return std::nullopt;
    }
    return d->subscriptions.first();
}

///
/// Sets the subscription.
///
/// This is a utility function for setSubscriptions(). It can be used for both
/// query types, Subscription and Subscriptions.
///
void PubSubIqBase::setSubscription(const std::optional<QXmppPubSubSubscription> &subscription)
{
    if (subscription) {
        d->subscriptions = { *subscription };
    } else {
        d->subscriptions.clear();
    }
}

///
/// Returns the included affiliations.
///
QVector<QXmppPubSubAffiliation> PubSubIqBase::affiliations() const
{
    return d->affiliations;
}

///
/// Sets the included affiliations.
///
void PubSubIqBase::setAffiliations(const QVector<QXmppPubSubAffiliation> &affiliations)
{
    d->affiliations = affiliations;
}

///
/// Returns the maximum of items that are requested.
///
/// This is only used for queries with type ItemsQuery.
///
std::optional<uint32_t> PubSubIqBase::maxItems() const
{
    if (d->maxItems) {
        return d->maxItems;
    }
    return std::nullopt;
}

///
/// Sets the maximum of items that are requested.
///
/// This is only used for queries with type ItemsQuery.
///
void PubSubIqBase::setMaxItems(std::optional<uint32_t> maxItems)
{
    d->maxItems = maxItems.value_or(0);
}

///
/// Returns a data form if the IQ contains one.
///
std::optional<QXmppDataForm> PubSubIqBase::dataForm() const
{
    return d->dataForm;
}

///
/// Sets a data form (or clears it by setting std::nullopt).
///
void PubSubIqBase::setDataForm(const std::optional<QXmppDataForm> &dataForm)
{
    d->dataForm = dataForm;
}

///
/// Returns a description of which items have been returned.
///
/// If this value is set the results are incomplete.
///
std::optional<QXmppResultSetReply> PubSubIqBase::itemsContinuation() const
{
    return d->itemsContinuation;
}

///
/// Returns a description of which items have been returned.
///
/// If this value is set the results are incomplete.
///
void PubSubIqBase::setItemsContinuation(const std::optional<QXmppResultSetReply> &itemsContinuation)
{
    d->itemsContinuation = itemsContinuation;
}

bool PubSubIqBase::isPubSubIq(const QDomElement &element)
{
    // no special requirements for the item / it's payload
    return PubSubIqBase::isPubSubIq(element, [](const QDomElement &) {
        return true;
    });
}

bool PubSubIqBase::isPubSubIq(const QDomElement &element, bool (*isItemValid)(const QDomElement &))
{
    // IQs must have only one direct child element.
    const auto pubSubElement = element.firstChildElement();
    if (pubSubElement.tagName() != QStringLiteral("pubsub")) {
        return false;
    }

    // check for correct namespace
    const bool isOwner = pubSubElement.namespaceURI() == ns_pubsub_owner;
    if (!isOwner && pubSubElement.namespaceURI() != ns_pubsub) {
        return false;
    }

    // check that the query type is valid
    auto queryElement = pubSubElement.firstChildElement();
    auto optionalType = queryTypeFromDomElement(queryElement);
    if (!optionalType) {
        return false;
    }
    auto queryType = *optionalType;

    // check for the "node" attribute
    switch (queryType) {
    case OwnerAffiliations:
    case Items:
    case Publish:
    case Retract:
    case Delete:
    case Purge:
        if (!queryElement.hasAttribute(QStringLiteral("node"))) {
            return false;
        }
    default:
        break;
    }

    // check for the "jid" attribute
    switch (queryType) {
    case Options:
    case OwnerSubscriptions:
    case Subscribe:
    case Unsubscribe:
        if (!queryElement.hasAttribute(QStringLiteral("jid"))) {
            return false;
        }
    default:
        break;
    }

    // check the individual content
    switch (queryType) {
    case Items:
    case Publish:
    case Retract:
        // check the items using isItemValid()
        for (auto itemElement = queryElement.firstChildElement(QStringLiteral("item"));
             !itemElement.isNull();
             itemElement = itemElement.nextSiblingElement(QStringLiteral("item"))) {
            if (!isItemValid(itemElement)) {
                return false;
            }
        }
        break;
    case Subscription:
        if (!QXmppPubSubSubscription::isSubscription(queryElement)) {
            return false;
        }
    case Delete:
    case Purge:
    case Configure:
        if (!isOwner) {
            return false;
        }
        break;
    case Affiliations:
    case OwnerAffiliations:
    case Create:
    case Default:
    case OwnerDefault:
    case Options:
    case Subscribe:
    case Subscriptions:
    case OwnerSubscriptions:
    case Unsubscribe:
        break;
    }
    return true;
}

void PubSubIqBase::parseElementFromChild(const QDomElement &element)
{
    const auto findChildElement = [](const QDomElement &element, const QString &tag, const QString &namespaceUri) {
        for (auto subElement = element.firstChildElement(tag);
             !subElement.isNull();
             subElement = subElement.nextSiblingElement(tag)) {
            if (subElement.namespaceURI() == namespaceUri) {
                return subElement;
            }
        }
        return QDomElement();
    };

    const auto parseDataFormFromChild = [=](const QDomElement &element) -> std::optional<QXmppDataForm> {
        if (const auto subElement = findChildElement(element, "x", ns_data);
            !subElement.isNull()) {
            QXmppDataForm form;
            form.parse(subElement);
            return form;
        }
        return {};
    };

    const auto pubSubElement = element.firstChildElement(QStringLiteral("pubsub"));
    const auto queryElement = pubSubElement.firstChildElement();

    // parse query type
    if (auto type = queryTypeFromDomElement(queryElement)) {
        d->queryType = *type;
    } else {
        return;
    }

    // Subscription is special: The query element is directly handled by
    // QXmppPubSubSubscription.
    if (d->queryType == Subscription) {
        QXmppPubSubSubscription subscription;
        subscription.parse(queryElement);
        setSubscription(subscription);

        // form inside following <options/>
        d->dataForm = parseDataFormFromChild(pubSubElement.firstChildElement("options"));
        return;
    }

    d->queryJid = queryElement.attribute(QStringLiteral("jid"));
    d->queryNode = queryElement.attribute(QStringLiteral("node"));

    // parse subid
    switch (d->queryType) {
    case Items:
    case Unsubscribe:
    case Options:
        d->subscriptionId = queryElement.attribute(QStringLiteral("subid"));
    default:
        break;
    }

    // parse contents
    switch (d->queryType) {
    case Affiliations:
    case OwnerAffiliations:
        for (auto subElement = queryElement.firstChildElement();
             !subElement.isNull();
             subElement = subElement.nextSiblingElement()) {
            if (QXmppPubSubAffiliation::isAffiliation(subElement)) {
                QXmppPubSubAffiliation affiliation;
                affiliation.parse(subElement);

                d->affiliations << std::move(affiliation);
            }
        }
        break;
    case Items:
        // Result Set Management (incomplete items result received)
        for (auto rsmEl = pubSubElement.firstChildElement(QStringLiteral("set"));
             !rsmEl.isNull();
             rsmEl = rsmEl.nextSiblingElement(QStringLiteral("set"))) {
            if (rsmEl.namespaceURI() == ns_rsm) {
                QXmppResultSetReply reply;
                reply.parse(rsmEl);
                d->itemsContinuation = reply;
            }
        }
        [[fallthrough]];
    case Publish:
    case Retract:
        parseItems(queryElement);

        if (d->queryType == Items) {
            d->maxItems = queryElement.attribute(QStringLiteral("max_items")).toUInt();
        } else if (d->queryType == Publish) {
            // form inside following <publish-options/>
            d->dataForm = parseDataFormFromChild(pubSubElement.firstChildElement("publish-options"));
        }

        break;
    case Subscriptions:
    case OwnerSubscriptions:
        for (auto subElement = queryElement.firstChildElement();
             !subElement.isNull();
             subElement = subElement.nextSiblingElement()) {
            if (QXmppPubSubSubscription::isSubscription(subElement)) {
                QXmppPubSubSubscription subscription;
                subscription.parse(subElement);

                d->subscriptions << std::move(subscription);
            }
        }
        break;
    case Configure:
    case Default:
    case OwnerDefault:
    case Options:
        // form in direct child <x/>
        d->dataForm = parseDataFormFromChild(queryElement);
        break;
    case Create:
        // form inside following <configure/>
        d->dataForm = parseDataFormFromChild(pubSubElement.firstChildElement("configure"));
        break;
    case Subscribe:
    case Subscription:
        // form inside following <options/>
        d->dataForm = parseDataFormFromChild(pubSubElement.firstChildElement("options"));
        break;
    case Delete:
    case Purge:
    case Unsubscribe:
        break;
    }
}

void PubSubIqBase::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("pubsub"));
    writer->writeDefaultNamespace(queryTypeIsOwnerIq(d->queryType) ? ns_pubsub_owner : ns_pubsub);

    // The SubscriptionQuery is special here: The query element is directly
    // handled by a QXmppPubSubSubscription.
    if (d->queryType == Subscription) {
        subscription().value_or(QXmppPubSubSubscription()).toXml(writer);
    } else {
        // write query type
        writer->writeStartElement(PUBSUB_QUERIES.at(d->queryType));
        helperToXmlAddAttribute(writer, QStringLiteral("jid"), d->queryJid);
        helperToXmlAddAttribute(writer, QStringLiteral("node"), d->queryNode);

        // write subid
        switch (d->queryType) {
        case Items:
        case Unsubscribe:
        case Options:
            helperToXmlAddAttribute(writer, QStringLiteral("subid"), d->subscriptionId);
        default:
            break;
        }

        // write contents
        switch (d->queryType) {
        case Affiliations:
        case OwnerAffiliations:
            for (const auto &affiliation : std::as_const(d->affiliations)) {
                affiliation.toXml(writer);
            }
            break;
        case Items:
            if (d->maxItems > 0) {
                writer->writeAttribute(QStringLiteral("max_items"), QString::number(d->maxItems));
            }
            [[fallthrough]];
        case Publish:
        case Retract:
            serializeItems(writer);
            break;
        case Subscriptions:
        case OwnerSubscriptions:
            for (const auto &sub : std::as_const(d->subscriptions)) {
                sub.toXml(writer);
            }
            break;
        case Configure:
        case Default:
        case OwnerDefault:
        case Options:
            if (auto form = d->dataForm) {
                // make sure data form type is correct
                switch (type()) {
                case QXmppIq::Result:
                    form->setType(QXmppDataForm::Result);
                    break;
                default:
                    if (form->type() != QXmppDataForm::Cancel) {
                        form->setType(QXmppDataForm::Submit);
                    }
                    break;
                }
                form->toXml(writer);
            }
            break;
        case Create:
        case Delete:
        case Purge:
        case Subscribe:
        case Subscription:
        case Unsubscribe:
            break;
        }

        writer->writeEndElement();  // query type

        // add extra element with data form
        if (auto form = d->dataForm) {
            const auto writeForm = [](QXmlStreamWriter *writer, const QXmppDataForm &form, const QString &subElementName) {
                writer->writeStartElement(subElementName);
                form.toXml(writer);
                writer->writeEndElement();
            };

            // make sure form type is 'submit'
            form->setType(type() == QXmppIq::Result ? QXmppDataForm::Result : QXmppDataForm::Submit);

            switch (d->queryType) {
            case Create:
                writeForm(writer, *form, QStringLiteral("configure"));
                break;
            case Publish:
                writeForm(writer, *form, QStringLiteral("publish-options"));
                break;
            case Subscribe:
            case Subscription:
                writeForm(writer, *form, QStringLiteral("options"));
                break;
            default:
                break;
            }
        }

        // Result Set Management
        if (d->queryType == Items && d->itemsContinuation.has_value()) {
            d->itemsContinuation->toXml(writer);
        }
    }
    writer->writeEndElement();  // pubsub
}

std::optional<PubSubIqBase::QueryType> PubSubIqBase::queryTypeFromDomElement(const QDomElement &element)
{
    QueryType type;
    if (auto index = PUBSUB_QUERIES.indexOf(element.tagName()); index != -1) {
        type = QueryType(index);
    } else {
        return std::nullopt;
    }

    // Some queries can have ns_pubsub_owner and normal ns_pubsub. To
    // distinguish those after parsing those with ns_pubsub_owner are replaced
    // by another query type.

    if (element.namespaceURI() != ns_pubsub_owner) {
        return type;
    }

    switch (type) {
    case Affiliations:
        return OwnerAffiliations;
    case Default:
        return OwnerDefault;
    case Subscriptions:
        return OwnerSubscriptions;
    default:
        return type;
    }
}

bool PubSubIqBase::queryTypeIsOwnerIq(QueryType type)
{
    switch (type) {
    case OwnerAffiliations:
    case OwnerSubscriptions:
    case OwnerDefault:
    case Configure:
    case Delete:
    case Purge:
        return true;
    case Affiliations:
    case Create:
    case Default:
    case Items:
    case Options:
    case Publish:
    case Retract:
    case Subscribe:
    case Subscription:
    case Subscriptions:
    case Unsubscribe:
        return false;
    }
    return false;
}

/// \endcond
