// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppElement.h"
#include "QXmppPubSubIq.h"
#include "QXmppPubSubItem.h"
#include "QXmppSessionIq.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QSharedData>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

// SessionIq

bool QXmppSessionIq::isSessionIq(const QDomElement &element)
{
    return isIqType(element, u"session", ns_session);
}

void QXmppSessionIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("session"));
    writer->writeDefaultNamespace(toString65(ns_session));
    writer->writeEndElement();
}

// PubSubIq

/// \cond
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED

static const QStringList PUBSUB_QUERIES = {
    QStringLiteral("affiliations"),
    QStringLiteral("default"),
    QStringLiteral("items"),
    QStringLiteral("publish"),
    QStringLiteral("retract"),
    QStringLiteral("subscribe"),
    QStringLiteral("subscription"),
    QStringLiteral("subscriptions"),
    QStringLiteral("unsubscribe"),
};

class QXmppPubSubIqPrivate : public QSharedData
{
public:
    QXmppPubSubIqPrivate();

    QXmppPubSubIq::QueryType queryType;
    QString queryJid;
    QString queryNode;
    QList<QXmppPubSubItem> items;
    QString subscriptionId;
    QString subscriptionType;
};

QXmppPubSubIqPrivate::QXmppPubSubIqPrivate()
    : queryType(QXmppPubSubIq::ItemsQuery)
{
}

QXmppPubSubIq::QXmppPubSubIq()
    : d(new QXmppPubSubIqPrivate)
{
}

QXmppPubSubIq::QXmppPubSubIq(const QXmppPubSubIq &iq) = default;

QXmppPubSubIq::~QXmppPubSubIq() = default;

QXmppPubSubIq &QXmppPubSubIq::operator=(const QXmppPubSubIq &iq) = default;

/// Returns the PubSub queryType for this IQ.

QXmppPubSubIq::QueryType QXmppPubSubIq::queryType() const
{
    return d->queryType;
}

/// Sets the PubSub queryType for this IQ.
///
/// \param queryType

void QXmppPubSubIq::setQueryType(QXmppPubSubIq::QueryType queryType)
{
    d->queryType = queryType;
}

/// Returns the JID being queried.

QString QXmppPubSubIq::queryJid() const
{
    return d->queryJid;
}

/// Sets the JID being queried.
///
/// \param queryJid

void QXmppPubSubIq::setQueryJid(const QString &queryJid)
{
    d->queryJid = queryJid;
}

/// Returns the node being queried.

QString QXmppPubSubIq::queryNode() const
{
    return d->queryNode;
}

/// Sets the node being queried.
///
/// \param queryNode

void QXmppPubSubIq::setQueryNode(const QString &queryNode)
{
    d->queryNode = queryNode;
}

/// Returns the subscription ID.

QString QXmppPubSubIq::subscriptionId() const
{
    return d->subscriptionId;
}

/// Sets the subscription ID.
///
/// \param subscriptionId

void QXmppPubSubIq::setSubscriptionId(const QString &subscriptionId)
{
    d->subscriptionId = subscriptionId;
}

/// Returns the IQ's items.

QList<QXmppPubSubItem> QXmppPubSubIq::items() const
{
    return d->items;
}

/// Sets the IQ's items.
///
/// \param items

void QXmppPubSubIq::setItems(const QList<QXmppPubSubItem> &items)
{
    d->items = items;
}

bool QXmppPubSubIq::isPubSubIq(const QDomElement &element)
{
    return element.firstChildElement(QStringLiteral("pubsub")).namespaceURI() == ns_pubsub;
}

void QXmppPubSubIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement pubSubElement = element.firstChildElement(QStringLiteral("pubsub"));

    const QDomElement queryElement = pubSubElement.firstChildElement();

    // determine query type
    const QString tagName = queryElement.tagName();
    int queryType = PUBSUB_QUERIES.indexOf(queryElement.tagName());
    if (queryType > -1) {
        d->queryType = QueryType(queryType);
    }

    d->queryJid = queryElement.attribute(QStringLiteral("jid"));
    d->queryNode = queryElement.attribute(QStringLiteral("node"));

    // parse contents
    QDomElement childElement;
    switch (d->queryType) {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        for (const auto &childElement : iterChildElements(queryElement, u"item")) {
            QXmppPubSubItem item;
            item.parse(childElement);
            d->items << item;
        }
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        d->subscriptionId = queryElement.attribute(QStringLiteral("subid"));
        d->subscriptionType = queryElement.attribute(QStringLiteral("subscription"));
        break;
    default:
        break;
    }
}

void QXmppPubSubIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("pubsub"));
    writer->writeDefaultNamespace(toString65(ns_pubsub));

    // write query type
    writer->writeStartElement(PUBSUB_QUERIES.at(d->queryType));
    writeOptionalXmlAttribute(writer, u"jid", d->queryJid);
    writeOptionalXmlAttribute(writer, u"node", d->queryNode);

    // write contents
    switch (d->queryType) {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        for (const auto &item : d->items) {
            item.toXml(writer);
        }
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        writeOptionalXmlAttribute(writer, u"subid", d->subscriptionId);
        writeOptionalXmlAttribute(writer, u"subscription", d->subscriptionType);
        break;
    default:
        break;
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
QT_WARNING_POP
/// \endcond

// PubSubItem

/// \cond
class QXmppPubSubItemPrivate : public QSharedData
{
public:
    QString id;
    QXmppElement contents;
};

QXmppPubSubItem::QXmppPubSubItem()
    : d(new QXmppPubSubItemPrivate)
{
}

QXmppPubSubItem::QXmppPubSubItem(const QXmppPubSubItem &iq) = default;

QXmppPubSubItem::~QXmppPubSubItem() = default;

QXmppPubSubItem &QXmppPubSubItem::operator=(const QXmppPubSubItem &iq) = default;

/// Returns the ID of the PubSub item.

QString QXmppPubSubItem::id() const
{
    return d->id;
}

/// Sets the ID of the PubSub item.
///
/// \param id

void QXmppPubSubItem::setId(const QString &id)
{
    d->id = id;
}

/// Returns the contents of the PubSub item.

QXmppElement QXmppPubSubItem::contents() const
{
    return d->contents;
}

/// Sets the contents of the PubSub item.
///
/// \param contents

void QXmppPubSubItem::setContents(const QXmppElement &contents)
{
    d->contents = contents;
}

void QXmppPubSubItem::parse(const QDomElement &element)
{
    d->id = element.attribute(QStringLiteral("id"));
    d->contents = QXmppElement(element.firstChildElement());
}

void QXmppPubSubItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("item"));
    writeOptionalXmlAttribute(writer, u"id", d->id);
    d->contents.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
