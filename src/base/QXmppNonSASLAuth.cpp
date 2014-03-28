/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QCryptographicHash>
#include <QDomElement>
#include <QXmlStreamWriter>

#include "QXmppConstants.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppUtils.h"

QXmppNonSASLAuthIq::QXmppNonSASLAuthIq()
    : QXmppIq(QXmppIq::Set)
{
}

QString QXmppNonSASLAuthIq::username() const
{
    return m_username;
}

void QXmppNonSASLAuthIq::setUsername( const QString &username )
{
    m_username = username;
}

QByteArray QXmppNonSASLAuthIq::digest() const
{
    return m_digest;
}

void QXmppNonSASLAuthIq::setDigest(const QString &streamId, const QString &password)
{
    m_digest = QCryptographicHash::hash(streamId.toUtf8() + password.toUtf8(), QCryptographicHash::Sha1);
}

QString QXmppNonSASLAuthIq::password() const
{
    return m_password;
}

void QXmppNonSASLAuthIq::setPassword( const QString &password )
{
    m_password = password;
}

QString QXmppNonSASLAuthIq::resource() const
{
    return m_resource;
}

void QXmppNonSASLAuthIq::setResource(const QString &resource)
{
    m_resource = resource;
}

/// \cond
bool QXmppNonSASLAuthIq::isNonSASLAuthIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return queryElement.namespaceURI() == ns_auth;
}

void QXmppNonSASLAuthIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    m_username = queryElement.firstChildElement("username").text();
    m_password = queryElement.firstChildElement("password").text();
    m_digest = QByteArray::fromHex(queryElement.firstChildElement("digest").text().toLatin1());
    m_resource = queryElement.firstChildElement("resource").text();
}

void QXmppNonSASLAuthIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_auth);
    if (!m_username.isEmpty())
        writer->writeTextElement("username", m_username);
    if (!m_digest.isEmpty())
        writer->writeTextElement("digest", m_digest.toHex());
    if (!m_password.isEmpty())
        writer->writeTextElement("password", m_password);
    if (!m_resource.isEmpty())
        writer->writeTextElement("resource", m_resource);
    writer->writeEndElement();
}
/// \endcond
