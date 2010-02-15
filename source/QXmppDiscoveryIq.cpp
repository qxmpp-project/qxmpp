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

enum QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::getQueryType() const
{
    if (getQueryNamespace() == ns_disco_items)
        return ItemsQuery;
    else
        return InfoQuery;
}

void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    if (type == ItemsQuery)
        setQueryNamespace(ns_disco_items);
    else
        setQueryNamespace(ns_disco_info);
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
    setQueryNamespace(queryElement.namespaceURI());

    QList<QXmppElement> items;
    QDomElement itemElement = queryElement.firstChildElement();
    while (!itemElement.isNull())
    {
        items.append(QXmppElement(itemElement));
        itemElement = itemElement.nextSiblingElement();
    }
    setQueryItems(items);
}

