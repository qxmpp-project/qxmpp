// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFileSharingManager.h"

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryData.h"
#include "QXmppClient.h"
#include "QXmppFileMetadata.h"
#include "QXmppFileShare.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHashing_p.h"
#include "QXmppThumbnail.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return { HashAlgorithm::Sha256, HashAlgorithm::Blake2b_256 };
#else
    return { HashAlgorithm::Sha256, HashAlgorithm::Sha3_256 };
#endif
}

template<typename T, typename Converter>
auto transform(T &input, Converter convert)
{
    using Output = std::decay_t<decltype(convert(input.front()))>;
    std::vector<Output> output;
    output.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(output), std::move(convert));
    return output;
}

class QXmppFileUploadPrivate
{
public:
    std::shared_ptr<QXmppFileSharingProvider::Upload> providerUpload;
    QFuture<std::shared_ptr<MetadataGeneratorResult>> metadataFuture;
    QFuture<HashingResultPtr> hashesFuture;
    std::optional<QXmppError> error;
    QXmppFileMetadata metadata;
    QXmppBitsOfBinaryDataList dataBlobs;
    std::any source;
    quint64 bytesSent = 0;
    quint64 bytesTotal = 0;
    bool finished = false;
    bool cancelled = false;
    bool success = false;
};

///
/// \class QXmppFileUpload
///
/// \brief Provides progress of stateless file sharing uploads.
///
/// \since QXmpp 1.5
///

///
/// \class QXmppFileUpload::FileResult
///
/// \brief Contains QXmppFileShare of the uploaded file and possible data blobs containing
/// referenced thumbnails.
///

///
/// \var QXmppFileUpload::FileResult::fileShare
///
/// \brief File share with file metadata and file shares of the uploaded file.
///

///
/// \var QXmppFileUpload::FileResult::dataBlobs
///
/// \brief Data blobs of possibly in the metadata referenced thumbnails.
///
/// The QXmppFileSharingManager may generate file thumbnails.
///

///
/// \typedef QXmppFileUpload::Result
///
/// \brief Contains FileResult (successfully finished), QXmpp::Cancelled (manually cancelled)
/// or QXmppError (an error occured while uploading).
///

QXmppFileUpload::~QXmppFileUpload() = default;

///
/// Returns the current progress between 0.0 and 1.0.
///
float QXmppFileUpload::progress() const
{
    return calculateProgress(d->bytesSent, d->bytesTotal);
}

///
/// \fn QXmppFileUpload::progressChanged()
///
/// \brief Emitted when new bytes have been transferred.
///

///
/// \brief Cancels the file transfer. finished() will be emitted.
///
void QXmppFileUpload::cancel()
{
    if (d->providerUpload) {
        d->providerUpload->cancel();
    }
    d->metadataFuture.cancel();
    d->hashesFuture.cancel();
}

///
/// \brief Returns whether the file transfer is finished.
///
bool QXmppFileUpload::isFinished() const
{
    return d->finished;
}

///
/// \brief Returns the number of bytes that have been uploaded or downloaded.
///
quint64 QXmppFileUpload::bytesTransferred() const
{
    return d->bytesSent;
}

///
/// \brief Returns the number of bytes that totally need to be transferred.
///
quint64 QXmppFileUpload::bytesTotal() const
{
    return d->bytesTotal;
}

///
/// \brief Returns the result of the upload.
///
/// The upload must be finished when calling this.
///
QXmppFileUpload::Result QXmppFileUpload::result() const
{
    Q_ASSERT(d->finished);

    if (d->error) {
        return *d->error;
    }
    if (d->cancelled) {
        return Cancelled();
    }
    if (d->success) {
        QXmppFileShare fs;
        fs.setMetadata(d->metadata);
        fs.addSource(d->source);

        return FileResult { std::move(fs), d->dataBlobs };
    }

    Q_UNREACHABLE();
}

///
/// \fn QXmppFileUpload::finished
///
/// Emitted when the upload has finished.
///

QXmppFileUpload::QXmppFileUpload()
    : d(std::make_unique<QXmppFileUploadPrivate>())
{
}

void QXmppFileUpload::reportFinished()
{
    Q_ASSERT(d->error || d->cancelled || d->success);
    if (!d->finished) {
        d->finished = true;
        Q_EMIT finished();
    }
}

class QXmppFileDownloadPrivate
{
public:
    std::shared_ptr<QXmppFileSharingProvider::Download> providerDownload;
    QFuture<HashVerificationResultPtr> hashesFuture;
    QVector<QXmppHash> hashes;
    QXmppFileDownload::Result result;
    quint64 bytesReceived = 0;
    quint64 bytesTotal = 0;
    bool finished = false;
};

///
/// \class QXmppFileDownload
///
/// \brief Provides progress of stateless file sharing uploads.
///
/// \since QXmpp 1.5
///

///
/// \enum QXmppFileDownload::HashVerificationResult
///
/// Describes the result of the hash verification.
///

///
/// \struct QXmppFileDownload::Downloaded
///
/// Indicates that the file could be downloaded.
///

///
/// \var QXmppFileDownload::Downloaded::hashVerificationResult
///
/// Describes the result of the hash verification.
///

///
/// \typedef QXmppFileDownload::Result
///
/// \brief Contains QXmpp::Success (successfully finished), QXmpp::Cancelled (manually cancelled)
/// or QXmppError (an error occured while downloading).
///

QXmppFileDownload::~QXmppFileDownload() = default;

///
/// Returns the current progress between 0.0 and 1.0.
///
float QXmppFileDownload::progress() const
{
    return calculateProgress(d->bytesReceived, d->bytesTotal);
}

///
/// \fn QXmppFileDownload::progressChanged
///
/// \brief Emitted when new bytes have been transferred.
///

///
/// \brief Cancels the file transfer. finished() will be emitted.
///
void QXmppFileDownload::cancel()
{
    if (d->providerDownload) {
        d->providerDownload->cancel();
    }
    d->hashesFuture.cancel();
}

///
/// \brief Returns whether the file transfer is finished.
///
bool QXmppFileDownload::isFinished() const
{
    return d->finished;
}

///
/// \brief Returns the number of bytes that have been uploaded or downloaded.
///
quint64 QXmppFileDownload::bytesTransferred() const
{
    return d->bytesReceived;
}

///
/// \brief Returns the number of bytes that totally need to be transferred.
///
quint64 QXmppFileDownload::bytesTotal() const
{
    return d->bytesTotal;
}

///
/// \brief Returns the result of the download.
///
/// The download must be finished when calling this.
///
QXmppFileDownload::Result QXmppFileDownload::result() const
{
    Q_ASSERT(d->finished);
    return d->result;
}

///
/// \fn QXmppFileDownload::finished
///
/// Emitted when the download has finished.
///

QXmppFileDownload::QXmppFileDownload()
    : d(std::make_unique<QXmppFileDownloadPrivate>())
{
}

void QXmppFileDownload::reportProgress(quint64 bytesReceived, quint64 bytesTotal)
{
    d->bytesReceived = bytesReceived;
    d->bytesTotal = bytesTotal;
    Q_EMIT progressChanged();
}

void QXmppFileDownload::reportFinished(Result result)
{
    d->finished = true;
    d->result = std::move(result);
    Q_EMIT finished();
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
std::shared_ptr<QXmppFileUpload> QXmppFileSharingManager::uploadFile(std::shared_ptr<QXmppFileSharingProvider> provider,
                                                                     const QString &filePath,
                                                                     const std::optional<QString> &description)
{
    QFileInfo fileInfo(filePath);
    auto metadata = QXmppFileMetadata::fromFileInfo(fileInfo);
    metadata.setDescription(description);

    std::shared_ptr<QXmppFileUpload> upload(new QXmppFileUpload());
    upload->d->metadata = std::move(metadata);

    auto openFile = [=]() -> std::unique_ptr<QIODevice> {
        auto device = std::make_unique<QFile>(fileInfo.absoluteFilePath());
        if (!device->open(QIODevice::ReadOnly)) {
            upload->d->error = QXmppError::fromIoDevice(*device);
            upload->reportFinished();
        }
        return device;
    };

    auto metadataIoDevice = openFile();
    auto hashesIoDevice = openFile();
    auto uploadIoDevice = openFile();

    if (upload->d->finished) {
        // error occurred while opening file
        return upload;
    }

    upload->d->metadataFuture = d->metadataGenerator(std::move(metadataIoDevice));
    upload->d->hashesFuture = calculateHashes(std::move(hashesIoDevice), hashAlgorithms());

    auto onProgress = [upload](quint64 sent, quint64 total) {
        upload->d->bytesSent = sent;
        upload->d->bytesTotal = total;
        Q_EMIT upload->progressChanged();
    };
    auto onFinished = [this, upload](QXmppFileSharingProvider::UploadResult uploadResult) {
        // free memory
        upload->d->providerUpload.reset();
        if (std::holds_alternative<std::any>(uploadResult)) {
            upload->d->source = std::get<std::any>(std::move(uploadResult));
            await(upload->d->metadataFuture, this, [this, upload](auto &&result) mutable {
                if (result->dimensions) {
                    upload->d->metadata.setWidth(result->dimensions->width());
                    upload->d->metadata.setHeight(result->dimensions->height());
                }
                if (result->length) {
                    upload->d->metadata.setLength(*result->length);
                }

                if (!result->thumbnails.empty()) {
                    QVector<QXmppThumbnail> thumbnails;
                    thumbnails.reserve(result->thumbnails.size());
                    upload->d->dataBlobs.reserve(result->thumbnails.size());

                    for (const auto &metadataThumb : result->thumbnails) {
                        auto bobData = QXmppBitsOfBinaryData::fromByteArray(metadataThumb.data);
                        bobData.setContentType(metadataThumb.mimeType);

                        QXmppThumbnail thumbnail;
                        thumbnail.setHeight(metadataThumb.height);
                        thumbnail.setWidth(metadataThumb.width);
                        thumbnail.setMediaType(metadataThumb.mimeType);
                        thumbnail.setUri(bobData.cid().toCidUrl());

                        thumbnails.append(std::move(thumbnail));
                        upload->d->dataBlobs.append(std::move(bobData));
                    }
                    upload->d->metadata.setThumbnails(thumbnails);
                }

                await(upload->d->hashesFuture, this, [upload](auto hashResult) mutable {
                    auto &hashValue = hashResult->result;
                    if (std::holds_alternative<std::vector<QXmppHash>>(hashValue)) {
                        const auto &hashesVector = std::get<std::vector<QXmppHash>>(hashValue);
                        QVector<QXmppHash> hashes;
                        hashes.reserve(hashesVector.size());
                        std::transform(hashesVector.begin(), hashesVector.end(),
                                       std::back_inserter(hashes), [](auto &&hash) {
                                           return hash;
                                       });
                        upload->d->metadata.setHashes(hashes);
                        upload->d->success = true;
                    } else if (std::holds_alternative<Cancelled>(hashValue)) {
                        upload->d->cancelled = true;
                    } else if (std::holds_alternative<QXmppError>(hashValue)) {
                        upload->d->error = std::get<QXmppError>(std::move(hashValue));
                    }
                    upload->reportFinished();
                });
            });
        } else if (std::holds_alternative<Cancelled>(uploadResult)) {
            upload->d->cancelled = true;
            upload->reportFinished();
        } else if (std::holds_alternative<QXmppError>(uploadResult)) {
            upload->d->error = std::get<QXmppError>(std::move(uploadResult));
            upload->reportFinished();
        }
    };

    upload->d->providerUpload = provider->uploadFile(std::move(uploadIoDevice), upload->d->metadata, std::move(onProgress), std::move(onFinished));
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
std::shared_ptr<QXmppFileDownload> QXmppFileSharingManager::downloadFile(
    const QXmppFileShare &fileShare,
    std::unique_ptr<QIODevice> output)
{
    std::shared_ptr<QXmppFileDownload> download(new QXmppFileDownload());
    download->d->hashes = fileShare.metadata().hashes();

    // currently hashing does only work with QFiles
    auto filePath = [&]() -> QString {
        if (auto *file = dynamic_cast<QFile *>(output.get())) {
            return file->fileName();
        }
        return {};
    }();

    auto onProgress = [download](quint64 received, quint64 total) {
        download->reportProgress(received, total);
    };
    auto onFinished = [this, download, filePath](QXmppFileSharingProvider::DownloadResult result) mutable {
        // reduce ref count
        download->d->providerDownload.reset();

        // pass errors directly
        if (std::holds_alternative<Cancelled>(result)) {
            download->reportFinished(Cancelled());
            return;
        }
        if (std::holds_alternative<QXmppError>(result)) {
            download->reportFinished(std::get<QXmppError>(std::move(result)));
            return;
        }

        // try to do hash verification
        if (filePath.isEmpty()) {
            warning(QStringLiteral("Can't verify hashes of other io devices than QFile!"));
            download->reportFinished(QXmppFileDownload::Downloaded { QXmppFileDownload::NoStrongHashes });
            return;
        }

        auto file = std::make_unique<QFile>(filePath);
        if (!file->open(QIODevice::ReadOnly)) {
            download->reportFinished(QXmppError::fromFileDevice(*file));
            return;
        }

        download->d->hashesFuture = verifyHashes(
            std::move(file),
            transform(download->d->hashes, [](auto hash) { return hash; }));

        await(download->d->hashesFuture, this, [download](HashVerificationResultPtr hashResult) {
            auto convert = overloaded {
                [](HashVerificationResult::NoStrongHashes) {
                    return QXmppFileDownload::Downloaded {
                        QXmppFileDownload::NoStrongHashes
                    };
                },
                [](HashVerificationResult::NotMatching) {
                    return QXmppError {
                        QStringLiteral("Checksum does not match"),
                        {}
                    };
                },
                [](HashVerificationResult::Verified) {
                    return QXmppFileDownload::Downloaded {
                        QXmppFileDownload::HashVerified
                    };
                }
            };
            download->reportFinished(visitForward<QXmppFileDownload::Result>(hashResult->result, convert));
        });
    };

    fileShare.visitSources([&](const std::any &source) {
        if (auto provider = providerForSource(source)) {
            download->d->providerDownload = provider->downloadFile(source, std::move(output), std::move(onProgress), std::move(onFinished));
            return true;
        }
        return false;
    });

    return download;
}

void QXmppFileSharingManager::internalRegisterProvider(std::type_index index, std::shared_ptr<QXmppFileSharingProvider> provider)
{
    d->providers.insert_or_assign(index, provider);
}

std::shared_ptr<QXmppFileSharingProvider> QXmppFileSharingManager::providerForSource(const std::any &source) const
{
    if (auto provider = d->providers.find(std::type_index(source.type()));
        provider != d->providers.cend()) {
        return provider->second;
    }
    return {};
}
