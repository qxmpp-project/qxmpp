#ifndef QXMPPENCRYPTEDFILESOURCE_H
#define QXMPPENCRYPTEDFILESOURCE_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QXmppHash;
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
    
    Cipher cipher() const;
    void setCipher(Cipher newCipher);
    
    const QByteArray &key() const;
    void setKey(const QByteArray &newKey);
    
    const QByteArray &iv() const;
    void setIv(const QByteArray &newIv);
    
    const QVector<QXmppHash> &hashes() const;
    void setHashes(const QVector<QXmppHash> &newHashes);
    
    const QVector<QUrl> &httpSources() const;
    void setHttpSources(const QVector<QUrl> &newHttpSources);
    
private:
    QSharedDataPointer<QXmppEncryptedFileSourcePrivate> d;
    Cipher m_cipher;
    QByteArray m_key;
    QByteArray m_iv;
    QVector<QXmppHash> m_hashes;
    QVector<QUrl> m_httpSources;
};

#endif // QXMPPENCRYPTEDFILESOURCE_H
