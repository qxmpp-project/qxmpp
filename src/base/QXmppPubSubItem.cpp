/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppPubSubItem.h"

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

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
