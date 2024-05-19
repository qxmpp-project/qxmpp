// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHash.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \enum QXmpp::HashAlgorithm
///
/// One of the hash algorithms specified by the IANA registry or \xep{0300, Use
/// of Cryptographic Hash Functions in XMPP}.
///
/// \since QXmpp 1.5
///

static QString algorithmToString(HashAlgorithm algorithm)
{
    switch (algorithm) {
    case HashAlgorithm::Unknown:
        return {};
    case HashAlgorithm::Md2:
        return u"md2"_s;
    case HashAlgorithm::Md5:
        return u"md5"_s;
    case HashAlgorithm::Shake128:
        return u"shake128"_s;
    case HashAlgorithm::Shake256:
        return u"shake256"_s;
    case HashAlgorithm::Sha1:
        return u"sha-1"_s;
    case HashAlgorithm::Sha224:
        return u"sha-224"_s;
    case HashAlgorithm::Sha256:
        return u"sha-256"_s;
    case HashAlgorithm::Sha384:
        return u"sha-384"_s;
    case HashAlgorithm::Sha512:
        return u"sha-512"_s;
    case HashAlgorithm::Sha3_256:
        return u"sha3-256"_s;
    case HashAlgorithm::Sha3_512:
        return u"sha3-512"_s;
    case HashAlgorithm::Blake2b_256:
        return u"blake2b-256"_s;
    case HashAlgorithm::Blake2b_512:
        return u"blake2b-512"_s;
    }
    Q_UNREACHABLE();
}

static HashAlgorithm hashAlgorithmFromString(const QString &str)
{
    if (str == u"md2") {
        return HashAlgorithm::Md2;
    }
    if (str == u"md5") {
        return HashAlgorithm::Md5;
    }
    if (str == u"shake128") {
        return HashAlgorithm::Shake128;
    }
    if (str == u"shake256") {
        return HashAlgorithm::Shake256;
    }
    if (str == u"sha-1") {
        return HashAlgorithm::Sha1;
    }
    if (str == u"sha-224") {
        return HashAlgorithm::Sha224;
    }
    if (str == u"sha-256") {
        return HashAlgorithm::Sha256;
    }
    if (str == u"sha-384") {
        return HashAlgorithm::Sha384;
    }
    if (str == u"sha-512") {
        return HashAlgorithm::Sha512;
    }
    if (str == u"sha3-256") {
        return HashAlgorithm::Sha3_256;
    }
    if (str == u"sha3-512") {
        return HashAlgorithm::Sha3_512;
    }
    if (str == u"blake2b-256") {
        return HashAlgorithm::Blake2b_256;
    }
    if (str == u"blake2b-512") {
        return HashAlgorithm::Blake2b_512;
    }
    return HashAlgorithm::Unknown;
}

///
/// \class QXmppHash
///
/// Contains a hash value and its algorithm.
///
/// \since QXmpp 1.5
///

QXmppHash::QXmppHash() = default;

/// \cond
bool QXmppHash::parse(const QDomElement &el)
{
    if (el.tagName() == u"hash" && el.namespaceURI() == ns_hashes) {
        m_algorithm = hashAlgorithmFromString(el.attribute(u"algo"_s));
        if (auto hashResult = QByteArray::fromBase64Encoding(el.text().toUtf8())) {
            m_hash = std::move(*hashResult);
        } else {
            return false;
        }
        return true;
    }
    return false;
}

void QXmppHash::toXml(QXmlStreamWriter *writer) const
{
    writer->writeDefaultNamespace(toString65(ns_hashes));
    writer->writeStartElement(QSL65("hash"));
    writer->writeAttribute(QSL65("algo"), algorithmToString(m_algorithm));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    writer->writeCharacters(m_hash.toBase64());
#else
    writer->writeCharacters(QString::fromUtf8(m_hash.toBase64()));
#endif
    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppHashUsed
///
/// Annotates the used hashing algorithm.
///
/// \since QXmpp 1.5
///

QXmppHashUsed::QXmppHashUsed() = default;

///
/// Creates an object that tells other XMPP entities to use this hash algorithm.
///
QXmppHashUsed::QXmppHashUsed(QXmpp::HashAlgorithm algorithm)
    : m_algorithm(algorithm)
{
}

/// \cond
bool QXmppHashUsed::parse(const QDomElement &el)
{
    if (el.tagName() == u"hash-used" && el.namespaceURI() == ns_hashes) {
        m_algorithm = hashAlgorithmFromString(el.attribute(u"algo"_s));
    }
    return false;
}

void QXmppHashUsed::toXml(QXmlStreamWriter *writer) const
{
    writer->writeDefaultNamespace(toString65(ns_hashes));
    writer->writeStartElement(QSL65("hash-used"));
    writer->writeAttribute(QSL65("algo"), algorithmToString(m_algorithm));
    writer->writeEndElement();
}
/// \endcond

///
/// Returns the algorithm used to create the hash.
///
HashAlgorithm QXmppHash::algorithm() const
{
    return m_algorithm;
}

///
/// Sets the algorithm that was used to create the hashed data
///
void QXmppHash::setAlgorithm(QXmpp::HashAlgorithm algorithm)
{
    m_algorithm = algorithm;
}

///
/// Returns the binary data of the hash.
///
QByteArray QXmppHash::hash() const
{
    return m_hash;
}

///
/// Sets the hashed data.
///
void QXmppHash::setHash(const QByteArray &data)
{
    m_hash = data;
}

///
/// Returns the algorithm that is supposed to be used for hashing.
///
HashAlgorithm QXmppHashUsed::algorithm() const
{
    return m_algorithm;
}

///
/// Sets the algorithm that was used to create the hashed data
///
void QXmppHashUsed::setAlgorithm(QXmpp::HashAlgorithm algorithm)
{
    m_algorithm = algorithm;
}
