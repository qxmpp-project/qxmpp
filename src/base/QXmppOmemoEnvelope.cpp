/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Germán Márquez Mejía
 *  Melvin Keskin
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

#include "QXmppOmemoEnvelope.h"

#include <QDomElement>

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

///
/// \class QXmppOmemoEnvelope
///
/// \brief The QXmppOmemoEnvelope class represents an OMEMO envelope as
/// defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoEnvelopePrivate : public QSharedData
{
public:
    int recipientDeviceId = 0;
    bool isUsedForKeyExchange = false;
    QByteArray data;
};

///
/// Constructs an OMEMO envelope.
///
QXmppOmemoEnvelope::QXmppOmemoEnvelope() : d(new QXmppOmemoEnvelopePrivate) {}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoEnvelope::QXmppOmemoEnvelope(const QXmppOmemoEnvelope &other) : d(other.d) {}

QXmppOmemoEnvelope::~QXmppOmemoEnvelope() = default;

///
/// Assigns \a other to this OMEMO envelope.
///
/// \param other
///
QXmppOmemoEnvelope &QXmppOmemoEnvelope::operator=(const QXmppOmemoEnvelope &other)
{
    d = other.d;
    return *this;
}

///
/// Returns the ID of the recipient's device.
///
/// The ID is 0 if it is unset.
///
/// \return the recipient's device ID
///
int QXmppOmemoEnvelope::recipientDeviceId() const
{
    return d->recipientDeviceId;
}

///
/// Sets the ID of the recipient's device.
///
/// The ID must be a positive value, otherwise it is ignored.
///
/// \param id recipient's device ID
///
void QXmppOmemoEnvelope::setRecipientDeviceId(const int id)
{
    if (id > 0) {
        d->recipientDeviceId = id;
    }
}

///
/// Returns true if a pre-key was used to prepare this envelope.
///
/// The default is false.
///
/// \return true if a pre-key was used to prepare this envelope, otherwise false
///
bool QXmppOmemoEnvelope::isUsedForKeyExchange() const
{
    return d->isUsedForKeyExchange;
}

///
/// Sets whether a pre-key was used to prepare this envelope.
///
/// \param isUsed whether a pre-key was used to prepare this envelope
///
void QXmppOmemoEnvelope::setIsUsedForKeyExchange(const bool isUsed)
{
    d->isUsedForKeyExchange = isUsed;
}

///
/// Returns the BLOB containing the data for the underlying double ratchet library.
///
/// It should be treated like an obscure BLOB being passed as is to the ratchet
/// library for further processing.
///
/// \return the binary data for the ratchet library
///
QByteArray QXmppOmemoEnvelope::data() const
{
    return d->data;
}

///
/// Sets the BLOB containing the data from the underlying double ratchet library.
///
/// It should be treated like an obscure BLOB produced by the ratchet library.
///
/// \param data binary data from the ratchet library
///
void QXmppOmemoEnvelope::setData(const QByteArray &data)
{
    d->data = data;
}

/// \cond
void QXmppOmemoEnvelope::parse(const QDomElement &element)
{
    d->recipientDeviceId = element.attribute("rid").toInt();

    const auto isUsedForKeyExchange = element.attribute("kex");
    if (isUsedForKeyExchange == "true" || isUsedForKeyExchange == "1") {
        d->isUsedForKeyExchange = true;
    }

    d->data = QByteArray::fromBase64(element.text().toLatin1());
}

void QXmppOmemoEnvelope::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("key");
    writer->writeAttribute("rid", QString::number(d->recipientDeviceId));

    if (d->isUsedForKeyExchange) {
        helperToXmlAddAttribute(writer, "kex", "true");
    }

    writer->writeCharacters(d->data.toBase64());
    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO envelope.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO envelope, otherwise false
///
bool QXmppOmemoEnvelope::isOmemoEnvelope(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("key") &&
        element.namespaceURI() == ns_omemo_1;
}
