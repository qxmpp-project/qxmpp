// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "OmemoCryptoProvider.h"

#include "QXmppOmemoManager_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QtCrypto>

using namespace QXmpp::Private;

inline QXmppOmemoManagerPrivate *managerPrivate(void *ptr)
{
    return reinterpret_cast<QXmppOmemoManagerPrivate *>(ptr);
}

static int random_func(uint8_t *data, size_t len, void *)
{
    generateRandomBytes(data, len);
    return 0;
}

int hmac_sha256_init_func(void **hmac_context, const uint8_t *key, size_t key_len, void *user_data)
{
    auto *d = managerPrivate(user_data);

    if (!QCA::MessageAuthenticationCode::supportedTypes().contains(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE)) {
        d->warning(u"Message authentication code type '" + PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE + u"' is not supported by this system");
        return -1;
    }

    QCA::SymmetricKey authenticationKey(QByteArray(reinterpret_cast<const char *>(key), key_len));
    *hmac_context = new QCA::MessageAuthenticationCode(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE.toString(), authenticationKey);
    return 0;
}

int hmac_sha256_update_func(void *hmac_context, const uint8_t *data, size_t data_len, void *)
{
    auto *messageAuthenticationCodeGenerator = reinterpret_cast<QCA::MessageAuthenticationCode *>(hmac_context);
    messageAuthenticationCodeGenerator->update(QCA::MemoryRegion(QByteArray(reinterpret_cast<const char *>(data), data_len)));
    return 0;
}

int hmac_sha256_final_func(void *hmac_context, signal_buffer **output, void *user_data)
{
    auto *d = managerPrivate(user_data);
    auto *messageAuthenticationCodeGenerator = reinterpret_cast<QCA::MessageAuthenticationCode *>(hmac_context);

    auto messageAuthenticationCode = messageAuthenticationCodeGenerator->final();
    if (!(*output = signal_buffer_create(reinterpret_cast<const uint8_t *>(messageAuthenticationCode.constData()), messageAuthenticationCode.size()))) {
        d->warning(u"Message authentication code could not be loaded"_s);
        return -1;
    }

    return 0;
}

void hmac_sha256_cleanup_func(void *hmac_context, void *)
{
    auto *messageAuthenticationCodeGenerator = reinterpret_cast<QCA::MessageAuthenticationCode *>(hmac_context);
    delete messageAuthenticationCodeGenerator;
}

int sha512_digest_init_func(void **digest_context, void *)
{
    *digest_context = new QCryptographicHash(QCryptographicHash::Sha512);
    return 0;
}

int sha512_digest_update_func(void *digest_context, const uint8_t *data, size_t data_len, void *)
{
    auto *hashGenerator = reinterpret_cast<QCryptographicHash *>(digest_context);
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    hashGenerator->addData(QByteArrayView(reinterpret_cast<const char *>(data), data_len));
#else
    hashGenerator->addData(reinterpret_cast<const char *>(data), data_len);
#endif
    return 0;
}

int sha512_digest_final_func(void *digest_context, signal_buffer **output, void *user_data)
{
    auto *d = managerPrivate(user_data);
    auto *hashGenerator = reinterpret_cast<QCryptographicHash *>(digest_context);

    auto hash = hashGenerator->result();
    if (!(*output = signal_buffer_create(reinterpret_cast<const uint8_t *>(hash.constData()), hash.size()))) {
        d->warning(u"Hash could not be loaded"_s);
        return -1;
    }

    return 0;
}

void sha512_digest_cleanup_func(void *digest_context, void *)
{
    auto *hashGenerator = reinterpret_cast<QCryptographicHash *>(digest_context);
    delete hashGenerator;
}

int encrypt_func(signal_buffer **output,
                 int cipher,
                 const uint8_t *key, size_t key_len,
                 const uint8_t *iv, size_t iv_len,
                 const uint8_t *plaintext, size_t plaintext_len,
                 void *user_data)
{
    auto *d = managerPrivate(user_data);

    QString cipherName;

    switch (key_len) {
    case 128 / 8:
        cipherName = u"aes128"_s;
        break;
    case 192 / 8:
        cipherName = u"aes192"_s;
        break;
    case 256 / 8:
        cipherName = u"aes256"_s;
        break;
    default:
        return -1;
    }

    QCA::Cipher::Mode mode;
    QCA::Cipher::Padding padding;

    switch (cipher) {
    case SG_CIPHER_AES_CTR_NOPADDING:
        mode = QCA::Cipher::CTR;
        padding = QCA::Cipher::NoPadding;
        break;
    case SG_CIPHER_AES_CBC_PKCS5:
        mode = QCA::Cipher::CBC;
        padding = QCA::Cipher::PKCS7;
        break;
    default:
        return -2;
    }

    const auto encryptionKey = QCA::SymmetricKey(QByteArray(reinterpret_cast<const char *>(key), key_len));
    const auto initializationVector = QCA::InitializationVector(QByteArray(reinterpret_cast<const char *>(iv), iv_len));
    QCA::Cipher encryptionCipher(cipherName, mode, padding, QCA::Encode, encryptionKey, initializationVector);

    auto encryptedData = encryptionCipher.process(QCA::MemoryRegion(QByteArray(reinterpret_cast<const char *>(plaintext), plaintext_len)));

    if (encryptedData.isEmpty()) {
        return -3;
    }

    if (!(*output = signal_buffer_create(reinterpret_cast<const uint8_t *>(encryptedData.constData()), encryptedData.size()))) {
        d->warning(u"Encrypted data could not be loaded"_s);
        return -4;
    }

    return 0;
}

int decrypt_func(signal_buffer **output,
                 int cipher,
                 const uint8_t *key, size_t key_len,
                 const uint8_t *iv, size_t iv_len,
                 const uint8_t *ciphertext, size_t ciphertext_len,
                 void *user_data)
{
    auto *d = managerPrivate(user_data);

    QString cipherName;

    switch (key_len) {
    case 128 / 8:
        cipherName = u"aes128"_s;
        break;
    case 192 / 8:
        cipherName = u"aes192"_s;
        break;
    case 256 / 8:
        cipherName = u"aes256"_s;
        break;
    default:
        return -1;
    }

    QCA::Cipher::Mode mode;
    QCA::Cipher::Padding padding;

    switch (cipher) {
    case SG_CIPHER_AES_CTR_NOPADDING:
        mode = QCA::Cipher::CTR;
        padding = QCA::Cipher::NoPadding;
        break;
    case SG_CIPHER_AES_CBC_PKCS5:
        mode = QCA::Cipher::CBC;
        padding = QCA::Cipher::PKCS7;
        break;
    default:
        return -2;
    }

    const auto encryptionKey = QCA::SymmetricKey(QByteArray(reinterpret_cast<const char *>(key), key_len));
    const auto initializationVector = QCA::InitializationVector(QByteArray(reinterpret_cast<const char *>(iv), iv_len));
    QCA::Cipher decryptionCipher(cipherName, mode, padding, QCA::Decode, encryptionKey, initializationVector);

    auto decryptedData = decryptionCipher.process(QCA::MemoryRegion(QByteArray(reinterpret_cast<const char *>(ciphertext), ciphertext_len)));

    if (decryptedData.isEmpty()) {
        return -3;
    }

    if (!(*output = signal_buffer_create(reinterpret_cast<const uint8_t *>(decryptedData.constData()), decryptedData.size()))) {
        d->warning(u"Decrypted data could not be loaded"_s);
        return -4;
    }

    return 0;
}

namespace QXmpp::Omemo::Private {

signal_crypto_provider createOmemoCryptoProvider(QXmppOmemoManagerPrivate *d)
{
    return {
        random_func,
        hmac_sha256_init_func,
        hmac_sha256_update_func,
        hmac_sha256_final_func,
        hmac_sha256_cleanup_func,
        sha512_digest_init_func,
        sha512_digest_update_func,
        sha512_digest_final_func,
        sha512_digest_cleanup_func,
        encrypt_func,
        decrypt_func,
        d,
    };
}

}  // namespace QXmpp::Omemo::Private
