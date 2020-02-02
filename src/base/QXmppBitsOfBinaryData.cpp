/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
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

#include "QXmppBitsOfBinaryData.h"

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSharedData>
#include <QXmlStreamWriter>

class QXmppBitsOfBinaryDataPrivate : public QSharedData
{
public:
    QXmppBitsOfBinaryDataPrivate();

    QXmppBitsOfBinaryContentId cid;
    int maxAge;
    QMimeType contentType;
    QByteArray data;
};

QXmppBitsOfBinaryDataPrivate::QXmppBitsOfBinaryDataPrivate()
    : maxAge(-1)
{
}

QXmppBitsOfBinaryData::QXmppBitsOfBinaryData()
    : d(new QXmppBitsOfBinaryDataPrivate)
{
}

QXmppBitsOfBinaryData::QXmppBitsOfBinaryData(const QXmppBitsOfBinaryData &) = default;

QXmppBitsOfBinaryData::~QXmppBitsOfBinaryData() = default;

QXmppBitsOfBinaryData &QXmppBitsOfBinaryData::operator=(const QXmppBitsOfBinaryData &) = default;

/// Returns the content id of the data

QXmppBitsOfBinaryContentId QXmppBitsOfBinaryData::cid() const
{
    return d->cid;
}

/// Sets the content id of the data

void QXmppBitsOfBinaryData::setCid(const QXmppBitsOfBinaryContentId &cid)
{
    d->cid = cid;
}

/// Returns the time in seconds the data should be cached
///
/// A value of 0 means that the data should not be cached, while a value of -1
/// means that nothing was set.
///
/// The default value is -1.

int QXmppBitsOfBinaryData::maxAge() const
{
    return d->maxAge;
}

/// Sets the time in seconds the data should be cached
///
/// A value of 0 means that the data should not be cached, while a value of -1
/// means that nothing was set.
///
/// The default value is -1.

void QXmppBitsOfBinaryData::setMaxAge(int maxAge)
{
    d->maxAge = maxAge;
}

/// Returns the content type of the data
///
/// \note This is the advertised content type and may differ from the actual
/// content type of the data.

QMimeType QXmppBitsOfBinaryData::contentType() const
{
    return d->contentType;
}

/// Sets the content type of the data

void QXmppBitsOfBinaryData::setContentType(const QMimeType &contentType)
{
    d->contentType = contentType;
}

/// Returns the included data in binary form

QByteArray QXmppBitsOfBinaryData::data() const
{
    return d->data;
}

/// Sets the data in binary form

void QXmppBitsOfBinaryData::setData(const QByteArray &data)
{
    d->data = data;
}

/// Returns true, if \c element is a XEP-0231: Bits of Binary data element

bool QXmppBitsOfBinaryData::isBitsOfBinaryData(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("data") && element.namespaceURI() == ns_bob;
}

/// \cond
void QXmppBitsOfBinaryData::parseElementFromChild(const QDomElement &dataElement)
{
    d->cid = QXmppBitsOfBinaryContentId::fromContentId(dataElement.attribute("cid"));
    d->maxAge = dataElement.attribute("max-age", "-1").toInt();
    d->contentType = QMimeDatabase().mimeTypeForName(dataElement.attribute("type"));
    d->data = QByteArray::fromBase64(dataElement.text().toUtf8());
}

void QXmppBitsOfBinaryData::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("data");
    writer->writeDefaultNamespace(ns_bob);
    helperToXmlAddAttribute(writer, "cid", d->cid.toContentId());
    if (d->maxAge > -1)
        helperToXmlAddAttribute(writer, "max-age", QString::number(d->maxAge));
    helperToXmlAddAttribute(writer, "type", d->contentType.name());
    writer->writeCharacters(d->data.toBase64());
    writer->writeEndElement();
}
/// \endcond

bool QXmppBitsOfBinaryData::operator==(const QXmppBitsOfBinaryData &other) const
{
    return d->cid == other.cid() &&
           d->maxAge == other.maxAge() &&
           d->contentType == other.contentType() &&
           d->data == other.data();
}

