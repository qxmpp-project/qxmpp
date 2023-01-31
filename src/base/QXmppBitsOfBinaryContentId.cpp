// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"

#include <QMap>
#include <QSharedData>
#include <QString>

#define CONTENTID_URL QStringLiteral("cid:")
#define CONTENTID_URL_LENGTH 4
#define CONTENTID_POSTFIX QStringLiteral("@bob.xmpp.org")
#define CONTENTID_POSTFIX_LENGTH 13
#define CONTENTID_HASH_SEPARATOR QStringLiteral("+")

static const QMap<QCryptographicHash::Algorithm, QString> HASH_ALGORITHMS = {
    { QCryptographicHash::Sha1, QStringLiteral("sha1") },
    { QCryptographicHash::Md4, QStringLiteral("md4") },
    { QCryptographicHash::Md5, QStringLiteral("md5") },
    { QCryptographicHash::Sha224, QStringLiteral("sha-224") },
    { QCryptographicHash::Sha256, QStringLiteral("sha-256") },
    { QCryptographicHash::Sha384, QStringLiteral("sha-384") },
    { QCryptographicHash::Sha512, QStringLiteral("sha-512") },
    { QCryptographicHash::Sha3_224, QStringLiteral("sha3-224") },
    { QCryptographicHash::Sha3_256, QStringLiteral("sha3-256") },
    { QCryptographicHash::Sha3_384, QStringLiteral("sha3-384") },
    { QCryptographicHash::Sha3_512, QStringLiteral("sha3-512") },
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    { QCryptographicHash::Blake2b_256, QStringLiteral("blake2b-256") },
    { QCryptographicHash::Blake2b_512, QStringLiteral("blake2b-512") },
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

///
/// \class QXmppBitsOfBinaryContentId
///
/// QXmppBitsOfBinaryContentId represents a link to or an identifier of
/// \xep{0231, Bits of Binary} data.
///
/// Currently supported hash algorithms:
///  * MD4
///  * MD5
///  * SHA-1
///  * SHA-2 (SHA-224, SHA-256, SHA-384, SHA-512)
///  * SHA-3 (SHA3-224, SHA3-256, SHA3-384, SHA3-512)
///  * BLAKE2 (BLAKE2b256, BLAKE2b512) (requires Qt 6, since QXmpp 1.5)
///
/// \note Security notice: When using the content IDs to cache data between multiple entities it is
/// important to avoid hash collisions. SHA-1 cannot fulfill this requirement. You SHOULD use
/// another more secure hash algorithm if you do this.
///
/// \since QXmpp 1.2
///

///
/// Parses a \c QXmppBitsOfBinaryContentId from a \xep{0231, Bits of Binary}
/// \c cid: URL
///
/// In case parsing failed, the returned \c QXmppBitsOfBinaryContentId is
/// empty.
///
/// \see QXmppBitsOfBinaryContentId::fromContentId
///
QXmppBitsOfBinaryContentId QXmppBitsOfBinaryContentId::fromCidUrl(const QString &input)
{
    if (input.startsWith(CONTENTID_URL)) {
        return fromContentId(input.mid(CONTENTID_URL_LENGTH));
    }

    return {};
}

///
/// Parses a \c QXmppBitsOfBinaryContentId from a \xep{0231, Bits of Binary}
/// content id
///
/// In case parsing failed, the returned \c QXmppBitsOfBinaryContentId is
/// empty.
///
/// \note This does not allow \c cid: URLs to be passed. Use
/// \c QXmppBitsOfBinaryContentId::fromCidUrl for that purpose.
///
/// \see QXmppBitsOfBinaryContentId::fromCidUrl
///
QXmppBitsOfBinaryContentId QXmppBitsOfBinaryContentId::fromContentId(const QString &input)
{
    if (input.startsWith(CONTENTID_URL) || !input.endsWith(CONTENTID_POSTFIX)) {
        return {};
    }

    // remove '@bob.xmpp.org'
    QString hashAndAlgoStr = input.left(input.size() - CONTENTID_POSTFIX_LENGTH);
    // get size of hash algo id
    QStringList algoAndHash = hashAndAlgoStr.split(CONTENTID_HASH_SEPARATOR);
    if (algoAndHash.size() != 2) {
        return {};
    }

    QCryptographicHash::Algorithm algo = HASH_ALGORITHMS.key(algoAndHash.first(), QCryptographicHash::Algorithm(-1));
    if (int(algo) == -1) {
        return {};
    }

    QXmppBitsOfBinaryContentId cid;
    cid.setAlgorithm(algo);
    cid.setHash(QByteArray::fromHex(algoAndHash.last().toUtf8()));

    return cid;
}

///
/// Default contructor
///
QXmppBitsOfBinaryContentId::QXmppBitsOfBinaryContentId()
    : d(new QXmppBitsOfBinaryContentIdPrivate)
{
}

///
/// Returns true, if two \c QXmppBitsOfBinaryContentId equal
///
bool QXmppBitsOfBinaryContentId::operator==(const QXmppBitsOfBinaryContentId &other) const
{
    return d->algorithm == other.algorithm() && d->hash == other.hash();
}

/// Default destructor
QXmppBitsOfBinaryContentId::~QXmppBitsOfBinaryContentId() = default;
/// Default copy-constructor
QXmppBitsOfBinaryContentId::QXmppBitsOfBinaryContentId(const QXmppBitsOfBinaryContentId &cid) = default;
/// Default move-constructor
QXmppBitsOfBinaryContentId::QXmppBitsOfBinaryContentId(QXmppBitsOfBinaryContentId &&cid) = default;
/// Default assignment operator
QXmppBitsOfBinaryContentId &QXmppBitsOfBinaryContentId::operator=(const QXmppBitsOfBinaryContentId &other) = default;
/// Default move-assignment operator
QXmppBitsOfBinaryContentId &QXmppBitsOfBinaryContentId::operator=(QXmppBitsOfBinaryContentId &&other) = default;

///
/// Returns a \xep{0231, Bits of Binary} content id
///
QString QXmppBitsOfBinaryContentId::toContentId() const
{
    if (!isValid()) {
        return {};
    }

    return HASH_ALGORITHMS.value(d->algorithm) +
        CONTENTID_HASH_SEPARATOR +
        d->hash.toHex() +
        CONTENTID_POSTFIX;
}

///
/// Returns a \xep{0231, Bits of Binary} \c cid: URL
///
QString QXmppBitsOfBinaryContentId::toCidUrl() const
{
    if (!isValid()) {
        return {};
    }

    return toContentId().prepend(CONTENTID_URL);
}

///
/// Returns the hash value in binary form
///
QByteArray QXmppBitsOfBinaryContentId::hash() const
{
    return d->hash;
}

///
/// Sets the hash value in binary form
///
void QXmppBitsOfBinaryContentId::setHash(const QByteArray &hash)
{
    d->hash = hash;
}

///
/// Returns the hash algorithm used to calculate the \c hash value
///
/// The default value is \c QCryptographicHash::Sha1.
///
QCryptographicHash::Algorithm QXmppBitsOfBinaryContentId::algorithm() const
{
    return d->algorithm;
}

///
/// Sets the hash algorithm used to calculate the \c hash value
///
/// The default value is \c QCryptographicHash::Sha1.
///
/// \note Only change this, if you know what you do. The XEP allows other
/// hashing algorithms than SHA-1 to be used, but not all clients support this.
///
void QXmppBitsOfBinaryContentId::setAlgorithm(QCryptographicHash::Algorithm algo)
{
    d->algorithm = algo;
}

///
/// Checks whether the content id is valid and can be serialized into a string.
///
/// Also checks the length of the hash.
///
/// \returns True, if the set hashing algorithm is supported, a hash value is
/// set and its length is correct, false otherwise.
///
bool QXmppBitsOfBinaryContentId::isValid() const
{
    return !d->hash.isEmpty() &&
        HASH_ALGORITHMS.contains(d->algorithm) &&
        d->hash.length() == QCryptographicHash::hashLength(d->algorithm);
}

///
/// Checks whether \c input is a Bits of Binary content id or \c cid: URL
///
/// \param input The string to be checked.
/// \param checkIsCidUrl If true, it only accepts \c cid: URLs.
///
/// \returns True, if \c input is valid.
///
bool QXmppBitsOfBinaryContentId::isBitsOfBinaryContentId(const QString &input, bool checkIsCidUrl)
{
    return input.endsWith(CONTENTID_POSTFIX) &&
        input.contains(CONTENTID_HASH_SEPARATOR) &&
        (!checkIsCidUrl || input.startsWith(CONTENTID_URL));
}
