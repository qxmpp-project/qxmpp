// SPDX-FileCopyrightText: 2022 <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppEncryptedFileSource_p.h"
#include "QXmppHttpFileSource.h"

#include <optional>

#include <QDomElement>
#include <QXmlStreamWriter>

QString cipherToString(QXmppEncryptedFileSource::Cipher cipher)
{
    switch (cipher) {
    case QXmppEncryptedFileSource::Aes128GcmNopadding:
        return "urn:xmpp:ciphers:aes-128-gcm-nopadding:0";
    case QXmppEncryptedFileSource::Aes256GcmNopadding:
        return "urn:xmpp:ciphers:aes-256-gcm-nopadding:0";
    case QXmppEncryptedFileSource::Aes256CbcPkcs7:
        return "urn:xmpp:ciphers:aes-256-cbc-pkcs7:0";
    }

    Q_UNREACHABLE();
}

std::optional<QXmppEncryptedFileSource::Cipher> cipherFromString(const QString &cipher)
{
    if (cipher == "urn:xmpp:ciphers:aes-128-gcm-nopadding:0") {
        return QXmppEncryptedFileSource::Aes128GcmNopadding;
    } else if (cipher == "urn:xmpp:ciphers:aes-256-gcm-nopadding:0") {
        return QXmppEncryptedFileSource::Aes256GcmNopadding;
    } else if (cipher == "urn:xmpp:ciphers:aes-256-cbc-pkcs7:0") {
        return QXmppEncryptedFileSource::Aes256CbcPkcs7;
    }

    return {};
}

class QXmppEncryptedFileSourcePrivate : public QSharedData
{
public:
    QXmppEncryptedFileSource::Cipher cipher = QXmppEncryptedFileSource::Aes128GcmNopadding;
    QByteArray key;
    QByteArray iv;
    QVector<QXmppHash> hashes;
    QVector<QXmppHttpFileSource> httpSources;
};

QXmppEncryptedFileSource::QXmppEncryptedFileSource()
    : d(new QXmppEncryptedFileSourcePrivate())
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppEncryptedFileSource);

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

const QVector<QXmppHttpFileSource> &QXmppEncryptedFileSource::httpSources() const
{
    return d->httpSources;
}

void QXmppEncryptedFileSource::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    d->httpSources = newHttpSources;
}

bool QXmppEncryptedFileSource::parse(const QDomElement &el)
{
    QString cipher = el.attribute(QStringLiteral("cipher"));
    if (auto parsedCipher = cipherFromString(cipher)) {
        d->cipher = *parsedCipher;
    } else {
        return false;
    }

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
    for (auto childEl = sourcesEl.firstChildElement(QStringLiteral("url-data"));
         !childEl.isNull();
         childEl = childEl.nextSiblingElement(QStringLiteral("url-data"))) {
        QXmppHttpFileSource source;
        source.parse(childEl);
        d->httpSources.push_back(std::move(source));
    }

    return true;
}

void QXmppEncryptedFileSource::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("encrypted"));
    writer->writeDefaultNamespace(ns_esfs);
    writer->writeAttribute(QStringLiteral("cipher"), cipherToString(d->cipher));
    writer->writeTextElement(QStringLiteral("key"), d->key.toBase64());
    writer->writeTextElement(QStringLiteral("iv"), d->iv.toBase64());
    for (const auto &hash : d->hashes) {
        hash.toXml(writer);
    }
    writer->writeStartElement(QStringLiteral("sources"));
    writer->writeDefaultNamespace(ns_sfs);
    for (const auto &source : d->httpSources) {
        source.toXml(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
