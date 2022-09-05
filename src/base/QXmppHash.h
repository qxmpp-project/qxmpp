// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPHASH_H
#define QXMPPHASH_H

#include "QXmppGlobal.h"

#include <QByteArray>

class QDomElement;
class QXmlStreamWriter;

namespace QXmpp {

enum class HashAlgorithm : uint32_t {
    Unknown,
    Md2,
    Md5,
    Shake128,
    Shake256,
    Sha1,
    Sha224,
    Sha256,
    Sha384,
    Sha512,
    Sha3_256,
    Sha3_512,
    Blake2b_256,
    Blake2b_512,
};

}

class QXMPP_EXPORT QXmppHash
{
public:
    QXmppHash();

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    QXmpp::HashAlgorithm algorithm() const;
    void setAlgorithm(QXmpp::HashAlgorithm algorithm);

    QByteArray hash() const;
    void setHash(const QByteArray &data);

private:
    QXmpp::HashAlgorithm m_algorithm = QXmpp::HashAlgorithm::Unknown;
    QByteArray m_hash;
};

class QXMPP_EXPORT QXmppHashUsed
{
public:
    QXmppHashUsed();
    QXmppHashUsed(QXmpp::HashAlgorithm algorithm);

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    QXmpp::HashAlgorithm algorithm() const;
    void setAlgorithm(QXmpp::HashAlgorithm algorithm);

private:
    QXmpp::HashAlgorithm m_algorithm = QXmpp::HashAlgorithm::Unknown;
};

#endif  // QXMPPHASH_H
