// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOmemoDeviceBundle_p.h"
#include "QXmppOmemoDeviceElement_p.h"
#include "QXmppOmemoDeviceList_p.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoIq_p.h"
#include "QXmppOmemoItems_p.h"

#include <QDomElement>
#include <QHash>

constexpr auto ns_omemo_2 = "urn:xmpp:omemo:2";

/// \cond
///
/// \class QXmppOmemoDeviceElement
///
/// \brief The QXmppOmemoDeviceElement class represents an element of the
/// OMEMO device list as defined by \xep{0384, OMEMO Encryption}.
///

///
/// Returns true if the IDs of both elements match.
///
bool QXmppOmemoDeviceElement::operator==(const QXmppOmemoDeviceElement &other) const
{
    return m_id == other.id();
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
    return m_id;
}

///
/// Sets the ID of this device element.
///
/// The ID must be at least 1 and at most
/// \c std::numeric_limits<int32_t>::max().
///
/// \param id this device element's ID
///
void QXmppOmemoDeviceElement::setId(uint32_t id)
{
    m_id = id;
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
    return m_label;
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
    m_label = label;
}

void QXmppOmemoDeviceElement::parse(const QDomElement &element)
{
    m_id = element.attribute(QStringLiteral("id")).toInt();
    m_label = element.attribute(QStringLiteral("label"));
}

void QXmppOmemoDeviceElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("device"));

    writer->writeAttribute(QStringLiteral("id"), QString::number(m_id));
    if (!m_label.isEmpty()) {
        writer->writeAttribute(QStringLiteral("label"), m_label);
    }

    writer->writeEndElement();  // device
}

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
        element.namespaceURI() == ns_omemo_2;
}

///
/// \class QXmppOmemoDeviceList
///
/// \brief The QXmppOmemoDeviceList class represents an OMEMO device list
/// as defined by \xep{0384, OMEMO Encryption}.
///

void QXmppOmemoDeviceList::parse(const QDomElement &element)
{
    for (auto device = element.firstChildElement(QStringLiteral("device"));
         !device.isNull();
         device = device.nextSiblingElement(QStringLiteral("device"))) {
        QXmppOmemoDeviceElement deviceElement;
        deviceElement.parse(device);
        append(deviceElement);
    }
}

void QXmppOmemoDeviceList::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("devices"));
    writer->writeDefaultNamespace(ns_omemo_2);

    for (const auto &device : *this) {
        device.toXml(writer);
    }

    writer->writeEndElement();
}

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
        element.namespaceURI() == ns_omemo_2;
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

///
/// Returns the public identity key.
///
/// The public identity key is the public long-term key which never changes.
///
/// \return the public identity key
///
QByteArray QXmppOmemoDeviceBundle::publicIdentityKey() const
{
    return m_publicIdentityKey;
}

///
/// Sets the public identity key.
///
/// \param key public identity key
///
void QXmppOmemoDeviceBundle::setPublicIdentityKey(const QByteArray &key)
{
    m_publicIdentityKey = key;
}

///
/// Returns the public pre key that is signed.
///
/// \return the signed public pre key
///
QByteArray QXmppOmemoDeviceBundle::signedPublicPreKey() const
{
    return m_signedPublicPreKey;
}

///
/// Sets the public pre key that is signed.
///
/// \param key signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKey(const QByteArray &key)
{
    m_signedPublicPreKey = key;
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
    return m_signedPublicPreKeyId;
}

///
/// Sets the ID of the public pre key that is signed.
///
/// The ID must be at least 1 and at most
/// \c std::numeric_limits<int32_t>::max().
///
/// \param id ID of the signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKeyId(uint32_t id)
{
    m_signedPublicPreKeyId = id;
}

///
/// Returns the signature of the public pre key that is signed.
///
/// \return the signature of the signed public pre key
///
QByteArray QXmppOmemoDeviceBundle::signedPublicPreKeySignature() const
{
    return m_signedPublicPreKeySignature;
}

///
/// Sets the signature of the public pre key that is signed.
///
/// \param signature signature of the signed public pre key
///
void QXmppOmemoDeviceBundle::setSignedPublicPreKeySignature(const QByteArray &signature)
{
    m_signedPublicPreKeySignature = signature;
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
QHash<uint32_t, QByteArray> QXmppOmemoDeviceBundle::publicPreKeys() const
{
    return m_publicPreKeys;
}

///
/// Adds a public pre key.
///
/// The ID must be at least 1 and at most
/// \c std::numeric_limits<int32_t>::max().
///
/// \param id ID of the public pre key
/// \param key public pre key
///
void QXmppOmemoDeviceBundle::addPublicPreKey(uint32_t id, const QByteArray &key)
{
    m_publicPreKeys.insert(id, key);
}

///
/// Removes a public pre key.
///
/// \param id ID of the public pre key
/// \param key public pre key
///
void QXmppOmemoDeviceBundle::removePublicPreKey(uint32_t id)
{
    m_publicPreKeys.remove(id);
}

void QXmppOmemoDeviceBundle::parse(const QDomElement &element)
{
    m_publicIdentityKey = QByteArray::fromBase64(element.firstChildElement(QStringLiteral("ik")).text().toLatin1());

    const auto signedPublicPreKeyElement = element.firstChildElement(QStringLiteral("spk"));
    if (!signedPublicPreKeyElement.isNull()) {
        m_signedPublicPreKeyId = signedPublicPreKeyElement.attribute(QStringLiteral("id")).toInt();
        m_signedPublicPreKey = QByteArray::fromBase64(signedPublicPreKeyElement.text().toLatin1());
    }
    m_signedPublicPreKeySignature = QByteArray::fromBase64(element.firstChildElement(QStringLiteral("spks")).text().toLatin1());

    const auto publicPreKeysElement = element.firstChildElement(QStringLiteral("prekeys"));
    if (!publicPreKeysElement.isNull()) {
        for (QDomElement publicPreKeyElement = publicPreKeysElement.firstChildElement(QStringLiteral("pk"));
             !publicPreKeyElement.isNull();
             publicPreKeyElement = publicPreKeyElement.nextSiblingElement(QStringLiteral("pk"))) {
            m_publicPreKeys.insert(publicPreKeyElement.attribute(QStringLiteral("id")).toInt(), QByteArray::fromBase64(publicPreKeyElement.text().toLatin1()));
        }
    }
}

void QXmppOmemoDeviceBundle::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("bundle"));
    writer->writeDefaultNamespace(ns_omemo_2);

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
    for (auto it = m_publicPreKeys.cbegin(); it != m_publicPreKeys.cend(); it++) {
        writer->writeStartElement(QStringLiteral("pk"));
        writer->writeAttribute(QStringLiteral("id"), QString::number(it.key()));
        writer->writeCharacters(it.value().toBase64());
        writer->writeEndElement();
    }
    writer->writeEndElement();  // prekeys

    writer->writeEndElement();  // bundle
}

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
        element.namespaceURI() == ns_omemo_2;
}

///
/// \class QXmppOmemoIq
///
/// \brief The QXmppOmemoIq class represents an encrypted IQ stanza as defined
/// by \xep{0384, OMEMO Encryption} and \xep{0420, Stanza Content Encryption}
/// (SCE).
///
/// \ingroup Stanzas
///

///
/// Returns the OMEMO element which contains the data used by OMEMO.
///
/// \return the OMEMO element
///
QXmppOmemoElement QXmppOmemoIq::omemoElement()
{
    return m_omemoElement;
}

///
/// Sets the OMEMO element which contains the data used by OMEMO.
///
/// \param omemoElement OMEMO element
///
void QXmppOmemoIq::setOmemoElement(const QXmppOmemoElement &omemoElement)
{
    m_omemoElement = omemoElement;
}

void QXmppOmemoIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    m_omemoElement.parse(child);
}

void QXmppOmemoIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    m_omemoElement.toXml(writer);
}

///
/// Determines whether the given DOM element is an OMEMO IQ stanza.
///
/// \param element DOM element being checked
///
/// \return true if element is an OMEMO IQ stanza, otherwise false
///
bool QXmppOmemoIq::isOmemoIq(const QDomElement &element)
{
    auto child = element.firstChildElement();
    return !child.isNull() && QXmppOmemoElement::isOmemoElement(child);
}

QXmppOmemoDeviceBundle QXmppOmemoDeviceBundleItem::deviceBundle() const
{
    return m_deviceBundle;
}

void QXmppOmemoDeviceBundleItem::setDeviceBundle(const QXmppOmemoDeviceBundle &deviceBundle)
{
    m_deviceBundle = deviceBundle;
}

bool QXmppOmemoDeviceBundleItem::isItem(const QDomElement &itemElement)
{
    return QXmppPubSubBaseItem::isItem(itemElement, QXmppOmemoDeviceBundle::isOmemoDeviceBundle);
}

void QXmppOmemoDeviceBundleItem::parsePayload(const QDomElement &payloadElement)
{
    m_deviceBundle.parse(payloadElement);
}

void QXmppOmemoDeviceBundleItem::serializePayload(QXmlStreamWriter *writer) const
{
    m_deviceBundle.toXml(writer);
}

QXmppOmemoDeviceList QXmppOmemoDeviceListItem::deviceList() const
{
    return m_deviceList;
}

void QXmppOmemoDeviceListItem::setDeviceList(const QXmppOmemoDeviceList &deviceList)
{
    m_deviceList = deviceList;
}

bool QXmppOmemoDeviceListItem::isItem(const QDomElement &itemElement)
{
    return QXmppPubSubBaseItem::isItem(itemElement, QXmppOmemoDeviceList::isOmemoDeviceList);
}

void QXmppOmemoDeviceListItem::parsePayload(const QDomElement &payloadElement)
{
    m_deviceList.parse(payloadElement);
}

void QXmppOmemoDeviceListItem::serializePayload(QXmlStreamWriter *writer) const
{
    m_deviceList.toXml(writer);
}
/// \endcond
