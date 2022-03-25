// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppOmemoDeviceBundle_p.h"
#include "QXmppOmemoDeviceElement_p.h"
#include "QXmppOmemoDeviceList_p.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoEnvelope_p.h"
#include "QXmppOmemoIq_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QHash>

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
/// Returns the signature of the public pre key that is signed.
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
/// \class QXmppOmemoEnvelope
///
/// \brief The QXmppOmemoEnvelope class represents an OMEMO envelope as
/// defined by \xep{0384, OMEMO Encryption}.
///

///
/// Returns the ID of the recipient's device.
///
/// The ID is 0 if it is unset.
///
/// \return the recipient's device ID
///
uint32_t QXmppOmemoEnvelope::recipientDeviceId() const
{
    return m_recipientDeviceId;
}

///
/// Sets the ID of the recipient's device.
///
/// The ID must be at least 1 and at most
/// \c std::numeric_limits<int32_t>::max().
///
/// \param id recipient's device ID
///
void QXmppOmemoEnvelope::setRecipientDeviceId(uint32_t id)
{
    m_recipientDeviceId = id;
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
    return m_isUsedForKeyExchange;
}

///
/// Sets whether a pre-key was used to prepare this envelope.
///
/// \param isUsed whether a pre-key was used to prepare this envelope
///
void QXmppOmemoEnvelope::setIsUsedForKeyExchange(bool isUsed)
{
    m_isUsedForKeyExchange = isUsed;
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
    return m_data;
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
    m_data = data;
}

void QXmppOmemoEnvelope::parse(const QDomElement &element)
{
    m_recipientDeviceId = element.attribute(QStringLiteral("rid")).toInt();

    const auto isUsedForKeyExchange = element.attribute(QStringLiteral("kex"));
    if (isUsedForKeyExchange == QStringLiteral("true") || isUsedForKeyExchange == QStringLiteral("1")) {
        m_isUsedForKeyExchange = true;
    }

    m_data = QByteArray::fromBase64(element.text().toLatin1());
}

void QXmppOmemoEnvelope::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("key"));
    writer->writeAttribute(QStringLiteral("rid"), QString::number(m_recipientDeviceId));

    if (m_isUsedForKeyExchange) {
        helperToXmlAddAttribute(writer, QStringLiteral("kex"), QStringLiteral("true"));
    }

    writer->writeCharacters(m_data.toBase64());
    writer->writeEndElement();
}

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
        element.namespaceURI() == ns_omemo_2;
}

///
/// \class QXmppOmemoElement
///
/// \brief The QXmppOmemoElement class represents an OMEMO element as
/// defined by \xep{0384, OMEMO Encryption}.
///

///
/// Returns the ID of the sender's device.
///
/// The ID is 0 if it is unset.
///
/// \return the sender's device ID
///
uint32_t QXmppOmemoElement::senderDeviceId() const
{
    return m_senderDeviceId;
}

///
/// Sets the ID of the sender's device.
///
/// The ID must be at least 1 and at most
/// \c std::numeric_limits<int32_t>::max().
///
/// \param id sender's device ID
///
void QXmppOmemoElement::setSenderDeviceId(uint32_t id)
{
    m_senderDeviceId = id;
}

///
/// Returns the payload which consists of the encrypted SCE envelope.
///
/// \return the encrypted payload
///
QByteArray QXmppOmemoElement::payload() const
{
    return m_payload;
}

///
/// Sets the payload which consists of the encrypted SCE envelope.
///
/// \param payload encrypted payload
///
void QXmppOmemoElement::setPayload(const QByteArray &payload)
{
    m_payload = payload;
}

///
/// Searches for an OMEMO envelope by its recipient JID and device ID.
///
/// \param recipientJid bare JID of the recipient
/// \param recipientDeviceId ID of the recipient's device
///
/// \return the found OMEMO envelope
///
std::optional<QXmppOmemoEnvelope> QXmppOmemoElement::searchEnvelope(const QString &recipientJid, uint32_t recipientDeviceId) const
{
    for (auto itr = m_envelopes.constFind(recipientJid); itr != m_envelopes.constEnd() && itr.key() == recipientJid; ++itr) {
        const auto &envelope = itr.value();
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
void QXmppOmemoElement::addEnvelope(const QString &recipientJid, const QXmppOmemoEnvelope &envelope)
{
    m_envelopes.insert(QXmppUtils::jidToBareJid(recipientJid), envelope);
}

void QXmppOmemoElement::parse(const QDomElement &element)
{
    const auto header = element.firstChildElement(QStringLiteral("header"));

    m_senderDeviceId = header.attribute(QStringLiteral("sid")).toInt();

    for (auto recipient = header.firstChildElement(QStringLiteral("keys"));
         !recipient.isNull();
         recipient = recipient.nextSiblingElement(QStringLiteral("keys"))) {
        const auto recipientJid = recipient.attribute(QStringLiteral("jid"));

        for (auto envelope = recipient.firstChildElement(QStringLiteral("key"));
             !envelope.isNull();
             envelope = envelope.nextSiblingElement(QStringLiteral("key"))) {
            QXmppOmemoEnvelope omemoEnvelope;
            omemoEnvelope.parse(envelope);
            addEnvelope(recipientJid, omemoEnvelope);
        }
    }

    m_payload = QByteArray::fromBase64(element.firstChildElement(QStringLiteral("payload")).text().toLatin1());
}

void QXmppOmemoElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("encrypted"));
    writer->writeDefaultNamespace(ns_omemo_2);

    writer->writeStartElement(QStringLiteral("header"));
    writer->writeAttribute(QStringLiteral("sid"), QString::number(m_senderDeviceId));

    const auto recipientJids = m_envelopes.uniqueKeys();
    for (const auto &recipientJid : recipientJids) {
        writer->writeStartElement(QStringLiteral("keys"));
        writer->writeAttribute(QStringLiteral("jid"), recipientJid);

        for (auto itr = m_envelopes.constFind(recipientJid); itr != m_envelopes.constEnd() && itr.key() == recipientJid; ++itr) {
            const auto &envelope = itr.value();
            envelope.toXml(writer);
        }

        writer->writeEndElement();  // keys
    }

    writer->writeEndElement();  // header

    // The payload element is only included if there is a payload.
    // An empty OMEMO message does not contain a payload.
    if (!m_payload.isEmpty()) {
        writer->writeTextElement(QStringLiteral("payload"), m_payload.toBase64());
    }

    writer->writeEndElement();  // encrypted
}

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
/// \endcond
