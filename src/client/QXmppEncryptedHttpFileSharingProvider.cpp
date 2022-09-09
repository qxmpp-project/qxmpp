// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEncryptedHttpFileSharingProvider.h"

#include "QXmppClient.h"
#include "QXmppFileEncryption.h"
#include "QXmppHttpUploadManager.h"
#include "QXmppUpload.h"
#include "QXmppUtils.h"

#include "QcaInitializer_p.h"
#include <QMimeDatabase>

using namespace QXmpp;
using namespace QXmpp::Private;

class QXmppSfsEncryptedHttpUpload : public QXmppUpload
{
    Q_OBJECT

public:
    QXmppSfsEncryptedHttpUpload(std::shared_ptr<QXmppHttpUpload> &&httpUpload, const QByteArray &key, const QByteArray &iv)
        : inner(std::move(httpUpload))
    {
        connect(inner.get(), &QXmppHttpUpload::finished, this, [=](const QXmppHttpUpload::Result &result) {
            Q_EMIT uploadFinished(std::visit([=](auto &&value) -> QXmpp::Private::UploadResult {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, QUrl>) {
                    QXmppEncryptedFileSource encryptedSource;
                    encryptedSource.setKey(key);
                    encryptedSource.setIv(iv);
                    encryptedSource.setHttpSources({ QXmppHttpFileSource(value) });
                    return std::any(encryptedSource);
                } else if constexpr (std::is_same_v<T, Cancelled>) {
                    return Cancelled {};
                } else if constexpr (std::is_same_v<T, QXmppError>) {
                    return value;
                }
            },
                                             result));
        });
    }

    float progress() override { return inner->progress(); }
    void cancel() override { inner->cancel(); }
    bool isFinished() override { return inner->isFinished(); }
    quint64 bytesTransferred() override { return inner->bytesSent(); }
    quint64 bytesTotal() override { return inner->bytesTotal(); }

private:
    std::shared_ptr<QXmppHttpUpload> inner;
};

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
    QXmppHttpUploadManager *manager;
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
    qRegisterMetaType<QXmpp::Private::UploadResult>();
    Q_ASSERT(client);
    d->manager = client->findExtension<QXmppHttpUploadManager>();
    Q_ASSERT(d->manager);
    d->httpProvider = new QXmppHttpFileSharingProvider(client, netManager);
}

QXmppEncryptedHttpFileSharingProvider::~QXmppEncryptedHttpFileSharingProvider() = default;

std::shared_ptr<QXmppDownload> QXmppEncryptedHttpFileSharingProvider::downloadFile(const std::any &source, std::unique_ptr<QIODevice> &&target)
{
    QXmppEncryptedFileSource encryptedSource;
    try {
        encryptedSource = std::any_cast<QXmppEncryptedFileSource>(source);
    } catch (const std::bad_any_cast &) {
        qFatal("QXmppEncryptedHttpFileSharingProvider::downloadFile can only handle QXmppEncryptedFileSource sources");
    }

    auto httpSource = encryptedSource.httpSources().front();
    auto output = std::make_unique<Encryption::DecryptionDevice>(std::move(target), encryptedSource.cipher(), encryptedSource.iv(), encryptedSource.key());
    return d->httpProvider->downloadFile(httpSource, std::move(output));
}

std::shared_ptr<QXmppUpload> QXmppEncryptedHttpFileSharingProvider::uploadFile(
    std::unique_ptr<QIODevice> data,
    const QXmppFileMetadata &info)
{
    auto cipher = Aes256CbcPkcs7;
    auto key = Encryption::generateKey(cipher);
    auto iv = Encryption::generateInitializationVector(cipher);

    auto encDevice = std::make_unique<Encryption::EncryptionDevice>(std::move(data), cipher, key, iv);
    auto encryptedSize = encDevice->size();

    auto upload = d->manager->uploadFile(
        std::move(encDevice),
        QXmppUtils::generateStanzaHash(10),
        QMimeDatabase().mimeTypeForName("application/octet-stream"),
        encryptedSize);

    return std::make_shared<QXmppSfsEncryptedHttpUpload>(std::move(upload), key, iv);
}

#include "QXmppEncryptedHttpFileSharingProvider.moc"
