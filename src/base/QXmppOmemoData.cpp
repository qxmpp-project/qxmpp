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

#include "QXmppConstants_p.h"
#include "QXmppOmemoDeviceBundle.h"
#include "QXmppOmemoDeviceElement.h"
#include "QXmppOmemoDeviceList.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QMap>

///
/// \class QXmppOmemoDeviceElement
///
/// \brief The QXmppOmemoDeviceElement class represents an element of the
/// OMEMO device list as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

class QXmppOmemoDeviceElementPrivate : public QSharedData
{
public:
    uint32_t id = 0;
    QString label;
};

///
/// Constructs an OMEMO device element.
///
QXmppOmemoDeviceElement::QXmppOmemoDeviceElement()
    : d(new QXmppOmemoDeviceElementPrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceElement::QXmppOmemoDeviceElement(const QXmppOmemoDeviceElement &other) = default;

QXmppOmemoDeviceElement::~QXmppOmemoDeviceElement() = default;

///
/// Assigns \a other to this OMEMO device element.
///
/// \param other
///
QXmppOmemoDeviceElement &QXmppOmemoDeviceElement::operator=(const QXmppOmemoDeviceElement &other) = default;

///
/// Returns true if the IDs of both elements match.
///
bool QXmppOmemoDeviceElement::operator==(const QXmppOmemoDeviceElement &other) const
{
    return d->id == other.id();
}

///
/// Returns the ID of this device element.
///
/// The ID is used to identify a device and fetch its bundle.
/// The ID is 0 if it is unset.
///
/// \see QXmppOmemoDeviceBundle
///
/// \return this device element's ID
///
uint32_t QXmppOmemoDeviceElement::id() const
{
    return d->id;
}

///
/// Sets the ID of this device element.
///
/// A valid ID must be at least 1 and at most 2^32-1.
///
/// \param id this device element's ID
///
void QXmppOmemoDeviceElement::setId(const uint32_t id)
{
    d->id = id;
}

///
/// Returns the label of this device element.
///
/// The label is a human-readable string used to identify the device by users.
/// If no label is set, a default-constructed QString is returned.
///
/// \return this device element's label
///
QString QXmppOmemoDeviceElement::label() const
{
    return d->label;
}

///
/// Sets the optional label of this device element.
///
/// The label should not contain more than 53 characters.
///
/// \param label this device element's label
///
void QXmppOmemoDeviceElement::setLabel(const QString &label)
{
    d->label = label;
}

/// \cond
void QXmppOmemoDeviceElement::parse(const QDomElement &element)
{
    d->id = element.attribute("id").toInt();
    d->label = element.attribute("label");
}

void QXmppOmemoDeviceElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("device");

    writer->writeAttribute("id", QString::number(d->id));
    if (!d->label.isEmpty()) {
        writer->writeAttribute("label", d->label);
    }

    writer->writeEndElement();  // device
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device element.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device element, otherwise false
///
bool QXmppOmemoDeviceElement::isOmemoDeviceElement(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("device") &&
        element.namespaceURI() == ns_omemo_1;
}

///
/// \class QXmppOmemoDeviceList
///
/// \brief The QXmppOmemoDeviceList class represents an OMEMO device list
/// as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///

///
/// Constructs an OMEMO device list.
///
QXmppOmemoDeviceList::QXmppOmemoDeviceList()
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceList::QXmppOmemoDeviceList(const QXmppOmemoDeviceList &other) = default;

QXmppOmemoDeviceList::~QXmppOmemoDeviceList() = default;

///
/// Assigns \a other to this OMEMO device list.
///
/// \param other
///
QXmppOmemoDeviceList &QXmppOmemoDeviceList::operator=(const QXmppOmemoDeviceList &other) = default;

/// \cond
void QXmppOmemoDeviceList::parse(const QDomElement &element)
{
    for (auto device = element.firstChildElement("device");
         !device.isNull();
         device = device.nextSiblingElement("device")) {
        QXmppOmemoDeviceElement deviceElement;
        deviceElement.parse(device);
        append(deviceElement);
    }
}

void QXmppOmemoDeviceList::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("devices");
    writer->writeDefaultNamespace(ns_omemo_1);

    for (const auto &device : *this) {
        device.toXml(writer);
    }

    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device list.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device list, otherwise false
///
bool QXmppOmemoDeviceList::isOmemoDeviceList(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("devices") &&
        element.namespaceURI() == ns_omemo_1;
}

///
/// \class QXmppOmemoDeviceBundle
///
/// \brief The QXmppOmemoDeviceBundle class represents an OMEMO bundle as
/// defined by \xep{0384, OMEMO Encryption}.
///
/// It is a collection of publicly accessible data used by the X3DH key exchange.
/// The data is used to build an encrypted session with an OMEMO device.
///
/// \since QXmpp 1.5
///

class QXmppOmemoDeviceBundlePrivate : public QSharedData
{
public:
    QByteArray publicIdentityKey;
    QByteArray signedPublicPreKey;
    uint32_t signedPublicPreKeyId = 0;
    QByteArray signedPublicPreKeySignature;
    QMap<uint32_t, QByteArray> publicPreKeys;
};

///
/// Constructs an OMEMO device bundle.
///
QXmppOmemoDeviceBundle::QXmppOmemoDeviceBundle()
    : d(new QXmppOmemoDeviceBundlePrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppOmemoDeviceBundle::QXmppOmemoDeviceBundle(const QXmppOmemoDeviceBundle &other) = default;

QXmppOmemoDeviceBundle::~QXmppOmemoDeviceBundle() = default;

///
/// Assigns \a other to this OMEMO device bundle.
///
/// \param other
///
QXmppOmemoDeviceBundle &QXmppOmemoDeviceBundle::operator=(const QXmppOmemoDeviceBundle &other) = default;

///
/// Returns the public identity key.
///
/// The public identity key is the public long-term key which never changes.
///
/// \return the public identity key
///
QByteArray QXmppOmemoDeviceBundle::publicIdentityKey() const
{
    return d->publicIdentityKey;
}

///
/// Sets the public identity key.
///
/// \param key public identity key
///
void QXmppOmemoDeviceBundle::setPublicIdentityKey(const QByteArray &key)
{
    d->publicIdentityKey = key;
}

///
/// Returns the public pre key that is signed.
///
/// \return the signed public pre key
///
QByteArray QXmppOmemoDeviceBundle::signedPublicPreKey() const
{
    return d->signedPublicPreKey;
}

///
/// Sets the public pre key that is signed.
///
/// \param key signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKey(const QByteArray &key)
{
    d->signedPublicPreKey = key;
}

///
/// Returns the ID of the public pre key that is signed.
///
/// The ID is 0 if it is unset.
///
/// \return the ID of the signed public pre key
///
uint32_t QXmppOmemoDeviceBundle::signedPublicPreKeyId() const
{
    return d->signedPublicPreKeyId;
}

///
/// Sets the ID of the public pre key that is signed.
///
/// A valid ID must be at least 1 and at most 2^32-1.
///
/// \param id ID of the signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKeyId(const uint32_t id)
{
    d->signedPublicPreKeyId = id;
}

///
/// Returns the signature of the public pre key that is signed.
///
/// \return the signature of the signed public pre key
///
QByteArray QXmppOmemoDeviceBundle::signedPublicPreKeySignature() const
{
    return d->signedPublicPreKeySignature;
}

///
/// Returns the signature of the public pre key that is signed.
///
/// \param signature signature of the signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKeySignature(const QByteArray &signature)
{
    d->signedPublicPreKeySignature = signature;
}

///
/// Returns the public pre keys.
///
/// The key of a key-value pair represents the ID of the corresponding public
/// pre key.
/// The value of a key-value pair represents the public pre key.
///
/// \return the public pre keys
///
QMap<uint32_t, QByteArray> QXmppOmemoDeviceBundle::publicPreKeys() const
{
    return d->publicPreKeys;
}

///
/// Sets the public pre keys.
///
/// The key of a key-value pair represents the ID of the corresponding public
/// pre key.
/// The ID must be at least 1 and at most 2^32-1, otherwise the corresponding
/// key-value pair is ignored.
/// The value of a key-value pair represents the public pre key.
///
/// \param keys public pre keys
///
void QXmppOmemoDeviceBundle::setPublicPreKeys(const QMap<uint32_t, QByteArray> &keys)
{
    for (auto it = keys.cbegin(); it != keys.cend(); it++) {
        if (it.key() > 0) {
            d->publicPreKeys.insert(it.key(), it.value());
        }
    }
}

/// \cond
void QXmppOmemoDeviceBundle::parse(const QDomElement &element)
{
    d->publicIdentityKey = QByteArray::fromBase64(element.firstChildElement(QStringLiteral("ik")).text().toLatin1());

    const auto signedPublicPreKeyElement = element.firstChildElement(QStringLiteral("spk"));
    if (!signedPublicPreKeyElement.isNull()) {
        d->signedPublicPreKeyId = signedPublicPreKeyElement.attribute(QStringLiteral("id")).toInt();
        d->signedPublicPreKey = QByteArray::fromBase64(signedPublicPreKeyElement.text().toLatin1());
    }
    d->signedPublicPreKeySignature = QByteArray::fromBase64(element.firstChildElement(QStringLiteral("spks")).text().toLatin1());

    const auto publicPreKeysElement = element.firstChildElement(QStringLiteral("prekeys"));
    if (!publicPreKeysElement.isNull()) {
        for (QDomElement publicPreKeyElement = publicPreKeysElement.firstChildElement(QStringLiteral("pk"));
             !publicPreKeyElement.isNull();
             publicPreKeyElement = publicPreKeyElement.nextSiblingElement(QStringLiteral("pk"))) {
            d->publicPreKeys.insert(publicPreKeyElement.attribute(QStringLiteral("id")).toInt(), QByteArray::fromBase64(publicPreKeyElement.text().toLatin1()));
        }
    }
}

void QXmppOmemoDeviceBundle::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("bundle"));
    writer->writeDefaultNamespace(ns_omemo_1);

    writer->writeStartElement(QStringLiteral("ik"));
    writer->writeCharacters(publicIdentityKey().toBase64());
    writer->writeEndElement();

    writer->writeStartElement(QStringLiteral("spk"));
    writer->writeAttribute(QStringLiteral("id"), QString::number(signedPublicPreKeyId()));
    writer->writeCharacters(signedPublicPreKey().toBase64());
    writer->writeEndElement();

    writer->writeStartElement(QStringLiteral("spks"));
    writer->writeCharacters(signedPublicPreKeySignature().toBase64());
    writer->writeEndElement();

    writer->writeStartElement(QStringLiteral("prekeys"));
    for (auto it = d->publicPreKeys.cbegin(); it != d->publicPreKeys.cend(); it++) {
        writer->writeStartElement(QStringLiteral("pk"));
        writer->writeAttribute(QStringLiteral("id"), QString::number(it.key()));
        writer->writeCharacters(it.value().toBase64());
        writer->writeEndElement();
    }
    writer->writeEndElement();  // prekeys

    writer->writeEndElement();  // bundle
}
/// \endcond

///
/// Determines whether the given DOM element is an OMEMO device bundle.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO device bundle, otherwise false
///
bool QXmppOmemoDeviceBundle::isOmemoDeviceBundle(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("bundle") &&
        element.namespaceURI() == ns_omemo_1;
}
