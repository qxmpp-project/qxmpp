/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppBitsOfBinaryContentId.h"

#include <QMap>
#include <QSharedData>
#include <QString>

#define CONTENTID_URL QSL("cid:")
#define CONTENTID_URL_LENGTH 4
#define CONTENTID_POSTFIX QSL("@bob.xmpp.org")
#define CONTENTID_POSTFIX_LENGTH 13
#define CONTENTID_HASH_SEPARATOR QSL("+")

static const QMap<QCryptographicHash::Algorithm, QString> HASH_ALGORITHMS = {
    { QCryptographicHash::Sha1, QSL("sha1") },
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    { QCryptographicHash::Md4, QSL("md4") },
    { QCryptographicHash::Md5, QSL("md5") },
    { QCryptographicHash::Sha224, QSL("sha224") },
    { QCryptographicHash::Sha256, QSL("sha256") },
    { QCryptographicHash::Sha384, QSL("sha384") },
    { QCryptographicHash::Sha512, QSL("sha512") },
    { QCryptographicHash::Sha3_224, QSL("sha3-224") },
    { QCryptographicHash::Sha3_256, QSL("sha3-256") },
    { QCryptographicHash::Sha3_384, QSL("sha3-384") },
    { QCryptographicHash::Sha3_512, QSL("sha3-512") },
#endif
};

class QXmppBitsOfBinaryContentIdPrivate : public QSharedData
{
public:
    QXmppBitsOfBinaryContentIdPrivate();

    QCryptographicHash::Algorithm algorithm;
    QByteArray hash;
};

QXmppBitsOfBinaryContentIdPrivate::QXmppBitsOfBinaryContentIdPrivate()
    : algorithm(QCryptographicHash::Sha1)
{
}

/// Parses a \c QXmppBitsOfBinaryContentId from a \xep{0231}: Bits of Binary
/// \c cid: URL
///
/// In case parsing failed, the returned \c QXmppBitsOfBinaryContentId is
/// empty.
///
/// \see QXmppBitsOfBinaryContentId::fromContentId

QXmppBitsOfBinaryContentId QXmppBitsOfBinaryContentId::fromCidUrl(const QString &input)
{
    if (input.startsWith(CONTENTID_URL))
        return fromContentId(input.mid(CONTENTID_URL_LENGTH));

    return {};
}

/// Parses a \c QXmppBitsOfBinaryContentId from a \xep{0231}: Bits of Binary
/// content id
///
/// In case parsing failed, the returned \c QXmppBitsOfBinaryContentId is
/// empty.
///
/// \note This does not allow \c cid: URLs to be passed. Use
/// \c QXmppBitsOfBinaryContentId::fromCidUrl for that purpose.
///
/// \see QXmppBitsOfBinaryContentId::fromCidUrl

QXmppBitsOfBinaryContentId QXmppBitsOfBinaryContentId::fromContentId(const QString &input)
{
    if (input.startsWith(CONTENTID_URL) || !input.endsWith(CONTENTID_POSTFIX))
        return {};

    // remove '@bob.xmpp.org'
    QString hashAndAlgoStr = input.left(input.size() - CONTENTID_POSTFIX_LENGTH);
    // get size of hash algo id
    QStringList algoAndHash = hashAndAlgoStr.split(CONTENTID_HASH_SEPARATOR);
    if (algoAndHash.size() != 2)
        return {};

    QCryptographicHash::Algorithm algo = HASH_ALGORITHMS.key(algoAndHash.first(), QCryptographicHash::Algorithm(-1));
    if (int(algo) == -1)
        return {};

    QXmppBitsOfBinaryContentId cid;
    cid.setAlgorithm(algo);
    cid.setHash(QByteArray::fromHex(algoAndHash.last().toUtf8()));

    return cid;
}

/// Default contructor

QXmppBitsOfBinaryContentId::QXmppBitsOfBinaryContentId()
    : d(new QXmppBitsOfBinaryContentIdPrivate)
{
}

/// Returns true, if two \c QXmppBitsOfBinaryContentId equal

bool QXmppBitsOfBinaryContentId::operator==(const QXmppBitsOfBinaryContentId &other) const
{
    return d->algorithm == other.algorithm() && d->hash == other.hash();
}

QXmppBitsOfBinaryContentId::~QXmppBitsOfBinaryContentId() = default;

QXmppBitsOfBinaryContentId::QXmppBitsOfBinaryContentId(const QXmppBitsOfBinaryContentId &cid) = default;

QXmppBitsOfBinaryContentId &QXmppBitsOfBinaryContentId::operator=(const QXmppBitsOfBinaryContentId &other) = default;

/// Returns a \xep{0231}: Bits of Binary content id

QString QXmppBitsOfBinaryContentId::toContentId() const
{
    if (!isValid())
        return {};

    return HASH_ALGORITHMS.value(d->algorithm) +
        CONTENTID_HASH_SEPARATOR +
        d->hash.toHex() +
        CONTENTID_POSTFIX;
}

/// Returns a \xep{0231}: Bits of Binary \c cid: URL

QString QXmppBitsOfBinaryContentId::toCidUrl() const
{
    if (!isValid())
        return {};

    return toContentId().prepend(CONTENTID_URL);
}

/// Returns the hash value in binary form

QByteArray QXmppBitsOfBinaryContentId::hash() const
{
    return d->hash;
}

/// Sets the hash value in binary form

void QXmppBitsOfBinaryContentId::setHash(const QByteArray &hash)
{
    d->hash = hash;
}

/// Returns the hash algorithm used to calculate the \c hash value
///
/// The default value is \c QCryptographicHash::Sha1.
///
/// This currently supports MD4, MD5, SHA-1, SHA-2 (SHA224 - SHA512) and SHA-3
/// (SHA3-224 - SHA3-512).

QCryptographicHash::Algorithm QXmppBitsOfBinaryContentId::algorithm() const
{
    return d->algorithm;
}

/// Sets the hash algorithm used to calculate the \c hash value
///
/// The default value is \c QCryptographicHash::Sha1.
///
/// This currently supports MD4, MD5, SHA-1, SHA-2 (SHA224 - SHA512) and SHA-3
/// (SHA3-224 - SHA3-512).
///
/// \note Only change this, if you know what you do. The XEP allows other
/// hashing algorithms than SHA-1 to be used, but not all clients support this.
/// Since in most cases the content id is not security relevant it is not a
/// problem to continue using SHA-1.

void QXmppBitsOfBinaryContentId::setAlgorithm(QCryptographicHash::Algorithm algo)
{
    d->algorithm = algo;
}

/// Checks whether the content id is valid and can be serialized into a string.
///
/// \note Checking the hash length requires QXmpp to be built with Qt 5.12.0 or
/// later.
///
/// \returns True, if the set hashing algorithm is supported, a hash value is
/// set and its length is correct, false otherwise.

bool QXmppBitsOfBinaryContentId::isValid() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return !d->hash.isEmpty() &&
        HASH_ALGORITHMS.contains(d->algorithm) &&
        d->hash.length() == QCryptographicHash::hashLength(d->algorithm);
#else
    return !d->hash.isEmpty() && HASH_ALGORITHMS.contains(d->algorithm);
#endif
}

/// Checks whether \c input is a Bits of Binary content id or \c cid: URL
///
/// \param checkIsCidUrl If true, it only accepts \c cid: URLs.
///
/// \returns True, if \c input is valid.

bool QXmppBitsOfBinaryContentId::isBitsOfBinaryContentId(const QString &input, bool checkIsCidUrl)
{
    return input.endsWith(CONTENTID_POSTFIX) &&
        input.contains(CONTENTID_HASH_SEPARATOR) &&
        (!checkIsCidUrl || input.startsWith(CONTENTID_URL));
}
