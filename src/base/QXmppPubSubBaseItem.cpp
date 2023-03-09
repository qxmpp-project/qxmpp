// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubBaseItem.h"

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

class QXmppPubSubBaseItemPrivate : public QSharedData
{
public:
    QXmppPubSubBaseItemPrivate(const QString &id, const QString &publisher);

    QString id;
    QString publisher;
};

QXmppPubSubBaseItemPrivate::QXmppPubSubBaseItemPrivate(const QString &id, const QString &publisher)
    : id(id), publisher(publisher)
{
}

///
/// \class QXmppPubSubBaseItem
///
/// The QXmppPubSubBaseItem class represents a publish-subscribe item as defined by
/// \xep{0060, Publish-Subscribe}.
///
/// To access the payload of an item, you need to create a derived class of this
/// and override QXmppPubSubBaseItem::parsePayload() and
/// QXmppPubSubBaseItem::serializePayload().
///
/// It is also required that you override QXmppPubSubBaseItem::isItem() and also
/// check for the correct payload of the PubSub item. This can be easily done by
/// using the protected overload of isItem() with an function that checks the
/// tag name and namespace of the payload. The function is only called if a
/// payload exists.
///
/// In short, you need to reimplement these methods:
///  * QXmppPubSubBaseItem::parsePayload()
///  * QXmppPubSubBaseItem::serializePayload()
///  * QXmppPubSubBaseItem::isItem()
///
/// \since QXmpp 1.5
///

///
/// Constructs an item with \a id and \a publisher.
///
/// \param id
/// \param publisher
///
QXmppPubSubBaseItem::QXmppPubSubBaseItem(const QString &id, const QString &publisher)
    : d(new QXmppPubSubBaseItemPrivate(id, publisher))
{
}

/// Default copy-constructor
QXmppPubSubBaseItem::QXmppPubSubBaseItem(const QXmppPubSubBaseItem &iq) = default;
/// Default move-constructor
QXmppPubSubBaseItem::QXmppPubSubBaseItem(QXmppPubSubBaseItem &&) = default;
QXmppPubSubBaseItem::~QXmppPubSubBaseItem() = default;
/// Default assignment operator
QXmppPubSubBaseItem &QXmppPubSubBaseItem::operator=(const QXmppPubSubBaseItem &iq) = default;
/// Default move-assignment operator
QXmppPubSubBaseItem &QXmppPubSubBaseItem::operator=(QXmppPubSubBaseItem &&iq) = default;

///
/// Returns the ID of the PubSub item.
///
QString QXmppPubSubBaseItem::id() const
{
    return d->id;
}

///
/// Sets the ID of the PubSub item.
///
/// \param id
///
void QXmppPubSubBaseItem::setId(const QString &id)
{
    d->id = id;
}

///
/// Returns the JID of the publisher of the item.
///
QString QXmppPubSubBaseItem::publisher() const
{
    return d->publisher;
}

///
/// Sets the JID of the publisher of the item.
///
void QXmppPubSubBaseItem::setPublisher(const QString &publisher)
{
    d->publisher = publisher;
}

/// \cond
void QXmppPubSubBaseItem::parse(const QDomElement &element)
{
    d->id = element.attribute(QStringLiteral("id"));
    d->publisher = element.attribute(QStringLiteral("publisher"));

    parsePayload(element.firstChildElement());
}

void QXmppPubSubBaseItem::toXml(QXmlStreamWriter *writer) const
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
bool QXmppPubSubBaseItem::isItem(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("item");
}

///
/// Parses the payload of the item (the child element of the &lt;item/&gt;).
///
/// This method needs to be overriden to perform the payload-specific parsing.
///
void QXmppPubSubBaseItem::parsePayload(const QDomElement &)
{
}

///
/// Serializes the payload of the item (the child element of the &lt;item/&gt;).
///
/// This method needs to be overriden to perform the payload-specific
/// serialization.
///
void QXmppPubSubBaseItem::serializePayload(QXmlStreamWriter *) const
{
}
