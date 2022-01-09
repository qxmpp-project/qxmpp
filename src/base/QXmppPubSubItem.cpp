/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Linus Jahn
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
    QXmppPubSubItemPrivate(const QString &id, const QString &publisher);

    QString id;
    QString publisher;
};

QXmppPubSubItemPrivate::QXmppPubSubItemPrivate(const QString &id, const QString &publisher)
    : id(id), publisher(publisher)
{
}

///
/// \class QXmppPubSubItem
///
/// The QXmppPubSubItem class represents a publish-subscribe item as defined by
/// \xep{0060, Publish-Subscribe}.
///
/// To access the payload of an item, you need to create a derived class of this
/// and override QXmppPubSubItem::parsePayload() and
/// QXmppPubSubItem::serializePayload().
///
/// It is also required that you override QXmppPubSubItem::isItem() and also
/// check for the correct payload of the PubSub item. This can be easily done by
/// using the protected overload of isItem() with an function that checks the
/// tag name and namespace of the payload. The function is only called if a
/// payload exists.
///
/// In short, you need to reimplement these methods:
///  * QXmppPubSubItem::parsePayload()
///  * QXmppPubSubItem::serializePayload()
///  * QXmppPubSubItem::isItem()
///
/// \since QXmpp 1.5
///

///
/// Constructs an item with \a id and \a publisher.
///
/// \param id
/// \param publisher
///
QXmppPubSubItem::QXmppPubSubItem(const QString &id, const QString &publisher)
    : d(new QXmppPubSubItemPrivate(id, publisher))
{
}

/// Default copy-constructor
QXmppPubSubItem::QXmppPubSubItem(const QXmppPubSubItem &iq) = default;

QXmppPubSubItem::~QXmppPubSubItem() = default;

/// Default assignment operator
QXmppPubSubItem &QXmppPubSubItem::operator=(const QXmppPubSubItem &iq) = default;

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
/// Returns the JID of the publisher of the item.
///
QString QXmppPubSubItem::publisher() const
{
    return d->publisher;
}

///
/// Sets the JID of the publisher of the item.
///
void QXmppPubSubItem::setPublisher(const QString &publisher)
{
    d->publisher = publisher;
}

/// \cond
void QXmppPubSubItem::parse(const QDomElement &element)
{
    d->id = element.attribute(QStringLiteral("id"));
    d->publisher = element.attribute(QStringLiteral("publisher"));

    parsePayload(element.firstChildElement());
}

void QXmppPubSubItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("item"));
    helperToXmlAddAttribute(writer, QStringLiteral("id"), d->id);
    helperToXmlAddAttribute(writer, QStringLiteral("publisher"), d->publisher);

    serializePayload(writer);

    writer->writeEndElement();
}
/// \endcond

///
/// Returns true, if the element is possibly a PubSub item.
///
bool QXmppPubSubItem::isItem(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("item");
}

///
/// Parses the payload of the item (the child element of the &lt;item/&gt;).
///
/// This method needs to be overriden to perform the payload-specific parsing.
///
void QXmppPubSubItem::parsePayload(const QDomElement &)
{
}

///
/// Serializes the payload of the item (the child element of the &lt;item/&gt;).
///
/// This method needs to be overriden to perform the payload-specific
/// serialization.
///
void QXmppPubSubItem::serializePayload(QXmlStreamWriter *) const
{
}
