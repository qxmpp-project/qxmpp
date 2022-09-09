// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHttpFileSharingProvider.h"

#include "QXmppClient.h"
#include "QXmppDownload.h"
#include "QXmppHttpUploadManager.h"
#include "QXmppUpload.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <QMimeDatabase>
#include <QNetworkReply>

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \class QXmppHttpFileSharingProvider
///
/// A file sharing provider that uses HTTP File Upload to upload the file.
///
/// \since QXmpp 1.5
///

class QXmppSfsHttpUpload : public QXmppUpload
{
    Q_OBJECT

public:
    QXmppSfsHttpUpload(std::shared_ptr<QXmppHttpUpload> &&httpUpload)
        : inner(std::move(httpUpload))
    {
        connect(inner.get(), &QXmppHttpUpload::finished, this, [this](const QXmppHttpUpload::Result &result) {
            Q_EMIT uploadFinished(std::visit([](auto &&value) -> QXmpp::Private::UploadResult {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, QUrl>) {
                    return std::any(QXmppHttpFileSource(value));
                } else if constexpr (std::is_same_v<T, Cancelled>) {
                    return Cancelled {};
                } else if constexpr (std::is_same_v<T, QXmppError>) {
                    return value;
                }
            },
                                             result));
        });
        connect(inner.get(), &QXmppHttpUpload::progressChanged,
                this, &QXmppSfsHttpUpload::progressChanged);
    }

    float progress() override { return inner->progress(); }
    void cancel() override { inner->cancel(); }
    bool isFinished() override { return inner->isFinished(); }
    quint64 bytesTransferred() override { return inner->bytesSent(); }
    quint64 bytesTotal() override { return inner->bytesTotal(); }

private:
    std::shared_ptr<QXmppHttpUpload> inner;
};

class QXmppHttpDownload : public QXmppDownload
{
    float progress() override
    {
        return calculateProgress(m_bytesSent, m_bytesTotal);
    }

    void cancel() override
    {
        m_aborted = true;
        reply->abort();
    }

    bool isFinished() override
    {
        return m_isFinished;
    }

    quint64 bytesTransferred() override
    {
        return m_bytesSent;
    }

    quint64 bytesTotal() override
    {
        return m_bytesTotal;
    }

public:
    void reportProgress(quint64 bytesSent, quint64 bytesTotal)
    {
        m_bytesSent = bytesSent;
        m_bytesTotal = bytesTotal;
        Q_EMIT progressChanged();
    }

    void reportFinished(Result &&result)
    {
        Q_EMIT finished(result);
        m_isFinished = true;
    }

    [[nodiscard]] bool aborted() const
    {
        return m_aborted;
    }

private:
    QNetworkReply *reply = nullptr;
    quint64 m_bytesSent = 0;
    quint64 m_bytesTotal = 0;
    bool m_isFinished = false;
    bool m_aborted = false;
};

class QXmppHttpFileSharingProviderPrivate
{
public:
    QXmppHttpUploadManager *manager;
    QNetworkAccessManager *netManager;
};

///
/// \brief Create a QXmppHttpFileSharingProvider
/// \param client
/// \param netManager QNetworkAccessManager that can be reused all over your application.
///
QXmppHttpFileSharingProvider::QXmppHttpFileSharingProvider(QXmppClient *client, QNetworkAccessManager *netManager)
    : d(std::make_unique<QXmppHttpFileSharingProviderPrivate>())
{
    qRegisterMetaType<QXmpp::Private::UploadResult>();
    Q_ASSERT(client);
    d->manager = client->findExtension<QXmppHttpUploadManager>();
    Q_ASSERT(d->manager);
    d->netManager = netManager;
}

QXmppHttpFileSharingProvider::~QXmppHttpFileSharingProvider() = default;

auto QXmppHttpFileSharingProvider::downloadFile(const std::any &source, std::unique_ptr<QIODevice> &&target)
    -> std::shared_ptr<QXmppDownload>
{
    QXmppHttpFileSource httpSource;
    try {
        httpSource = std::any_cast<QXmppHttpFileSource>(source);
    } catch (const std::bad_any_cast &) {
        qFatal("QXmppHttpFileSharingProvider::downloadFile can only handle QXmppHttpFileSharingProvider sources");
    }

    auto *reply = d->netManager->get(QNetworkRequest(httpSource.url()));

    auto download = std::make_shared<QXmppHttpDownload>();

    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            download->reportFinished(QXmppError::fromIoDevice(*reply));
        } else if (download->aborted()) {
            download->reportFinished(Cancelled());
        } else {
            download->reportFinished(Success());
        }
        reply->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::readyRead, [file = std::move(target), reply]() {
        file->write(reply->readAll());
    });

    QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        download->reportProgress(bytesReceived, bytesTotal);
    });

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(reply, &QNetworkReply::errorOccurred,
#else
    QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
#endif
                     [download, reply]() {
                         download->reportFinished(QXmppError::fromNetworkReply(*reply));
                     });

    QObject::connect(reply, &QNetworkReply::uploadProgress, [download](qint64 sent, qint64 total) {
        quint64 sentBytes = sent < 0 ? 0 : quint64(sent);
        quint64 totalBytes = total < 0 ? 0 : quint64(total);
        download->reportProgress(sentBytes, totalBytes);
    });

    return download;
}

auto QXmppHttpFileSharingProvider::uploadFile(std::unique_ptr<QIODevice> data,
                                              const QXmppFileMetadata &info)
    -> std::shared_ptr<QXmppUpload>
{
    Q_ASSERT(d->manager);
    auto upload = d->manager->uploadFile(
        std::move(data),
        QXmppUtils::generateStanzaHash(10),
        info.mediaType().value_or(QMimeDatabase().mimeTypeForName("application/octet-stream")),
        info.size() ? info.size().value() : -1);

    return std::make_shared<QXmppSfsHttpUpload>(std::move(upload));
}

#include "QXmppHttpFileSharingProvider.moc"
