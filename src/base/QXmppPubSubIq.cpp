/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include "QXmppPubSubIq.h"

#include <QDomElement>
#include <QSharedData>

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

static const char *ns_pubsub = "http://jabber.org/protocol/pubsub";

static const char *pubsub_queries[] = {
    "affiliations",
    "default",
    "items",
    "publish",
    "retract",
    "subscribe",
    "subscription",
    "subscriptions",
    "unsubscribe",
};

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

/// \cond
void QXmppPubSubItem::parse(const QDomElement &element)
{
    d->id = element.attribute("id");
    d->contents = QXmppElement(element.firstChildElement());
}

void QXmppPubSubItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "id", d->id);
    d->contents.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

class QXmppPubSubIqPrivate : public QSharedData
{
public:
    QXmppPubSubIq::QueryType queryType;
    QString queryJid;
    QString queryNode;
    QList<QXmppPubSubItem> items;
    QString subscriptionId;
    QString subscriptionType;
};

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

/// \cond
bool QXmppPubSubIq::isPubSubIq(const QDomElement &element)
{
    const QDomElement pubSubElement = element.firstChildElement("pubsub");
    return pubSubElement.namespaceURI() == ns_pubsub;
}

void QXmppPubSubIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement pubSubElement = element.firstChildElement("pubsub");

    const QDomElement queryElement = pubSubElement.firstChildElement();

    // determine query type
    const QString tagName = queryElement.tagName();
    for (int i = ItemsQuery; i <= SubscriptionsQuery; i++)
    {
        if (tagName == pubsub_queries[i])
        {
            d->queryType = static_cast<QueryType>(i);
            break;
        }
    }
    d->queryJid = queryElement.attribute("jid");
    d->queryNode = queryElement.attribute("node");

    // parse contents
    QDomElement childElement;
    switch (d->queryType)
    {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        childElement = queryElement.firstChildElement("item");
        while (!childElement.isNull())
        {
            QXmppPubSubItem item;
            item.parse(childElement);
            d->items << item;
            childElement = childElement.nextSiblingElement("item");
        }
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        d->subscriptionId = queryElement.attribute("subid");
        d->subscriptionType = queryElement.attribute("subscription");
        break;
    default:
        break;
    }
}

void QXmppPubSubIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("pubsub");
    writer->writeAttribute("xmlns", ns_pubsub);

    // write query type
    writer->writeStartElement(pubsub_queries[d->queryType]);
    helperToXmlAddAttribute(writer, "jid", d->queryJid);
    helperToXmlAddAttribute(writer, "node", d->queryNode);

    // write contents
    switch (d->queryType)
    {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        for (const auto &item : d->items)
            item.toXml(writer);
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        helperToXmlAddAttribute(writer, "subid", d->subscriptionId);
        helperToXmlAddAttribute(writer, "subscription", d->subscriptionType);
        break;
    default:
        break;
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
