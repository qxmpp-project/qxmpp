/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include "QXmppPubSubItem.h"

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

class QXmppPubSubItemPrivate : public QSharedData
{
public:
    QString id;
    QXmppElement payload;
};

///
/// Constructs a PubSub item.
///
QXmppPubSubItem::QXmppPubSubItem()
    : d(new QXmppPubSubItemPrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppPubSubItem::QXmppPubSubItem(const QXmppPubSubItem &other) = default;

QXmppPubSubItem::~QXmppPubSubItem() = default;

///
/// Assigns \a other to this item.
///
/// \param other
///
QXmppPubSubItem &QXmppPubSubItem::operator=(const QXmppPubSubItem &other) = default;

///
/// Constructs an item with \a id but no payload.
///
/// \param id
///
QXmppPubSubItem::QXmppPubSubItem(const QString &id) : QXmppPubSubItem()
{
    d->id = id;
}

///
/// Constructs an item with \a payload but no ID.
///
/// \param payload
///
QXmppPubSubItem::QXmppPubSubItem(const QXmppElement &payload) : QXmppPubSubItem()
{
    d->payload = payload;
}

///
/// Constructs an item with the given \a id and \a payload.
///
/// \param id
/// \param payload
///
QXmppPubSubItem::QXmppPubSubItem(const QString &id, const QXmppElement &payload) : QXmppPubSubItem()
{
    d->id = id;
    d->payload = payload;
}

///
/// Returns the ID of the PubSub item.
///
QString QXmppPubSubItem::id() const
{
    return d->id;
}

///
/// Sets the ID of the PubSub item.
///
/// \param id
///
void QXmppPubSubItem::setId(const QString &id)
{
    d->id = id;
}

///
/// Returns the contents of the PubSub item.
///
QXmppElement QXmppPubSubItem::payload() const
{
    return d->payload;
}

///
/// Sets the contents of the PubSub item.
///
/// \param contents
///
void QXmppPubSubItem::setPayload(const QXmppElement &payload)
{
    d->payload = payload;
}

/// \cond
void QXmppPubSubItem::parse(const QDomElement &element)
{
    d->id = element.attribute(QStringLiteral("id"));
    d->payload = QXmppElement(element.firstChildElement());
}

void QXmppPubSubItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("item"));

    if (!d->id.isEmpty())
        helperToXmlAddAttribute(writer, QStringLiteral("id"), d->id);

    d->payload.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
