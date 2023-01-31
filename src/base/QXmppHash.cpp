// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHash.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp;

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
        return QStringLiteral("md2");
    case HashAlgorithm::Md5:
        return QStringLiteral("md5");
    case HashAlgorithm::Shake128:
        return QStringLiteral("shake128");
    case HashAlgorithm::Shake256:
        return QStringLiteral("shake256");
    case HashAlgorithm::Sha1:
        return QStringLiteral("sha-1");
    case HashAlgorithm::Sha224:
        return QStringLiteral("sha-224");
    case HashAlgorithm::Sha256:
        return QStringLiteral("sha-256");
    case HashAlgorithm::Sha384:
        return QStringLiteral("sha-384");
    case HashAlgorithm::Sha512:
        return QStringLiteral("sha-512");
    case HashAlgorithm::Sha3_256:
        return QStringLiteral("sha3-256");
    case HashAlgorithm::Sha3_512:
        return QStringLiteral("sha3-512");
    case HashAlgorithm::Blake2b_256:
        return QStringLiteral("blake2b-256");
    case HashAlgorithm::Blake2b_512:
        return QStringLiteral("blake2b-512");
    }
    Q_UNREACHABLE();
}

static HashAlgorithm hashAlgorithmFromString(const QString &str)
{
    if (str == "md2") {
        return HashAlgorithm::Md2;
    }
    if (str == "md5") {
        return HashAlgorithm::Md5;
    }
    if (str == "shake128") {
        return HashAlgorithm::Shake128;
    }
    if (str == "shake256") {
        return HashAlgorithm::Shake256;
    }
    if (str == "sha-1") {
        return HashAlgorithm::Sha1;
    }
    if (str == "sha-224") {
        return HashAlgorithm::Sha224;
    }
    if (str == "sha-256") {
        return HashAlgorithm::Sha256;
    }
    if (str == "sha-384") {
        return HashAlgorithm::Sha384;
    }
    if (str == "sha-512") {
        return HashAlgorithm::Sha512;
    }
    if (str == "sha3-256") {
        return HashAlgorithm::Sha3_256;
    }
    if (str == "sha3-512") {
        return HashAlgorithm::Sha3_512;
    }
    if (str == "blake2b-256") {
        return HashAlgorithm::Blake2b_256;
    }
    if (str == "blake2b-512") {
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
    if (el.tagName() == "hash" && el.namespaceURI() == ns_hashes) {
        m_algorithm = hashAlgorithmFromString(el.attribute("algo"));
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
    writer->writeDefaultNamespace(ns_hashes);
    writer->writeStartElement("hash");
    writer->writeAttribute("algo", algorithmToString(m_algorithm));
    writer->writeCharacters(m_hash.toBase64());
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
    if (el.tagName() == "hash-used" && el.namespaceURI() == ns_hashes) {
        m_algorithm = hashAlgorithmFromString(el.attribute("algo"));
    }
    return false;
}

void QXmppHashUsed::toXml(QXmlStreamWriter *writer) const
{
    writer->writeDefaultNamespace(ns_hashes);
    writer->writeStartElement("hash-used");
    writer->writeAttribute("algo", algorithmToString(m_algorithm));
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
