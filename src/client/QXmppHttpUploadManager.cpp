// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHttpUploadManager.h"

#include "QXmppClient.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHttpUploadIq.h"
#include "QXmppUploadRequestManager.h"

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>

using namespace QXmpp::Private;

struct QXmppHttpUploadPrivate
{
    explicit QXmppHttpUploadPrivate(QXmppHttpUpload *q) : q(q) { }

    QUrl getUrl;
    std::optional<QXmppError> error;
    quint64 bytesSent = 0;
    quint64 bytesTotal = 0;
    QPointer<QNetworkReply> reply;
    bool finished = false;
    bool cancelled = false;

    void reportError(QXmppStanza::Error err)
    {
        error = QXmppError {
            err.text(),
            std::move(err)
        };
    }
    void reportError(QXmppError newError)
    {
        error.emplace(std::move(newError));
    }
    void reportFinished()
    {
        if (!finished) {
            finished = true;
            emit q->finished(result());
        }
    }
    void reportProgress(quint64 sent, quint64 total)
    {
        if (total == 0 && bytesTotal > 0) {
            // QNetworkReply resets the progress in the end
            // ignore that so we can still see how large the file was
            return;
        }

        if (bytesSent != sent || bytesTotal != total) {
            bytesSent = sent;
            bytesTotal = total;
            emit q->progressChanged();
        }
    }
    [[nodiscard]] QXmppHttpUpload::Result result() const
    {
        // assumes finished = true
        if (error) {
            return *error;
        }
        if (cancelled) {
            return QXmppHttpUpload::Cancelled();
        }
        return getUrl;
    }

private:
    QXmppHttpUpload *q;
};

///
/// \class QXmppHttpUpload
///
/// Object that represents an ongoing or finished upload.
///
/// \since QXmpp 1.5
///

///
/// \property QXmppHttpUpload::progress
///
/// The current progress of the upload as a floating point number between 0 and 1.
///

///
/// \property QXmppHttpUpload::bytesSent
///
/// Number of bytes sent so far
///

///
/// \property QXmppHttpUpload::bytesTotal
///
/// Number of bytes that need to be sent in total to complete the upload
///

///
/// \fn QXmppHttpUpload::progressChanged
///
/// Emitted when the upload has made progress.
///

///
/// \fn QXmppHttpUpload::finished
///
/// Emitted when the upload has finished for any reason (success, cancelled, error).
///
/// \param result Result of the upload
///

QXmppHttpUpload::~QXmppHttpUpload() = default;

///
/// \class QXmppHttpUploadManager
///
/// The upload manager allows to upload a file to a server via \xep{0363, HTTP File Upload}.
/// This can be used for sending files to other users.
///
/// QXmppHttpUploadManager depends on QXmppUploadRequestManager. You can enable them using
/// \code
/// client.addNewExtension<QXmppUploadRequestManager>();
/// auto *uploadManager = client.addNewExtension<QXmppHttpUploadManager>();
/// \endcode
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppHttpUpload::Result
///
/// Represents the result of an upload. It can either be a url to the
/// uploaded file, a QXmppHttpUpload::Cancelled unit struct, or an error as
/// QXmppError.
///

///
/// Returns the current progress of the upload as a floating point number between 0 and 1.
///
float QXmppHttpUpload::progress() const
{
    if (d->bytesTotal > 0) {
        if (d->bytesSent > d->bytesTotal) {
            return 1;
        }
        return float(d->bytesSent) / d->bytesTotal;
    }
    return 0;
}

///
/// The number of bytes sent so far.
///
quint64 QXmppHttpUpload::bytesSent() const
{
    return d->bytesSent;
}

///
/// The number of bytes that need to be sent in total to complete the upload.
///
quint64 QXmppHttpUpload::bytesTotal() const
{
    return d->bytesTotal;
}

///
/// Cancels the upload.
///
void QXmppHttpUpload::cancel()
{
    d->cancelled = true;
    if (d->reply) {
        d->reply->abort();
    }
}

///
/// Returns whether the upload is already finished.
///
bool QXmppHttpUpload::isFinished() const
{
    return d->finished;
}

///
/// If the upload has already finished, returns the result of the upload as QXmppHttpUpload::Result,
/// otherwise returns an empty std::optional.
///
std::optional<QXmppHttpUpload::Result> QXmppHttpUpload::result() const
{
    if (d->finished) {
        return d->result();
    }
    return {};
}

QXmppHttpUpload::QXmppHttpUpload()
    : d(std::make_unique<QXmppHttpUploadPrivate>(this))
{
}

struct QXmppHttpUploadManagerPrivate
{
    explicit QXmppHttpUploadManagerPrivate(QNetworkAccessManager *netManager)
        : netManager(netManager)
    {
    }

    QNetworkAccessManager *netManager;
};

///
/// Constructor
///
/// Creates and uses a new network access manager.
///
QXmppHttpUploadManager::QXmppHttpUploadManager()
    : d(std::make_unique<QXmppHttpUploadManagerPrivate>(new QNetworkAccessManager(this)))
{
}

///
/// Constructor
///
/// \param netManager shared network access manager, needs to have at least the lifetime of this
/// manager.
///
QXmppHttpUploadManager::QXmppHttpUploadManager(QNetworkAccessManager *netManager)
    : d(std::make_unique<QXmppHttpUploadManagerPrivate>(netManager))
{
}

QXmppHttpUploadManager::~QXmppHttpUploadManager() = default;

///
/// Uploads the data from a QIODevice.
///
/// \param data QIODevice to read the data from. This can for example be a QFile.
///        It can be sequential or non-sequential.
/// \param filename How the file on the server should be called.
///        This is commonly used as last part of the resulting url.
/// \param fileSize The size of the file, in byte. If negative the size from the IO device is used.
/// \param mimeType The mime type of the file
/// \param uploadServiceJid optionally,
///        the jid from which an upload url can be requested (upload service)
/// \return an object representing the ongoing upload.
///         The object is passed as a shared_ptr,
///         which means it will be stored as long as there is a reference to it.
///         While this avoids errors from accessing it after it was deleted,
///         you should try not store it unneccesarily long to keep the memory usage down.
///         You can for example use std::weak_ptr to not increase the lifetime,
///         for example when capturing in long living lambdas, for example in connects.
///         You should also make sure to use the 4-arg QObject::connect,
///         so the connection and the connected lambda can be deleted after the upload finished.
///         \code
///         std::weak_ptr<QXmppHttpUpload> uploadPtr = upload;
///         connect(upload.get(), &QXmppHttpUpload::progressChanged, this, [uploadPtr]() {
///             auto upload = uploadPtr.lock();
///             if (upload) {
///                 qDebug() << upload->progress();
///             }
///         });
///         \endcode
///
std::shared_ptr<QXmppHttpUpload> QXmppHttpUploadManager::uploadFile(QIODevice *data, const QString &filename, const QMimeType &mimeType, qint64 fileSize, const QString &uploadServiceJid)
{
    using SlotResult = QXmppUploadRequestManager::SlotResult;

    std::shared_ptr<QXmppHttpUpload> upload(new QXmppHttpUpload);

    if (!data->isOpen()) {
        upload->d->reportError({ QStringLiteral("Input data device MUST be open."), std::any() });
        upload->d->reportFinished();
        return upload;
    }

    if (fileSize < 0) {
        if (!data->isSequential()) {
            fileSize = data->size();
        } else {
            warning(QStringLiteral("No fileSize set and cannot determine size from IO device."));
            upload->d->reportError({ QStringLiteral("File size MUST be set for sequential devices."), std::any() });
            upload->d->reportFinished();
            return upload;
        }
    }

    auto future = client()->findExtension<QXmppUploadRequestManager>()->requestSlot(filename, fileSize, mimeType, uploadServiceJid);
    await(future, this, [this, upload, data](SlotResult result) {
        // first check whether upload was cancelled in the meantime
        if (upload->d->cancelled) {
            upload->d->reportFinished();
            return;
        }

        if (std::holds_alternative<QXmppStanza::Error>(result)) {
            upload->d->reportError(std::get<QXmppStanza::Error>(std::move(result)));
            upload->d->reportFinished();
        } else {
            auto slot = std::get<QXmppHttpUploadSlotIq>(std::move(result));
            upload->d->getUrl = slot.getUrl();

            QNetworkRequest request(slot.putUrl());
            auto headers = slot.putHeaders();
            for (auto itr = headers.cbegin(); itr != headers.cend(); ++itr) {
                request.setRawHeader(itr.key().toUtf8(), itr.value().toUtf8());
            }
            auto *reply = d->netManager->put(request, data);
            upload->d->reply = reply;

            connect(reply, &QNetworkReply::finished, this, [reply, upload]() {
                if (reply->error() == QNetworkReply::NoError) {
                    upload->d->reportFinished();
                }
                reply->deleteLater();
            });

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            connect(reply, &QNetworkReply::errorOccurred, this,
#else
            connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this,
#endif
                    [upload, reply](QNetworkReply::NetworkError error) {
                        upload->d->reportError({ reply->errorString(), error });
                        upload->d->reportFinished();
                        reply->deleteLater();
                    });

            connect(reply, &QNetworkReply::uploadProgress, this, [upload](qint64 sent, qint64 total) {
                quint64 sentBytes = sent < 0 ? 0 : quint64(sent);
                quint64 totalBytes = total < 0 ? 0 : quint64(total);
                upload->d->reportProgress(sentBytes, totalBytes);
            });
        }
    });

    return upload;
}

///
/// Upload data from a local file.
///
/// \param fileInfo QFileInfo about a local file
/// \param filename filename How the file on the server should be called.
///        This is commonly used as last part of the resulting url.
/// \param uploadServiceJid optionally,
///        the jid from which an upload url can be requested (upload service)
/// \return an object representing the ongoing upload.
///         The object is passed as a shared_ptr,
///         which means it will be stored as long as there is a reference to it.
///         While this avoids errors from accessing it after it was deleted,
///         you should try not store it unneccesarily long to keep the memory usage down.
///         You can for example use std::weak_ptr to not increase the lifetime,
///         for example when capturing in long living lambdas, for example in connects.
///         You should also make sure to use the 4-arg QObject::connect,
///         so the connection and the connected lambda can be deleted after the upload finished.
///         \code
///         std::weak_ptr<QXmppHttpUpload> uploadPtr = upload;
///         connect(upload.get(), &QXmppHttpUpload::progressChanged, this, [uploadPtr]() {
///             auto upload = uploadPtr.lock();
///             if (upload) {
///                 qDebug() << upload->progress();
///             }
///         });
///         \endcode
std::shared_ptr<QXmppHttpUpload> QXmppHttpUploadManager::uploadFile(const QFileInfo &fileInfo, const QString &filename, const QString &uploadServiceJid)
{
    auto file = std::make_unique<QFile>(fileInfo.absoluteFilePath());
    if (!file->open(QIODevice::ReadOnly)) {
        std::shared_ptr<QXmppHttpUpload> upload(new QXmppHttpUpload);
        upload->d->reportError({ file->errorString(), file->error() });
        upload->d->reportFinished();
        return upload;
    }

    auto upload = uploadFile(
        file.get(),
        filename.isEmpty() ? fileInfo.fileName() : filename,
        QMimeDatabase().mimeTypeForFile(fileInfo),
        -1,
        uploadServiceJid);
    // from now on lifetime is bound to the upload
    file.release()->setParent(upload.get());
    return upload;
}
