// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
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

#include <any>
#include <unordered_map>

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
    std::shared_ptr<QXmppUpload> upload;

    QFileInfo fileInfo(filePath);
    auto metadata = QXmppFileMetadata::fromFileInfo(fileInfo);
    if (description) {
        metadata.setDescription(description);
    }

    auto openFile = [=]() -> std::unique_ptr<QIODevice> {
        auto device = std::make_unique<QFile>(fileInfo.absoluteFilePath());
        if (!device->open(QIODevice::ReadOnly)) {
            Q_EMIT upload->finished(QXmppError::fromIoDevice(*device));
        }
        return device;
    };

    auto metadataFuture = d->metadataGenerator(openFile());
    auto hashesFuture = calculateHashes(openFile(), hashAlgorithms());

    upload = provider->uploadFile(openFile(), metadata);
    upload->metadata = metadata;

    connect(upload.get(), &QXmppUpload::uploadFinished, this, [=](auto &&uploadResult) {
        if (std::holds_alternative<std::any>(uploadResult)) {
            auto source = std::get<std::any>(uploadResult);
            await(metadataFuture, this, [=](auto &&result) mutable {
                if (result->dimensions) {
                    upload->metadata.setWidth(result->dimensions->width());
                    upload->metadata.setHeight(result->dimensions->height());
                }
                if (result->length) {
                    upload->metadata.setLength(*result->length);
                }

                QVector<QXmppThumbnail> thumbnails;
                thumbnails.reserve(result->thumbnails.size());
                QXmppBitsOfBinaryDataList dataBlobs;
                dataBlobs.reserve(result->thumbnails.size());

                for (const auto &metadataThumb : result->thumbnails) {
                    auto bobData = QXmppBitsOfBinaryData::fromByteArray(metadataThumb.data);
                    bobData.setContentType(metadataThumb.mimeType);

                    QXmppThumbnail thumbnail;
                    thumbnail.setHeight(metadataThumb.height);
                    thumbnail.setWidth(metadataThumb.width);
                    thumbnail.setMediaType(metadataThumb.mimeType);
                    thumbnail.setUri(bobData.cid().toCidUrl());

                    thumbnails.append(std::move(thumbnail));
                    dataBlobs.append(std::move(bobData));
                }
                upload->metadata.setThumbnails(thumbnails);

                await(hashesFuture, this, [=, dataBlobs = std::move(dataBlobs)](const auto &hashResult) mutable {
                    QVector<QXmppHash> hashes;
                    const auto hashValue = hashResult->result;
                    if (std::holds_alternative<Cancelled>(hashValue)) {
                        Q_EMIT upload->finished(Cancelled {});
                    } else if (std::holds_alternative<std::vector<QXmppHash>>(hashValue)) {
                        auto hashesVector = std::get<std::vector<QXmppHash>>(hashValue);
                        std::transform(hashesVector.begin(), hashesVector.end(),
                                       std::back_inserter(hashes), [](auto &&hash) {
                                           return hash;
                                       });

                        upload->metadata.setHashes(hashes);

                        QXmppFileShare fs;
                        fs.setMetadata(upload->metadata);
                        fs.addSource(std::move(source));

                        Q_EMIT upload->finished(QXmppUpload::FileResult { fs, std::move(dataBlobs) });
                    } else if (std::holds_alternative<QXmppError>(hashValue)) {
                        Q_EMIT upload->finished(std::get<QXmppError>(hashValue));
                    }
                });
            });
        } else if (std::holds_alternative<QXmppError>(uploadResult)) {
            Q_EMIT upload->finished(std::get<QXmppError>(uploadResult));
        }
    });

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
    std::unique_ptr<QIODevice> &&output)
{
    std::shared_ptr<QXmppDownload> download;
    fileShare.visitSources([&](const std::any &source) {
        std::type_index index(source.type());
        try {
            auto provider = d->providers.at(index);
            download = provider->downloadFile(source, std::move(output));
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
