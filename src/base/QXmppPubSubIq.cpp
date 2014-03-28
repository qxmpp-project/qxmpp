/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppPubSubIq.h"
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

/// Returns the ID of the PubSub item.
///

QString QXmppPubSubItem::id() const
{
    return m_id;
}

/// Sets the ID of the PubSub item.
///
/// \param id

void QXmppPubSubItem::setId(const QString &id)
{
    m_id = id;
}

/// Returns the contents of the PubSub item.
///

QXmppElement QXmppPubSubItem::contents() const
{
    return m_contents;
}

/// Sets the contents of the PubSub item.
///
/// \param contents

void QXmppPubSubItem::setContents(const QXmppElement &contents)
{
    m_contents = contents;
}

/// \cond
void QXmppPubSubItem::parse(const QDomElement &element)
{
    m_id = element.attribute("id");
    m_contents = QXmppElement(element.firstChildElement());
}

void QXmppPubSubItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "id", m_id);
    m_contents.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

/// Returns the PubSub queryType for this IQ.
///

QXmppPubSubIq::QueryType QXmppPubSubIq::queryType() const
{
    return m_queryType;
}

/// Sets the PubSub queryType for this IQ.
///
/// \param queryType

void QXmppPubSubIq::setQueryType(QXmppPubSubIq::QueryType queryType)
{
    m_queryType = queryType;
}

/// Returns the JID being queried.
///

QString QXmppPubSubIq::queryJid() const
{
    return m_queryJid;
}

/// Sets the JID being queried.
///
/// \param queryJid

void QXmppPubSubIq::setQueryJid(const QString &queryJid)
{
    m_queryJid = queryJid;
}

/// Returns the node being queried.
///

QString QXmppPubSubIq::queryNode() const
{
    return m_queryNode;
}

/// Sets the node being queried.
///
/// \param queryNode

void QXmppPubSubIq::setQueryNode(const QString &queryNode)
{
    m_queryNode = queryNode;
}

/// Returns the subscription ID.
///

QString QXmppPubSubIq::subscriptionId() const
{
    return m_subscriptionId;
}

/// Sets the subscription ID.
///
/// \param subscriptionId

void QXmppPubSubIq::setSubscriptionId(const QString &subscriptionId)
{
    m_subscriptionId = subscriptionId;
}

/// Returns the IQ's items.
///

QList<QXmppPubSubItem> QXmppPubSubIq::items() const
{
    return m_items;
}

/// Sets the IQ's items.
///
/// \param items

void QXmppPubSubIq::setItems(const QList<QXmppPubSubItem> &items)
{
    m_items = items;
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
            m_queryType = static_cast<QueryType>(i);
            break;
        }
    }
    m_queryJid = queryElement.attribute("jid");
    m_queryNode = queryElement.attribute("node");

    // parse contents
    QDomElement childElement;
    switch (m_queryType)
    {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
        childElement = queryElement.firstChildElement("item");
        while (!childElement.isNull())
        {
            QXmppPubSubItem item;
            item.parse(childElement);
            m_items << item;
            childElement = childElement.nextSiblingElement("item");
        }
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        m_subscriptionId = queryElement.attribute("subid");
        m_subscriptionType = queryElement.attribute("subscription");
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
    writer->writeStartElement(pubsub_queries[m_queryType]);
    helperToXmlAddAttribute(writer, "jid", m_queryJid);
    helperToXmlAddAttribute(writer, "node", m_queryNode);

    // write contents
    switch (m_queryType)
    {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
        foreach (const QXmppPubSubItem &item, m_items)
            item.toXml(writer);
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        helperToXmlAddAttribute(writer, "subid", m_subscriptionId);
        helperToXmlAddAttribute(writer, "subscription", m_subscriptionType);
        break;
    default:
        break;
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
