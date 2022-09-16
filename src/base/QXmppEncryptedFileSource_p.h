// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPENCRYPTEDFILESOURCE_H
#define QXMPPENCRYPTEDFILESOURCE_H

#include "QXmppGlobal.h"
#include "QXmppHash.h"
#include "QXmppHttpFileSource.h"

#include <QSharedDataPointer>
#include <QUrl>
#include <QVector>

class QXmppEncryptedFileSourcePrivate;

// exported for tests
class QXMPP_EXPORT QXmppEncryptedFileSource
{
public:
    enum Cipher {
        Aes128GcmNopadding,
        Aes256GcmNopadding,
        Aes256CbcPkcs7,
    };

    QXmppEncryptedFileSource();

    Cipher cipher() const;
    void setCipher(Cipher newCipher);

    const QByteArray &key() const;
    void setKey(const QByteArray &newKey);

    const QByteArray &iv() const;
    void setIv(const QByteArray &newIv);

    const QVector<QXmppHash> &hashes() const;
    void setHashes(const QVector<QXmppHash> &newHashes);

    const QVector<QXmppHttpFileSource> &httpSources() const;
    void setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources);

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    Cipher m_cipher = Aes128GcmNopadding;
    QByteArray m_key;
    QByteArray m_iv;
    QVector<QXmppHash> m_hashes;
    QVector<QXmppHttpFileSource> m_httpSources;
};

#endif  // QXMPPENCRYPTEDFILESOURCE_H
