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

QStringList QXmppDiscoveryItem::attributes() const
{
    return m_attributes.keys();
}

QString QXmppDiscoveryItem::attribute(const QString &name) const
{
    return m_attributes.value(name);
}

void QXmppDiscoveryItem::setAttribute(const QString &name, const QString &value)
{
    m_attributes.insert(name, value);
}

QString QXmppDiscoveryItem::type() const
{
    return m_type;
}

void QXmppDiscoveryItem::setType(const QString &type)
{
    m_type = type;
}

QList<QXmppDiscoveryItem> QXmppDiscoveryIq::getItems() const
{
    return m_items;
}

void QXmppDiscoveryIq::setItems(const QList<QXmppDiscoveryItem> &items)
{
    m_items = items;
}

enum QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::getQueryType() const
{
    return m_queryType;
}

void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    m_queryType = type;
}

bool QXmppDiscoveryIq::isDiscoveryIq( QDomElement &element )
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_disco_info ||
            queryElement.namespaceURI() == ns_disco_items); 
}

void QXmppDiscoveryIq::parse( QDomElement &element )
{
    setFrom(element.attribute("from"));
    setTo(element.attribute("to"));
    setTypeFromStr(element.attribute("type"));
    QDomElement queryElement = element.firstChildElement("query");
    if (queryElement.namespaceURI() == ns_disco_items)
        m_queryType = ItemsQuery;
    else
        m_queryType = InfoQuery;

    QDomElement itemElement = queryElement.firstChildElement();
    while (!itemElement.isNull())
    {
        QXmppDiscoveryItem item;
        item.setType(itemElement.tagName());
        QDomNamedNodeMap attributes = itemElement.attributes();
        for (int i = 0; i < attributes.size(); i++)
        {
            QDomAttr attr = attributes.item(i).toAttr();
            item.setAttribute(attr.name(), attr.value());
        }
        m_items.append(item);
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppDiscoveryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns",
        m_queryType == InfoQuery ? ns_disco_info : ns_disco_items);
    foreach (const QXmppDiscoveryItem &item, m_items)
    {
        writer->writeStartElement(item.type());
        foreach (const QString &attr, item.attributes())
            helperToXmlAddAttribute(writer, attr, item.attribute(attr));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

