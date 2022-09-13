// SPDX-FileCopyrightText: 2022 <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPENCRYPTEDFILESOURCE_H
#define QXMPPENCRYPTEDFILESOURCE_H

#include "QXmppGlobal.h"
#include "QXmppHttpFileSource.h"

#include <QSharedDataPointer>
#include <QVector>
#include <QUrl>

#include "QXmppHash.h"

class QXmppEncryptedFileSourcePrivate;

class QXMPP_EXPORT QXmppEncryptedFileSource
{
public:
    enum Cipher {
        Aes128GcmNopadding,
        Aes256GcmNopadding,
        Aes256CbcPkcs7,
    };

    QXmppEncryptedFileSource();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppEncryptedFileSource);
    
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
    QSharedDataPointer<QXmppEncryptedFileSourcePrivate> d;
};

#endif // QXMPPENCRYPTEDFILESOURCE_H
