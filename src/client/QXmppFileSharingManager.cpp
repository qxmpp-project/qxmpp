// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFileSharingManager.h"

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryData.h"
#include "QXmppClient.h"
#include "QXmppDownload.h"
#include "QXmppFileMetadata.h"
#include "QXmppFileShare.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHashing_p.h"
#include "QXmppThumbnail.h"
#include "QXmppUpload.h"
#include "QXmppUploadRequestManager.h"
#include "QXmppUtils_p.h"

#include <any>
#include <unordered_map>
#include <utility>

#include <QFile>
#include <QFileInfo>
#include <QFutureInterface>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QXmpp;
using namespace QXmpp::Private;

using MetadataGenerator = QXmppFileSharingManager::MetadataGenerator;
using MetadataGeneratorResult = QXmppFileSharingManager::MetadataGeneratorResult;

// The manager generates a hash with each hash algorithm
static std::vector<HashAlgorithm> hashAlgorithms()
{
    return {
        HashAlgorithm::Sha256,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        HashAlgorithm::Blake2b_256,
#else
        HashAlgorithm::Sha3_256,
#endif
    };
}

class UploadImpl : public QXmppUpload
{
    Q_OBJECT
public:
    float progress() const override { return calculateProgress(m_bytesSent, m_bytesTotal); }
    void cancel() override
    {
        if (m_providerUpload) {
            m_providerUpload->cancel();
        }
        m_metadataFuture.cancel();
        m_hashesFuture.cancel();
    }
    bool isFinished() const override { return m_finished; }
    quint64 bytesTransferred() const override { return m_bytesSent; }
    quint64 bytesTotal() const override { return m_bytesTotal; }

    void reportFinished(Result result)
    {
        if (!m_finished) {
            m_finished = true;
            Q_EMIT finished(std::move(result));
        }
    }

    std::shared_ptr<QXmppFileSharingProvider::Upload> m_providerUpload;
    QFuture<std::shared_ptr<MetadataGeneratorResult>> m_metadataFuture;
    QFuture<HashingResultPtr> m_hashesFuture;
    QXmppFileMetadata m_metadata;
    QXmppBitsOfBinaryDataList m_dataBlobs;
    std::any m_source;
    quint64 m_bytesSent = 0;
    quint64 m_bytesTotal = 0;
    bool m_finished = false;
};

class DownloadImpl : public QXmppDownload
{
    float progress() const override { return calculateProgress(m_bytesReceived, m_bytesTotal); }
    void cancel() override
    {
        if (m_providerDownload) {
            m_providerDownload->cancel();
        }
    }
    bool isFinished() const override { return m_finished; }
    quint64 bytesTransferred() const override { return m_bytesReceived; }
    quint64 bytesTotal() const override { return m_bytesTotal; }

public:
    void reportProgress(quint64 bytesReceived, quint64 bytesTotal)
    {
        m_bytesReceived = bytesReceived;
        m_bytesTotal = bytesTotal;
        Q_EMIT progressChanged();
    }
    void reportFinished(Result result)
    {
        m_finished = true;
        Q_EMIT finished(std::move(result));
    }

    std::shared_ptr<QXmppFileSharingProvider::Download> m_providerDownload;
    quint64 m_bytesReceived = 0;
    quint64 m_bytesTotal = 0;
    bool m_finished = false;
};

class QXmppFileSharingManagerPrivate
{
public:
    MetadataGenerator metadataGenerator = [](std::unique_ptr<QIODevice>) -> QFuture<std::shared_ptr<MetadataGeneratorResult>> {
        return makeReadyFuture(std::make_shared<MetadataGeneratorResult>());
    };
    std::unordered_map<std::type_index, std::shared_ptr<QXmppFileSharingProvider>> providers;
};

///
/// \class QXmppFileSharingProvider
///
/// Base class for Stateless File Sharing providers
///
/// A provider defines the way that files can be uploaded and downloaded.
///
/// An example is the QXmppHttpFileSharingProvider, which uses HTTP File Upload.
///
/// \since QXmpp 1.5
///

///
/// \class QXmppFileSharingManager
///
/// The file sharing manager allows to easily send and retrieve files in a chat.
///
/// \since QXmpp 1.5
///

QXmppFileSharingManager::QXmppFileSharingManager()
    : d(std::make_unique<QXmppFileSharingManagerPrivate>())
{
}

QXmppFileSharingManager::~QXmppFileSharingManager() = default;

///
/// \typedef QXmppFileSharingManager::MetadataGenerator
///
/// The function signature of a metadata generator function
///

///
/// \brief Register a function that is called when metadata needs to be gererated for a file.
///
/// The function is passed a QIODevice, so if you need the path of the file on disk,
/// you can dynamically cast it to a QFile and access the fileName.
/// When doing that, make sure to check the result,
/// as in the future this function might be passed other QIODevices than QFile.
///
void QXmppFileSharingManager::setMetadataGenerator(MetadataGenerator &&generator)
{
    d->metadataGenerator = std::move(generator);
}

///
/// \brief Upload a file in a way that it can be attached to a message.
/// \param provider The provider class decides how the file is uploaded
/// \param filePath Path to a file that should be uploaded
/// \param description Optional description of the file
/// \return An object that allows to track the progress of the upload.
///         Once the upload is finished, the finished signal is emitted on it.
///
std::shared_ptr<QXmppUpload> QXmppFileSharingManager::sendFile(std::shared_ptr<QXmppFileSharingProvider> provider,
                                                               const QString &filePath,
                                                               const std::optional<QString> &description)
{
    QFileInfo fileInfo(filePath);
    auto metadata = QXmppFileMetadata::fromFileInfo(fileInfo);
    metadata.setDescription(description);

    auto upload = std::make_shared<UploadImpl>();
    upload->m_metadata = std::move(metadata);

    auto openFile = [=]() -> std::unique_ptr<QIODevice> {
        auto device = std::make_unique<QFile>(fileInfo.absoluteFilePath());
        if (!device->open(QIODevice::ReadOnly)) {
            upload->reportFinished(QXmppError::fromIoDevice(*device));
        }
        return device;
    };

    auto metadataIoDevice = openFile();
    auto hashesIoDevice = openFile();
    auto uploadIoDevice = openFile();

    if (upload->m_finished) {
        // error occurred while opening file
        return upload;
    }

    upload->m_metadataFuture = d->metadataGenerator(std::move(metadataIoDevice));
    upload->m_hashesFuture = calculateHashes(std::move(hashesIoDevice), hashAlgorithms());

    auto onProgress = [upload](quint64 sent, quint64 total) {
        upload->m_bytesSent = sent;
        upload->m_bytesTotal = total;
        Q_EMIT upload->progressChanged();
    };
    auto onFinished = [this, upload](QXmppFileSharingProvider::UploadResult uploadResult) {
        // free memory
        upload->m_providerUpload.reset();
        if (std::holds_alternative<std::any>(uploadResult)) {
            upload->m_source = std::get<std::any>(std::move(uploadResult));
            await(upload->m_metadataFuture, this, [this, upload](auto &&result) mutable {
                if (result->dimensions) {
                    upload->m_metadata.setWidth(result->dimensions->width());
                    upload->m_metadata.setHeight(result->dimensions->height());
                }
                if (result->length) {
                    upload->m_metadata.setLength(*result->length);
                }

                if (!result->thumbnails.empty()) {
                    QVector<QXmppThumbnail> thumbnails;
                    thumbnails.reserve(result->thumbnails.size());
                    upload->m_dataBlobs.reserve(result->thumbnails.size());

                    for (const auto &metadataThumb : result->thumbnails) {
                        auto bobData = QXmppBitsOfBinaryData::fromByteArray(metadataThumb.data);
                        bobData.setContentType(metadataThumb.mimeType);

                        QXmppThumbnail thumbnail;
                        thumbnail.setHeight(metadataThumb.height);
                        thumbnail.setWidth(metadataThumb.width);
                        thumbnail.setMediaType(metadataThumb.mimeType);
                        thumbnail.setUri(bobData.cid().toCidUrl());

                        thumbnails.append(std::move(thumbnail));
                        upload->m_dataBlobs.append(std::move(bobData));
                    }
                    upload->m_metadata.setThumbnails(thumbnails);
                }

                await(upload->m_hashesFuture, this, [upload](auto hashResult) mutable {
                    auto &hashValue = hashResult->result;
                    if (std::holds_alternative<std::vector<QXmppHash>>(hashValue)) {
                        const auto &hashesVector = std::get<std::vector<QXmppHash>>(hashValue);
                        QVector<QXmppHash> hashes;
                        hashes.reserve(hashesVector.size());
                        std::transform(hashesVector.begin(), hashesVector.end(),
                                       std::back_inserter(hashes), [](auto &&hash) {
                                           return hash;
                                       });
                        upload->m_metadata.setHashes(hashes);

                        QXmppFileShare fs;
                        fs.setMetadata(upload->m_metadata);
                        fs.addSource(upload->m_source);

                        upload->reportFinished(QXmppUpload::FileResult { fs, std::move(upload->m_dataBlobs) });
                    } else if (std::holds_alternative<Cancelled>(hashValue)) {
                        upload->reportFinished(Cancelled());
                    } else if (std::holds_alternative<QXmppError>(hashValue)) {
                        upload->reportFinished(std::get<QXmppError>(std::move(hashValue)));
                    }
                });
            });
        } else if (std::holds_alternative<Cancelled>(uploadResult)) {
            upload->reportFinished(Cancelled());
        } else if (std::holds_alternative<QXmppError>(uploadResult)) {
            upload->reportFinished(std::get<QXmppError>(std::move(uploadResult)));
        }
    };

    upload->m_providerUpload = provider->uploadFile(std::move(uploadIoDevice), upload->m_metadata, std::move(onProgress), std::move(onFinished));
    return upload;
}

///
/// \brief Download a file from a QXmppFileShare
///
/// \warning This function currently does not check the hash of the downloaded file.
///
/// Make sure to register the provider
/// that handles the sources used in this file share before calling this function.
///
/// \param fileShare The file share object which you want to download
/// \param output An open QIODevice that the data should be written into.
///               In most cases, a QFile
/// \return An object that allows to track the progress of the download.
///
std::shared_ptr<QXmppDownload> QXmppFileSharingManager::downloadFile(
    const QXmppFileShare &fileShare,
    std::unique_ptr<QIODevice> output)
{
    auto download = std::make_shared<DownloadImpl>();

    auto onProgress = [download](quint64 received, quint64 total) {
        download->reportProgress(received, total);
    };
    auto onFinished = [download](QXmppFileSharingProvider::DownloadResult result) {
        download->reportFinished(std::move(result));
    };

    fileShare.visitSources([&](const std::any &source) {
        std::type_index index(source.type());
        try {
            download->m_providerDownload = d->providers.at(index)->downloadFile(source, std::move(output), std::move(onProgress), std::move(onFinished));
            return true;
        } catch (const std::out_of_range &) {
            return false;
        }
    });

    return download;
}

void QXmppFileSharingManager::internalRegisterProvider(std::type_index index, std::shared_ptr<QXmppFileSharingProvider> provider)
{
    d->providers.insert_or_assign(index, provider);
}

#include "QXmppFileSharingManager.moc"
