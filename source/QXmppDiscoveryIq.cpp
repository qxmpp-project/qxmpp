/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppUtils.h"

#include <QDomElement>

QString QXmppDiscoveryIq::Identity::category() const
{
    return m_category;
}

void QXmppDiscoveryIq::Identity::setCategory(const QString &category)
{
    m_category = category;
}

QString QXmppDiscoveryIq::Identity::name() const
{
    return m_name;
}

void QXmppDiscoveryIq::Identity::setName(const QString &name)
{
    m_name = name;
}

QString QXmppDiscoveryIq::Identity::type() const
{
    return m_type;
}

void QXmppDiscoveryIq::Identity::setType(const QString &type)
{
    m_type = type;
}

QString QXmppDiscoveryIq::Item::jid() const
{
    return m_jid;
}

void QXmppDiscoveryIq::Item::setJid(const QString &jid)
{
    m_jid = jid;
}

QString QXmppDiscoveryIq::Item::name() const
{
    return m_name;
}

void QXmppDiscoveryIq::Item::setName(const QString &name)
{
    m_name = name;
}

QString QXmppDiscoveryIq::Item::node() const
{
    return m_node;
}

void QXmppDiscoveryIq::Item::setNode(const QString &node)
{
    m_node = node;
}

QStringList QXmppDiscoveryIq::features() const
{
    return m_features;
}

void QXmppDiscoveryIq::setFeatures(const QStringList &features)
{
    m_features = features;
}

QList<QXmppDiscoveryIq::Identity> QXmppDiscoveryIq::identities() const
{
    return m_identities;
}

void QXmppDiscoveryIq::setIdentities(const QList<QXmppDiscoveryIq::Identity> &identities)
{
    m_identities = identities;
}

QList<QXmppDiscoveryIq::Item> QXmppDiscoveryIq::items() const
{
    return m_items;
}

void QXmppDiscoveryIq::setItems(const QList<QXmppDiscoveryIq::Item> &items)
{
    m_items = items;
}

QString QXmppDiscoveryIq::queryNode() const
{
    return m_queryNode;
}

void QXmppDiscoveryIq::setQueryNode(const QString &node)
{
    m_queryNode = node;
}

enum QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::queryType() const
{
    return m_queryType;
}

void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    m_queryType = type;
}

bool QXmppDiscoveryIq::isDiscoveryIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_disco_info ||
            queryElement.namespaceURI() == ns_disco_items); 
}

void QXmppDiscoveryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    m_queryNode = queryElement.attribute("node");
    if (queryElement.namespaceURI() == ns_disco_items)
        m_queryType = ItemsQuery;
    else
        m_queryType = InfoQuery;

    QDomElement itemElement = queryElement.firstChildElement();
    while (!itemElement.isNull())
    {
        if (itemElement.tagName() == "feature")
        {
            m_features.append(itemElement.attribute("var"));
        } else if (itemElement.tagName() == "identity") {
            QXmppDiscoveryIq::Identity identity;
            identity.setCategory(itemElement.attribute("category"));
            identity.setName(itemElement.attribute("name"));
            identity.setType(itemElement.attribute("type"));
            m_identities.append(identity);
        } else if (itemElement.tagName() == "item") {
            QXmppDiscoveryIq::Item item;
            item.setJid(itemElement.attribute("jid"));
            item.setName(itemElement.attribute("name"));
            item.setNode(itemElement.attribute("node"));
            m_items.append(item);
        }
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppDiscoveryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns",
        m_queryType == InfoQuery ? ns_disco_info : ns_disco_items);
    helperToXmlAddAttribute(writer, "node", m_queryNode);

    foreach (const QString &feature, m_features)
    {
        writer->writeStartElement("feature");
        helperToXmlAddAttribute(writer, "var", feature);
        writer->writeEndElement();
    }

    foreach (const QXmppDiscoveryIq::Identity& identity, m_identities)
    {
        writer->writeStartElement("identity");
        helperToXmlAddAttribute(writer, "category", identity.category());
        helperToXmlAddAttribute(writer, "name", identity.name());
        helperToXmlAddAttribute(writer, "type", identity.type());
        writer->writeEndElement();
    }

    foreach (const QXmppDiscoveryIq::Item& item, m_items)
    {
        writer->writeStartElement("item");
        helperToXmlAddAttribute(writer, "jid", item.jid());
        helperToXmlAddAttribute(writer, "name", item.name());
        helperToXmlAddAttribute(writer, "node", item.node());
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

