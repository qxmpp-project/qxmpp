// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEncryptedHttpFileSharingProvider.h"

#include "QXmppClient.h"
#include "QXmppFileEncryption.h"
#include "QXmppFileMetadata.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHttpUploadManager.h"
#include "QXmppUtils.h"

#include "QcaInitializer_p.h"
#include <QMimeDatabase>

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \class QXmppEncryptedHttpFileSharingProvider
///
/// Support for storing files encrypted on an HTTP server
///
/// \since QXmpp 1.5
///

class QXmppEncryptedHttpFileSharingProviderPrivate
{
public:
    QXmpp::Private::QcaInitializer init;
    QXmppHttpFileSharingProvider *httpProvider;
};

///
/// \brief Create a new QXmppEncryptedHttpFileSharingProvider
/// \param client
/// \param netManager QNetworkAccessManager that can be reused all over your application.
///
QXmppEncryptedHttpFileSharingProvider::QXmppEncryptedHttpFileSharingProvider(QXmppClient *client, QNetworkAccessManager *netManager)
    : d(std::make_unique<QXmppEncryptedHttpFileSharingProviderPrivate>())
{
    d->httpProvider = new QXmppHttpFileSharingProvider(client, netManager);
}

QXmppEncryptedHttpFileSharingProvider::~QXmppEncryptedHttpFileSharingProvider() = default;

auto QXmppEncryptedHttpFileSharingProvider::downloadFile(const std::any &source,
                                                         std::unique_ptr<QIODevice> target,
                                                         std::function<void(quint64, quint64)> reportProgress,
                                                         std::function<void(DownloadResult)> reportFinished)
    -> std::shared_ptr<Download>
{
    QXmppEncryptedFileSource encryptedSource;
    try {
        encryptedSource = std::any_cast<QXmppEncryptedFileSource>(source);
    } catch (const std::bad_any_cast &) {
        qFatal("QXmppEncryptedHttpFileSharingProvider::downloadFile can only handle QXmppEncryptedFileSource sources");
    }

    auto httpSource = encryptedSource.httpSources().front();
    auto output = std::make_unique<Encryption::DecryptionDevice>(std::move(target), encryptedSource.cipher(), encryptedSource.iv(), encryptedSource.key());
    return d->httpProvider->downloadFile(httpSource, std::move(output), std::move(reportProgress), std::move(reportFinished));
}

auto QXmppEncryptedHttpFileSharingProvider::uploadFile(std::unique_ptr<QIODevice> data,
                                                       const QXmppFileMetadata &,
                                                       std::function<void(quint64, quint64)> reportProgress,
                                                       std::function<void(UploadResult)> reportFinished)
    -> std::shared_ptr<Upload>
{
    auto cipher = Aes256CbcPkcs7;
    auto key = Encryption::generateKey(cipher);
    auto iv = Encryption::generateInitializationVector(cipher);

    auto encDevice = std::make_unique<Encryption::EncryptionDevice>(std::move(data), cipher, key, iv);
    auto encryptedSize = encDevice->size();

    QXmppFileMetadata metadata;
    metadata.setFilename(QXmppUtils::generateStanzaHash(10));
    metadata.setMediaType(QMimeDatabase().mimeTypeForName("application/octet-stream"));
    metadata.setSize(encryptedSize);

    return d->httpProvider->uploadFile(
        std::move(encDevice),
        metadata,
        std::move(reportProgress),
        [=, reportFinished = std::move(reportFinished)](UploadResult result) {
            auto encryptedResult = visitForward<UploadResult>(std::move(result), [&](std::any httpSourceAny) {
                QXmppEncryptedFileSource encryptedSource;
                encryptedSource.setKey(key);
                encryptedSource.setIv(iv);
                encryptedSource.setHttpSources({ std::any_cast<QXmppHttpFileSource>(std::move(httpSourceAny)) });

                return encryptedSource;
            });
            reportFinished(std::move(encryptedResult));
        });
}
