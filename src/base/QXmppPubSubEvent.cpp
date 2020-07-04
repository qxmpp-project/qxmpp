/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Germán Márquez Mejía
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

#include "QXmppConstants_p.h"
#include "QXmppPubSubEvent.h"

class QXmppPubSubEventPrivate : public QSharedData
{
public:
    QString nodeName;
    QList<QXmppPubSubItem> items;
};

///
/// Constructs a PubSub event.
///
QXmppPubSubEvent::QXmppPubSubEvent() : d(new QXmppPubSubEventPrivate) {}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppPubSubEvent::QXmppPubSubEvent(const QXmppPubSubEvent &other) : d(other.d) {}

QXmppPubSubEvent::~QXmppPubSubEvent() {}

///
/// Assigns \a other to this event.
///
/// \param other
///
QXmppPubSubEvent &QXmppPubSubEvent::operator=(const QXmppPubSubEvent &other)
{
    d = other.d;
    return *this;
}

///
/// Returns the name of the event's node.
///
/// \return the node name
///
QString QXmppPubSubEvent::nodeName() const
{
    return d->nodeName;
}

///
/// Sets the name of the event's node.
///
/// \param nodeName
///
void QXmppPubSubEvent::setNodeName(const QString &nodeName)
{
    d->nodeName = nodeName;
}

///
/// Returns the PubSub items of the event.
///
/// \return a list of the PubSub items
///
QList<QXmppPubSubItem> QXmppPubSubEvent::items() const
{
    return d->items;
}

///
/// Sets the PubSub items of the event.
///
/// \param items
///
void QXmppPubSubEvent::setItems(const QList<QXmppPubSubItem> &items)
{
    d->items = items;
}

///
/// Determines whether the event object is initialized.
///
/// \return true if the node name is empty and the event contains no items
///
bool QXmppPubSubEvent::isNull() const
{
    return d->nodeName.isEmpty() && d->items.isEmpty();
}

/// \cond
void QXmppPubSubEvent::parse(const QDomElement &element)
{
    // parse node name
    QDomElement itemsElement = element.firstChildElement("items");
    d->nodeName = itemsElement.attribute("node");

    // parse items
    QDomElement childElement = itemsElement.firstChildElement("item");
    while (!childElement.isNull())
    {
        QXmppPubSubItem item;
        item.parse(childElement);
        d->items << item;
        childElement = childElement.nextSiblingElement("item");
    }
}

void QXmppPubSubEvent::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("event");
    writer->writeAttribute("xmlns", ns_pubsub_event);

    // write node name
    writer->writeStartElement("items");
    writer->writeAttribute("node", d->nodeName);

    // write items
    for (const QXmppPubSubItem &item : d->items)
        item.toXml(writer);

    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
