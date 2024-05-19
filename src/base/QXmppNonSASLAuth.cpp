// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppNonSASLAuth.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QCryptographicHash>
#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

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
    m_digest = QCryptographicHash::hash(QByteArray(streamId.toUtf8() + password.toUtf8()), QCryptographicHash::Sha1);
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
    return isIqType(element, u"query", ns_auth);
}

void QXmppNonSASLAuthIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    m_username = queryElement.firstChildElement(u"username"_s).text();
    m_password = queryElement.firstChildElement(u"password"_s).text();
    m_digest = QByteArray::fromHex(queryElement.firstChildElement(u"digest"_s).text().toLatin1());
    m_resource = queryElement.firstChildElement(u"resource"_s).text();
}

void QXmppNonSASLAuthIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_auth));
    if (!m_username.isEmpty()) {
        writer->writeTextElement(QSL65("username"), m_username);
    }
    if (!m_digest.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        writer->writeTextElement("digest", m_digest.toHex());
#else
        writer->writeTextElement(u"digest"_s, QString::fromUtf8(m_digest.toHex()));
#endif
    }
    if (!m_password.isEmpty()) {
        writer->writeTextElement(QSL65("password"), m_password);
    }
    if (!m_resource.isEmpty()) {
        writer->writeTextElement(QSL65("resource"), m_resource);
    }
    writer->writeEndElement();
}
/// \endcond
