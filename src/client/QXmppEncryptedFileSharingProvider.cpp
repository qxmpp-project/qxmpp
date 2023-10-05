// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEncryptedFileSharingProvider.h"

#include "QXmppFileEncryption.h"
#include "QXmppFileMetadata.h"
#include "QXmppFileSharingManager.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppUtils.h"

#include "QcaInitializer_p.h"
#include <QMimeDatabase>

using namespace QXmpp;
using namespace QXmpp::Private;

constexpr auto ENCRYPTION_DEFAULT_CIPHER = Aes256CbcPkcs7;

///
/// \class QXmppEncryptedFileSharingProvider
///
/// Encrypts or decrypts files on the fly when uploading or downloading.
///
/// \since QXmpp 1.5
///

class QXmppEncryptedFileSharingProviderPrivate
{
public:
    QcaInitializer init;
    QXmppFileSharingManager *manager;
    std::shared_ptr<QXmppFileSharingProvider> uploadBaseProvider;
};

///
/// \brief Create a new QXmppEncryptedFileSharingProvider
///
/// \param manager QXmppFileSharingManager to be used to find other providers for downloading
/// encrypted files.
/// \param uploadBaseProvider Provider to be used for uploading the encrypted files.
///
QXmppEncryptedFileSharingProvider::QXmppEncryptedFileSharingProvider(
    QXmppFileSharingManager *manager,
    std::shared_ptr<QXmppFileSharingProvider> uploadBaseProvider)
    : d(std::make_unique<QXmppEncryptedFileSharingProviderPrivate>())
{
    d->manager = manager;
    d->uploadBaseProvider = std::move(uploadBaseProvider);
}

QXmppEncryptedFileSharingProvider::~QXmppEncryptedFileSharingProvider() = default;

auto QXmppEncryptedFileSharingProvider::downloadFile(const std::any &source,
                                                     std::unique_ptr<QIODevice> target,
                                                     std::function<void(quint64, quint64)> reportProgress,
                                                     std::function<void(DownloadResult)> reportFinished)
    -> std::shared_ptr<Download>
{
    QXmppEncryptedFileSource encryptedSource;
    try {
        encryptedSource = std::any_cast<QXmppEncryptedFileSource>(source);
    } catch (const std::bad_any_cast &) {
        qFatal("QXmppEncryptedFileSharingProvider::downloadFile can only handle QXmppEncryptedFileSource sources");
    }

    auto output = std::make_unique<Encryption::DecryptionDevice>(std::move(target), encryptedSource.cipher(), encryptedSource.key(), encryptedSource.iv());

    // find provider for source of encrypted file
    std::any httpSource = encryptedSource.httpSources().front();
    if (auto provider = d->manager->providerForSource(httpSource)) {
        auto onFinished = [decryptDevice = output.get(), reportFinished = std::move(reportFinished)](DownloadResult result) {
            decryptDevice->finish();
            reportFinished(std::move(result));
        };

        return provider->downloadFile(httpSource, std::move(output), std::move(reportProgress), std::move(onFinished));
    }

    reportFinished(QXmppError { QStringLiteral("No basic file sharing provider available for encrypted file."), {} });
    return {};
}

auto QXmppEncryptedFileSharingProvider::uploadFile(std::unique_ptr<QIODevice> data,
                                                   const QXmppFileMetadata &,
                                                   std::function<void(quint64, quint64)> reportProgress,
                                                   std::function<void(UploadResult)> reportFinished)
    -> std::shared_ptr<Upload>
{
    auto cipher = ENCRYPTION_DEFAULT_CIPHER;
    auto key = Encryption::generateKey(cipher);
    auto iv = Encryption::generateInitializationVector(cipher);

    auto encDevice = std::make_unique<Encryption::EncryptionDevice>(std::move(data), cipher, key, iv);
    auto encryptedSize = encDevice->size();

    QXmppFileMetadata metadata;
    metadata.setFilename(QXmppUtils::generateStanzaHash(10));
    metadata.setMediaType(QMimeDatabase().mimeTypeForName("application/octet-stream"));
    metadata.setSize(encryptedSize);

    // find provider for source of encrypted file
    Q_ASSERT(d->uploadBaseProvider);
    return d->uploadBaseProvider->uploadFile(
        std::move(encDevice),
        metadata,
        std::move(reportProgress),
        [=, reportFinished = std::move(reportFinished)](UploadResult result) {
            auto encryptedResult = visitForward<UploadResult>(std::move(result), [&](std::any httpSourceAny) {
                QXmppEncryptedFileSource encryptedSource;
                encryptedSource.setCipher(ENCRYPTION_DEFAULT_CIPHER);
                encryptedSource.setKey(key);
                encryptedSource.setIv(iv);
                encryptedSource.setHttpSources({ std::any_cast<QXmppHttpFileSource>(std::move(httpSourceAny)) });

                return encryptedSource;
            });
            reportFinished(std::move(encryptedResult));
        });
}
