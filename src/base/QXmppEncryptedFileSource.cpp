#include "QXmppEncryptedFileSource.h"

#include <QDomElement>

class QXmppEncryptedFileSourcePrivate : public QSharedData {
public:
    QXmppEncryptedFileSource::Cipher cipher = QXmppEncryptedFileSource::Aes128GcmNopadding;
    QByteArray key;
    QByteArray iv;
    QVector<QXmppHash> hashes;
    QVector<QUrl> httpSources;
};

QXmppEncryptedFileSource::QXmppEncryptedFileSource()
    : d(new QXmppEncryptedFileSourcePrivate())
{
}

QXmppEncryptedFileSource::Cipher QXmppEncryptedFileSource::cipher() const
{
    return d->cipher;
}

void QXmppEncryptedFileSource::setCipher(Cipher newCipher)
{
    d->cipher = newCipher;
}

const QByteArray &QXmppEncryptedFileSource::key() const
{
    return d->key;
}

void QXmppEncryptedFileSource::setKey(const QByteArray &newKey)
{
    d->key = newKey;
}

const QByteArray &QXmppEncryptedFileSource::iv() const
{
    return d->iv;
}

void QXmppEncryptedFileSource::setIv(const QByteArray &newIv)
{
    d->iv = newIv;
}

const QVector<QXmppHash> &QXmppEncryptedFileSource::hashes() const
{
    return d->hashes;
}

void QXmppEncryptedFileSource::setHashes(const QVector<QXmppHash> &newHashes)
{
    d->hashes = newHashes;
}

const QVector<QUrl> &QXmppEncryptedFileSource::httpSources() const
{
    return d->httpSources;
}

void QXmppEncryptedFileSource::setHttpSources(const QVector<QUrl> &newHttpSources)
{
    d->httpSources = newHttpSources;
}

bool QXmppEncryptedFileSource::parse(const QDomElement &el)
{
    auto keyEl = el.firstChildElement(QStringLiteral("key"));
    if (keyEl.isNull()) {
        return false;
    }
    d->key = QByteArray::fromBase64(keyEl.text().toUtf8());

    auto ivEl = el.firstChildElement(QStringLiteral("iv"));
    if (ivEl.isNull()) {
        return false;
    }
    d->iv = QByteArray::fromBase64(ivEl.text().toUtf8());

    for (auto childEl = el.firstChildElement(QStringLiteral("hash"));
         !childEl.isNull();
         childEl = childEl.nextSiblingElement(QStringLiteral("hash"))) {
        QXmppHash hash;
        if (!hash.parse(childEl)) {
            return false;
        }
        d->hashes.push_back(std::move(hash));
    }

    auto sourcesEl = el.firstChildElement(QStringLiteral("sources"));
    if (sourcesEl.isNull()) {
        return false;
    }
    for (auto childEl = el.firstChildElement(QStringLiteral("url-data"));
         !childEl.isNull();
         childEl = childEl.nextSiblingElement(QStringLiteral("url-data"))) {
        d->httpSources.push_back(QUrl(childEl.text()));
    }

    return true;
}

void QXmppEncryptedFileSource::toXml(QXmlStreamWriter *writer) const
{

}
