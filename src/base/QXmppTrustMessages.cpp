// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTrustMessages.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// \class QXmppTrustMessageElement
///
/// \brief The QXmppTrustMessageElement class represents a trust message element
/// as defined by \xep{0434, Trust Messages (TM)}.
///
/// \since QXmpp 1.5
///

class QXmppTrustMessageElementPrivate : public QSharedData
{
public:
    QString usage;
    QString encryption;
    QList<QXmppTrustMessageKeyOwner> keyOwners;
};

///
/// Constructs a trust message element.
///
QXmppTrustMessageElement::QXmppTrustMessageElement()
    : d(new QXmppTrustMessageElementPrivate)
{
}

/// Copy-constructor.
QXmppTrustMessageElement::QXmppTrustMessageElement(const QXmppTrustMessageElement &other) = default;
/// Move-constructor.
QXmppTrustMessageElement::QXmppTrustMessageElement(QXmppTrustMessageElement &&) = default;
QXmppTrustMessageElement::~QXmppTrustMessageElement() = default;
/// Assignment operator.
QXmppTrustMessageElement &QXmppTrustMessageElement::operator=(const QXmppTrustMessageElement &other) = default;
/// Move-assignment operator.
QXmppTrustMessageElement &QXmppTrustMessageElement::operator=(QXmppTrustMessageElement &&) = default;

///
/// Returns the namespace of the trust management protocol.
///
/// \return the trust management protocol namespace
///
QString QXmppTrustMessageElement::usage() const
{
    return d->usage;
}

///
/// Sets the namespace of the trust management protocol.
///
/// \param usage trust management protocol namespace
///
void QXmppTrustMessageElement::setUsage(const QString &usage)
{
    d->usage = usage;
}

///
/// Returns the namespace of the keys' encryption protocol.
///
/// \return the encryption protocol namespace
///
QString QXmppTrustMessageElement::encryption() const
{
    return d->encryption;
}

///
/// Sets the namespace of the keys' encryption protocol.
///
/// \param encryption encryption protocol namespace
///
void QXmppTrustMessageElement::setEncryption(const QString &encryption)
{
    d->encryption = encryption;
}

///
/// Returns the key owners containing the corresponding information for
/// trusting or distrusting their keys.
///
/// \return the owners of the keys for trusting or distrusting
///
QList<QXmppTrustMessageKeyOwner> QXmppTrustMessageElement::keyOwners() const
{
    return d->keyOwners;
}

///
/// Sets the key owners containing the corresponding information for trusting or
/// distrusting their keys.
///
/// \param keyOwners owners of the keys for trusting or distrusting
///
void QXmppTrustMessageElement::setKeyOwners(const QList<QXmppTrustMessageKeyOwner> &keyOwners)
{
    d->keyOwners = keyOwners;
}

///
/// Adds a key owner containing the corresponding information for trusting or
/// distrusting the owners keys.
///
/// \param keyOwner owner of the keys for trusting or distrusting
///
void QXmppTrustMessageElement::addKeyOwner(const QXmppTrustMessageKeyOwner &keyOwner)
{
    d->keyOwners.append(keyOwner);
}

/// \cond
void QXmppTrustMessageElement::parse(const QDomElement &element)
{
    d->usage = element.attribute(u"usage"_s);
    d->encryption = element.attribute(u"encryption"_s);

    for (const auto &keyOwnerElement : iterChildElements(element, u"key-owner")) {
        if (QXmppTrustMessageKeyOwner::isTrustMessageKeyOwner(keyOwnerElement)) {
            QXmppTrustMessageKeyOwner keyOwner;
            keyOwner.parse(keyOwnerElement);
            d->keyOwners.append(keyOwner);
        }
    }
}

void QXmppTrustMessageElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("trust-message"));
    writer->writeDefaultNamespace(toString65(ns_tm));
    writer->writeAttribute(QSL65("usage"), d->usage);
    writer->writeAttribute(QSL65("encryption"), d->encryption);

    for (const auto &keyOwner : d->keyOwners) {
        keyOwner.toXml(writer);
    }

    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is a trust message element.
///
/// \param element DOM element being checked
///
/// \return true if element is a trust message element, otherwise false
///
bool QXmppTrustMessageElement::isTrustMessageElement(const QDomElement &element)
{
    return element.tagName() == u"trust-message" &&
        element.namespaceURI() == ns_tm;
}

///
/// \class QXmppTrustMessageKeyOwner
///
/// \brief The QXmppTrustMessageKeyOwner class represents a key owner of the
/// trust message as defined by \xep{0434, Trust Messages (TM)}.
///
/// \since QXmpp 1.5
///

class QXmppTrustMessageKeyOwnerPrivate : public QSharedData
{
public:
    QString jid;
    QList<QByteArray> trustedKeys;
    QList<QByteArray> distrustedKeys;
};

///
/// Constructs a trust message key owner.
///
QXmppTrustMessageKeyOwner::QXmppTrustMessageKeyOwner()
    : d(new QXmppTrustMessageKeyOwnerPrivate)
{
}

/// Copy constructor.
QXmppTrustMessageKeyOwner::QXmppTrustMessageKeyOwner(const QXmppTrustMessageKeyOwner &other) = default;
/// Copy constructor.
QXmppTrustMessageKeyOwner::QXmppTrustMessageKeyOwner(QXmppTrustMessageKeyOwner &&) = default;
QXmppTrustMessageKeyOwner::~QXmppTrustMessageKeyOwner() = default;
/// Assignment operator.
QXmppTrustMessageKeyOwner &QXmppTrustMessageKeyOwner::operator=(const QXmppTrustMessageKeyOwner &other) = default;
/// Assignment operator.
QXmppTrustMessageKeyOwner &QXmppTrustMessageKeyOwner::operator=(QXmppTrustMessageKeyOwner &&) = default;

///
/// Returns the bare JID of the key owner.
///
/// \return the key owner's bare JID
///
QString QXmppTrustMessageKeyOwner::jid() const
{
    return d->jid;
}

///
/// Sets the bare JID of the key owner.
///
/// If a full JID is passed, it is converted into a bare JID.
///
/// \param jid key owner's bare JID
///
void QXmppTrustMessageKeyOwner::setJid(const QString &jid)
{
    d->jid = QXmppUtils::jidToBareJid(jid);
}

///
/// Returns the IDs of the keys that are trusted.
///
/// \return the IDs of trusted keys
///
QList<QByteArray> QXmppTrustMessageKeyOwner::trustedKeys() const
{
    return d->trustedKeys;
}

///
/// Sets the IDs of keys that are trusted.
///
/// \param keyIds IDs of trusted keys
///
void QXmppTrustMessageKeyOwner::setTrustedKeys(const QList<QByteArray> &keyIds)
{
    d->trustedKeys = keyIds;
}

///
/// Returns the IDs of the keys that are distrusted.
///
/// \return the IDs of distrusted keys
///
QList<QByteArray> QXmppTrustMessageKeyOwner::distrustedKeys() const
{
    return d->distrustedKeys;
}

///
/// Sets the IDs of keys that are distrusted.
///
/// \param keyIds IDs of distrusted keys
///
void QXmppTrustMessageKeyOwner::setDistrustedKeys(const QList<QByteArray> &keyIds)
{
    d->distrustedKeys = keyIds;
}

/// \cond
void QXmppTrustMessageKeyOwner::parse(const QDomElement &element)
{
    d->jid = element.attribute(u"jid"_s);

    for (const auto &childElement : iterChildElements(element, u"trust")) {
        d->trustedKeys.append(QByteArray::fromBase64(childElement.text().toLatin1()));
    }
    for (const auto &childElement : iterChildElements(element, u"distrust")) {
        d->distrustedKeys.append(QByteArray::fromBase64(childElement.text().toLatin1()));
    }
}

void QXmppTrustMessageKeyOwner::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("key-owner"));
    writer->writeAttribute(QSL65("jid"), d->jid);

    for (const auto &keyIdentifier : d->trustedKeys) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeTextElement("trust", keyIdentifier.toBase64());
#else
        writer->writeTextElement(u"trust"_s, QString::fromUtf8(keyIdentifier.toBase64()));
#endif
    }

    for (const auto &keyIdentifier : d->distrustedKeys) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeTextElement("distrust", keyIdentifier.toBase64());
#else
        writer->writeTextElement(u"distrust"_s, QString::fromUtf8(keyIdentifier.toBase64()));
#endif
    }

    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is a trust message key owner.
///
/// \param element DOM element being checked
///
/// \return true if element is a trust message key owner, otherwise false
///
bool QXmppTrustMessageKeyOwner::isTrustMessageKeyOwner(const QDomElement &element)
{
    return element.tagName() == u"key-owner" &&
        element.namespaceURI() == ns_tm;
}
