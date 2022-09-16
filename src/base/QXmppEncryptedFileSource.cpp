// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppEncryptedFileSource_p.h"
#include "QXmppHttpFileSource.h"

#include <optional>

#include <QDomElement>
#include <QXmlStreamWriter>

/// \cond
static QString cipherToString(QXmppEncryptedFileSource::Cipher cipher)
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

static std::optional<QXmppEncryptedFileSource::Cipher> cipherFromString(const QString &cipher)
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

QXmppEncryptedFileSource::QXmppEncryptedFileSource() = default;

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

const QVector<QXmppHttpFileSource> &QXmppEncryptedFileSource::httpSources() const
{
    return m_httpSources;
}

void QXmppEncryptedFileSource::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    m_httpSources = newHttpSources;
}

bool QXmppEncryptedFileSource::parse(const QDomElement &el)
{
    QString cipher = el.attribute(QStringLiteral("cipher"));
    if (auto parsedCipher = cipherFromString(cipher)) {
        m_cipher = *parsedCipher;
    } else {
        return false;
    }

    auto keyEl = el.firstChildElement(QStringLiteral("key"));
    if (keyEl.isNull()) {
        return false;
    }
    m_key = QByteArray::fromBase64(keyEl.text().toUtf8());

    auto ivEl = el.firstChildElement(QStringLiteral("iv"));
    if (ivEl.isNull()) {
        return false;
    }
    m_iv = QByteArray::fromBase64(ivEl.text().toUtf8());

    for (auto childEl = el.firstChildElement(QStringLiteral("hash"));
         !childEl.isNull();
         childEl = childEl.nextSiblingElement(QStringLiteral("hash"))) {
        QXmppHash hash;
        if (!hash.parse(childEl)) {
            return false;
        }
        m_hashes.push_back(std::move(hash));
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
        m_httpSources.push_back(std::move(source));
    }

    return true;
}

void QXmppEncryptedFileSource::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("encrypted"));
    writer->writeDefaultNamespace(ns_esfs);
    writer->writeAttribute(QStringLiteral("cipher"), cipherToString(m_cipher));
    writer->writeTextElement(QStringLiteral("key"), m_key.toBase64());
    writer->writeTextElement(QStringLiteral("iv"), m_iv.toBase64());
    for (const auto &hash : m_hashes) {
        hash.toXml(writer);
    }
    writer->writeStartElement(QStringLiteral("sources"));
    writer->writeDefaultNamespace(ns_sfs);
    for (const auto &source : m_httpSources) {
        source.toXml(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
