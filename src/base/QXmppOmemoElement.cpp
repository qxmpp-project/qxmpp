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

#include "QXmppOmemoElement.h"

#include <QDomElement>

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

///
/// \class QXmppOmemoElement
///
/// \brief The QXmppOmemoElement class represents an OMEMO element as
/// defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoElementPrivate : public QSharedData
{
public:
        int senderDeviceId = 0;
        QByteArray payload;
        QMultiMap<QString, QXmppOmemoEnvelope> envelopes;
};

///
/// Constructs an OMEMO element.
///
QXmppOmemoElement::QXmppOmemoElement() : d(new QXmppOmemoElementPrivate) {}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoElement::QXmppOmemoElement(const QXmppOmemoElement &other) : d(other.d) {}

QXmppOmemoElement::~QXmppOmemoElement() = default;

///
/// Assigns \a other to this OMEMO element.
///
/// \param other
///
QXmppOmemoElement &QXmppOmemoElement::operator=(const QXmppOmemoElement &other)
{
    d = other.d;
    return *this;
}

///
/// Returns the ID of the sender's device.
///
/// The ID is 0 if it is unset.
///
/// \return the sender's device ID
///
int QXmppOmemoElement::senderDeviceId() const
{
    return d->senderDeviceId;
}

///
/// Sets the ID of the sender's device.
///
/// The ID must be a positive value, otherwise it is ignored.
///
/// \param id sender's device ID
///
void QXmppOmemoElement::setSenderDeviceId(const int id)
{
    if (id > 0) {
        d->senderDeviceId = id;
    }
}

///
/// Returns the payload which consists of the encrypted SCE envelope.
///
/// \return the encrypted payload
///
QByteArray QXmppOmemoElement::payload() const
{
    return d->payload;
}

///
/// Sets the payload which consists of the encrypted SCE envelope.
///
/// \param payload encrypted payload
///
void QXmppOmemoElement::setPayload(const QByteArray &payload)
{
    d->payload = payload;
}

///
/// Searches for an OMEMO envelope by its recipient JID and device ID.
///
/// \param recipientJid bare JID of the recipient
/// \param recipientDeviceId ID of the recipient's device
///
/// \return the found OMEMO envelope
///
std::optional<QXmppOmemoEnvelope> QXmppOmemoElement::searchEnvelope(const QString &recipientJid, int recipientDeviceId) const
{
    for (const auto &envelope : d->envelopes.values(recipientJid)) {
        if (envelope.recipientDeviceId() == recipientDeviceId) {
            return envelope;
        }
    }

    return std::nullopt;
}

///
/// Adds an OMEMO envelope.
///
/// If a full JID is passed as recipientJid, it is converted into a bare JID.
///
/// \see QXmppOmemoEnvelope
///
/// \param recipientJid bare JID of the recipient
/// \param envelope OMEMO envelope
///
void QXmppOmemoElement::addEnvelope(const QString &recipientJid, QXmppOmemoEnvelope &envelope)
{
    d->envelopes.insert(QXmppUtils::jidToBareJid(recipientJid), envelope);
}

/// \cond
void QXmppOmemoElement::parse(const QDomElement &element)
{
    const auto header = element.firstChildElement("header");

    d->senderDeviceId = header.attribute("sid").toInt();

    auto recipient = header.firstChildElement("keys");
    while (!recipient.isNull()) {
        const auto recipientJid = recipient.attribute("jid");
        auto envelope = recipient.firstChildElement("key");

        while (!envelope.isNull()) {
            QXmppOmemoEnvelope omemoEnvelope;
            omemoEnvelope.parse(envelope);
            addEnvelope(recipientJid, omemoEnvelope);

            envelope = envelope.nextSiblingElement("key");
        }

        recipient = recipient.nextSiblingElement("keys");
    }

    d->payload = QByteArray::fromBase64(element.firstChildElement("payload").text().toLatin1());
}

void QXmppOmemoElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("encrypted");
    writer->writeAttribute("xmlns", ns_omemo_1);

    writer->writeStartElement("header");
    writer->writeAttribute("sid", QString::number(d->senderDeviceId));

    const auto recipientJids = d->envelopes.uniqueKeys();
    for (const auto &recipientJid : recipientJids) {
        writer->writeStartElement("keys");
        writer->writeAttribute("jid", recipientJid);

        for (const auto &envelope : d->envelopes.values(recipientJid)) {
            envelope.toXml(writer);
        }

        writer->writeEndElement(); // keys
    }

    writer->writeEndElement(); // header

    helperToXmlAddTextElement(writer, "payload", d->payload.toBase64());

    writer->writeEndElement(); // encrypted
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO element.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO element, otherwise false
///
bool QXmppOmemoElement::isOmemoElement(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("encrypted") &&
        element.namespaceURI() == ns_omemo_1;
}
