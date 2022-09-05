#include "QXmppEncryptedFileSource.h"

QXmppEncryptedFileSource::QXmppEncryptedFileSource()
{
    
}

QXmppEncryptedFileSource::Cipher QXmppEncryptedFileSource::cipher() const
{
    return m_cipher;
}

void QXmppEncryptedFileSource::setCipher(Cipher newCipher)
{
    m_cipher = newCipher;
}

const QByteArray &QXmppEncryptedFileSource::key() const
{
    return m_key;
}

void QXmppEncryptedFileSource::setKey(const QByteArray &newKey)
{
    m_key = newKey;
}

const QByteArray &QXmppEncryptedFileSource::iv() const
{
    return m_iv;
}

void QXmppEncryptedFileSource::setIv(const QByteArray &newIv)
{
    m_iv = newIv;
}

const QVector<QXmppHash> &QXmppEncryptedFileSource::hashes() const
{
    return m_hashes;
}

void QXmppEncryptedFileSource::setHashes(const QVector<QXmppHash> &newHashes)
{
    m_hashes = newHashes;
}

const QVector<QUrl> &QXmppEncryptedFileSource::httpSources() const
{
    return m_httpSources;
}

void QXmppEncryptedFileSource::setHttpSources(const QVector<QUrl> &newHttpSources)
{
    m_httpSources = newHttpSources;
}
