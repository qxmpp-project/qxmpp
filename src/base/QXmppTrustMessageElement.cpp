/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
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

#include "QXmppTrustMessageElement.h"

#include "QXmppConstants_p.h"
#include "QXmppTrustMessageKeyOwner.h"

#include <QDomElement>

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

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppTrustMessageElement::QXmppTrustMessageElement(const QXmppTrustMessageElement &other) = default;

QXmppTrustMessageElement::~QXmppTrustMessageElement() = default;

///
/// Assigns \a other to this trust message element.
///
/// \param other
///
QXmppTrustMessageElement &QXmppTrustMessageElement::operator=(const QXmppTrustMessageElement &other) = default;

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
    d->usage = element.attribute("usage");
    d->encryption = element.attribute("encryption");

    for (auto keyOwnerElement = element.firstChildElement("key-owner");
         !keyOwnerElement.isNull();
         keyOwnerElement = keyOwnerElement.nextSiblingElement("key-owner")) {
        if (QXmppTrustMessageKeyOwner::isTrustMessageKeyOwner(keyOwnerElement)) {
            QXmppTrustMessageKeyOwner keyOwner;
            keyOwner.parse(keyOwnerElement);
            d->keyOwners.append(keyOwner);
        }
    }
}

void QXmppTrustMessageElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("trust-message");
    writer->writeDefaultNamespace(ns_tm);
    writer->writeAttribute("usage", d->usage);
    writer->writeAttribute("encryption", d->encryption);

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
    return element.tagName() == QStringLiteral("trust-message") &&
        element.namespaceURI() == ns_tm;
}
