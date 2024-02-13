// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEncryptedFileSource.h"

#include "QXmppConstants_p.h"
#include "QXmppHttpFileSource.h"
#include "QXmppUtils_p.h"

#include <optional>

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp;
using namespace QXmpp::Private;

/// \cond
class QXmppEncryptedFileSourcePrivate : public QSharedData
{
public:
    Cipher cipher = Aes128GcmNoPad;
    QByteArray key;
    QByteArray iv;
    QVector<QXmppHash> hashes;
    QVector<QXmppHttpFileSource> httpSources;
};

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppEncryptedFileSource)

static QString cipherToString(Cipher cipher)
{
    switch (cipher) {
    case Aes128GcmNoPad:
        return "urn:xmpp:ciphers:aes-128-gcm-nopadding:0";
    case Aes256GcmNoPad:
        return "urn:xmpp:ciphers:aes-256-gcm-nopadding:0";
    case Aes256CbcPkcs7:
        return "urn:xmpp:ciphers:aes-256-cbc-pkcs7:0";
    }
    Q_UNREACHABLE();
}

static std::optional<Cipher> cipherFromString(const QString &cipher)
{
    if (cipher == "urn:xmpp:ciphers:aes-128-gcm-nopadding:0") {
        return Aes128GcmNoPad;
    } else if (cipher == "urn:xmpp:ciphers:aes-256-gcm-nopadding:0") {
        return Aes256GcmNoPad;
    } else if (cipher == "urn:xmpp:ciphers:aes-256-cbc-pkcs7:0") {
        return Aes256CbcPkcs7;
    }
    return {};
}
/// \endcond

///
/// \class QXmppEncryptedFileSource
///
/// \brief Represents an encrypted file source for file sharing.
///
/// \since QXmpp 1.5
///

QXmppEncryptedFileSource::QXmppEncryptedFileSource()
    : d(new QXmppEncryptedFileSourcePrivate())
{
}

/// Returns the cipher that was used to encrypt the data in this file source
Cipher QXmppEncryptedFileSource::cipher() const
{
    return d->cipher;
}

/// Sets the cipher that was used to encrypt the data in this file source
void QXmppEncryptedFileSource::setCipher(Cipher newCipher)
{
    d->cipher = newCipher;
}

/// Returns the key that can be used to decrypt the data in this file source
const QByteArray &QXmppEncryptedFileSource::key() const
{
    return d->key;
}

/// Sets the key that was used to encrypt the data in this file source
void QXmppEncryptedFileSource::setKey(const QByteArray &newKey)
{
    d->key = newKey;
}

/// Returns the Initialization vector that can be used to decrypt the data in this file source
const QByteArray &QXmppEncryptedFileSource::iv() const
{
    return d->iv;
}

/// Sets the initialization vector that was used to encrypt the data in this file source
void QXmppEncryptedFileSource::setIv(const QByteArray &newIv)
{
    d->iv = newIv;
}

/// Returns the hashes of the file contained in this file source
const QVector<QXmppHash> &QXmppEncryptedFileSource::hashes() const
{
    return d->hashes;
}

/// Sets the hashes of the file contained in this file source
void QXmppEncryptedFileSource::setHashes(const QVector<QXmppHash> &newHashes)
{
    d->hashes = newHashes;
}

/// Returns the http sources that can be used to retrieve the encrypted data
const QVector<QXmppHttpFileSource> &QXmppEncryptedFileSource::httpSources() const
{
    return d->httpSources;
}

/// Sets the http sources containing the encrypted data
void QXmppEncryptedFileSource::setHttpSources(const QVector<QXmppHttpFileSource> &newHttpSources)
{
    d->httpSources = newHttpSources;
}

/// \cond
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

    for (const auto &childEl : iterChildElements(el, u"hash", ns_hashes)) {
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
    for (const auto &childEl : iterChildElements(sourcesEl, u"url-data", ns_url_data)) {
        QXmppHttpFileSource source;
        source.parse(childEl);
        d->httpSources.push_back(std::move(source));
    }

    return true;
}

void QXmppEncryptedFileSource::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("encrypted"));
    writer->writeDefaultNamespace(toString65(ns_esfs));
    writer->writeAttribute(QStringLiteral("cipher"), cipherToString(d->cipher));
    writer->writeTextElement(QStringLiteral("key"), d->key.toBase64());
    writer->writeTextElement(QStringLiteral("iv"), d->iv.toBase64());
    for (const auto &hash : d->hashes) {
        hash.toXml(writer);
    }
    writer->writeStartElement(QStringLiteral("sources"));
    writer->writeDefaultNamespace(toString65(ns_sfs));
    for (const auto &source : d->httpSources) {
        source.toXml(writer);
    }
    writer->writeEndElement();
    writer->writeEndElement();
}
/// \endcond
