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

#include "QXmppTrustMessageKeyOwner.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

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
    QList<QString> trustedKeys;
    QList<QString> distrustedKeys;
};

///
/// Constructs a trust message key owner.
///
QXmppTrustMessageKeyOwner::QXmppTrustMessageKeyOwner()
    : d(new QXmppTrustMessageKeyOwnerPrivate)
{
}

///
/// Constructs a copy of \a other.
///
/// \param other
///
QXmppTrustMessageKeyOwner::QXmppTrustMessageKeyOwner(const QXmppTrustMessageKeyOwner &other) = default;

QXmppTrustMessageKeyOwner::~QXmppTrustMessageKeyOwner() = default;

///
/// Assigns \a other to this trust message key owner.
///
/// \param other
///
QXmppTrustMessageKeyOwner &QXmppTrustMessageKeyOwner::operator=(const QXmppTrustMessageKeyOwner &other) = default;

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
QList<QString> QXmppTrustMessageKeyOwner::trustedKeys() const
{
    return d->trustedKeys;
}

///
/// Sets the IDs of keys that are trusted.
///
/// \param keyIds IDs of trusted keys
///
void QXmppTrustMessageKeyOwner::setTrustedKeys(const QList<QString> &keyIds)
{
    d->trustedKeys = keyIds;
}

///
/// Returns the IDs of the keys that are distrusted.
///
/// \return the IDs of distrusted keys
///
QList<QString> QXmppTrustMessageKeyOwner::distrustedKeys() const
{
    return d->distrustedKeys;
}

///
/// Sets the IDs of keys that are distrusted.
///
/// \param keyIds IDs of distrusted keys
///
void QXmppTrustMessageKeyOwner::setDistrustedKeys(const QList<QString> &keyIds)
{
    d->distrustedKeys = keyIds;
}

/// \cond
void QXmppTrustMessageKeyOwner::parse(const QDomElement &element)
{
    d->jid = element.attribute("jid");

    for (auto childElement = element.firstChildElement();
         !childElement.isNull();
         childElement = childElement.nextSiblingElement()) {
        if (childElement.tagName() == "trust") {
            d->trustedKeys.append(childElement.text());
        } else if (childElement.tagName() == "distrust") {
            d->distrustedKeys.append(childElement.text());
        }
    }
}

void QXmppTrustMessageKeyOwner::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("key-owner");
    writer->writeAttribute("jid", d->jid);

    for (const auto &keyIdentifier : d->trustedKeys) {
        writer->writeTextElement("trust", keyIdentifier);
    }

    for (const auto &keyIdentifier : d->distrustedKeys) {
        writer->writeTextElement("distrust", keyIdentifier);
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
    return element.tagName() == QStringLiteral("key-owner") &&
        element.namespaceURI() == ns_tm;
}
