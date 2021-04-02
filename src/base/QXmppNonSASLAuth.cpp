/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#include "QXmppNonSASLAuth.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QCryptographicHash>
#include <QDomElement>
#include <QXmlStreamWriter>

///
/// \class QXmppNonSASLAuthIq
///
/// QXmppNonSASLAuthIq represents a Non-SASL authentication IQ as defined by
/// \xep{0078, Non-SASL Authentication}.
///
/// \ingroup Stanzas
///

QXmppNonSASLAuthIq::QXmppNonSASLAuthIq()
    : QXmppIq(QXmppIq::Set)
{
}

///
/// Returns the username of the account
///
QString QXmppNonSASLAuthIq::username() const
{
    return m_username;
}

///
/// Sets the username of the account
///
void QXmppNonSASLAuthIq::setUsername(const QString &username)
{
    m_username = username;
}

///
/// Returns the SHA1 hash of the concatenated string
///
QByteArray QXmppNonSASLAuthIq::digest() const
{
    return m_digest;
}

///
/// Sets the digest by creating a hash of the concatenation of streamId and
/// password.
///
void QXmppNonSASLAuthIq::setDigest(const QString &streamId, const QString &password)
{
    m_digest = QCryptographicHash::hash(streamId.toUtf8() + password.toUtf8(), QCryptographicHash::Sha1);
}

///
/// Returns the username of the account in plaintext
///
QString QXmppNonSASLAuthIq::password() const
{
    return m_password;
}

///
/// Sets the username of the account in plaintext
///
void QXmppNonSASLAuthIq::setPassword(const QString &password)
{
    m_password = password;
}

///
/// Returns the resource to bind to
///
QString QXmppNonSASLAuthIq::resource() const
{
    return m_resource;
}

///
/// Sets the resource to bind to
///
void QXmppNonSASLAuthIq::setResource(const QString &resource)
{
    m_resource = resource;
}

/// \cond
bool QXmppNonSASLAuthIq::isNonSASLAuthIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    return queryElement.namespaceURI() == ns_auth;
}

void QXmppNonSASLAuthIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    m_username = queryElement.firstChildElement(QStringLiteral("username")).text();
    m_password = queryElement.firstChildElement(QStringLiteral("password")).text();
    m_digest = QByteArray::fromHex(queryElement.firstChildElement(QStringLiteral("digest")).text().toLatin1());
    m_resource = queryElement.firstChildElement(QStringLiteral("resource")).text();
}

void QXmppNonSASLAuthIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_auth);
    if (!m_username.isEmpty())
        writer->writeTextElement(QStringLiteral("username"), m_username);
    if (!m_digest.isEmpty())
        writer->writeTextElement(QStringLiteral("digest"), m_digest.toHex());
    if (!m_password.isEmpty())
        writer->writeTextElement(QStringLiteral("password"), m_password);
    if (!m_resource.isEmpty())
        writer->writeTextElement(QStringLiteral("resource"), m_resource);
    writer->writeEndElement();
}
/// \endcond
