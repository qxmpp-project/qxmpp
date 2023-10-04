// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppHttpFileSharingProvider.h"

#include "QXmppFileMetadata.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHttpUploadManager.h"
#include "QXmppUtils.h"

#include <QMimeDatabase>
#include <QNetworkReply>

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \class QXmppHttpFileSharingProvider
///
/// A file sharing provider that uses HTTP File Upload to upload and download files.
///
/// \since QXmpp 1.5
///

class QXmppHttpFileSharingProviderPrivate
{
public:
    QXmppHttpUploadManager *manager;
    QNetworkAccessManager *netManager;
};

///
/// \brief Create a QXmppHttpFileSharingProvider
/// \param manager
/// \param netManager QNetworkAccessManager that can be reused all over your application.
///
QXmppHttpFileSharingProvider::QXmppHttpFileSharingProvider(QXmppHttpUploadManager *manager, QNetworkAccessManager *netManager)
    : d(std::make_unique<QXmppHttpFileSharingProviderPrivate>())
{
    d->manager = manager;
    d->netManager = netManager;
}

QXmppHttpFileSharingProvider::~QXmppHttpFileSharingProvider() = default;

auto QXmppHttpFileSharingProvider::downloadFile(const std::any &source,
                                                std::unique_ptr<QIODevice> target,
                                                std::function<void(quint64, quint64)> reportProgress,
                                                std::function<void(DownloadResult)> reportFinished)
    -> std::shared_ptr<Download>
{
    struct State : Download
    {
        ~State() override = default;

        std::unique_ptr<QIODevice> output;
        std::function<void(DownloadResult)> reportFinished;
        std::optional<QXmppError> error;
        QNetworkReply *reply = nullptr;
        bool finished = false;
        bool cancelled = false;

        void cancel() override
        {
            if (!cancelled && !finished) {
                cancelled = true;
                reply->abort();
            }
        }
        void finish(DownloadResult &&result)
        {
            if (output && output->isOpen()) {
                output->close();
            }
            reportFinished(std::move(result));
            finished = true;
            reply->deleteLater();
        }
    };

    QXmppHttpFileSource httpSource;
    try {
        httpSource = std::any_cast<QXmppHttpFileSource>(source);
    } catch (const std::bad_any_cast &) {
        qFatal("QXmppHttpFileSharingProvider::downloadFile can only handle QXmppHttpFileSource.");
    }

    auto state = std::make_shared<State>();
    state->output = std::move(target);
    state->reportFinished = std::move(reportFinished);
    state->reply = d->netManager->get(QNetworkRequest(httpSource.url()));

    QObject::connect(state->reply, &QNetworkReply::finished, [state]() mutable {
        if (!state->finished) {
            if (state->error) {
                state->finish(std::move(*state->error));
            } else if (state->cancelled) {
                state->finish(Cancelled());
            } else {
                state->finish(Success());
            }
        }
    });

    QObject::connect(state->reply, &QNetworkReply::readyRead, [state]() {
        Q_ASSERT(state->output);

        auto data = state->reply->readAll();
        if (state->output->write(data) != data.size()) {
            state->error = QXmppError::fromIoDevice(*state->output);
        }
    });

    QObject::connect(state->reply, &QNetworkReply::downloadProgress, [state, reportProgress = std::move(reportProgress)](qint64 bytesReceived, qint64 bytesTotal) {
        if (!state->finished) {
            reportProgress(bytesReceived, bytesTotal);
        }
    });

    QObject::connect(state->reply, &QNetworkReply::errorOccurred,
                     [state](QNetworkReply::NetworkError) {
                         // Qt doc: the finished() signal will "probably" follow
                         // => we can't be sure that finished() is going to be called
                         state->finish(QXmppError::fromNetworkReply(*state->reply));
                     });

    return std::dynamic_pointer_cast<QXmppFileSharingProvider::Download>(state);
}

auto QXmppHttpFileSharingProvider::uploadFile(std::unique_ptr<QIODevice> data,
                                              const QXmppFileMetadata &info,
                                              std::function<void(quint64, quint64)> reportProgress,
                                              std::function<void(UploadResult)> reportFinished)
    -> std::shared_ptr<Upload>
{
    struct State : Upload
    {
        ~State() override = default;

        std::shared_ptr<QXmppHttpUpload> upload;
        void cancel() override { upload->cancel(); }
    };

    Q_ASSERT(d->manager);

    auto state = std::make_shared<State>();
    state->upload = d->manager->uploadFile(
        std::move(data),
        info.filename().value_or(QXmppUtils::generateStanzaHash(10)),
        info.mediaType().value_or(QMimeDatabase().mimeTypeForName("application/octet-stream")),
        info.size() ? info.size().value() : -1);

    QObject::connect(state->upload.get(), &QXmppHttpUpload::finished, [state, reportFinished = std::move(reportFinished)](const QXmppHttpUpload::Result &result) mutable {
        reportFinished(visitForward<UploadResult>(result, [](QUrl url) {
            return std::any(QXmppHttpFileSource(std::move(url)));
        }));

        // reduce ref count, so the signal connection doesn't keep the state alive forever
        state.reset();
    });
    QObject::connect(state->upload.get(), &QXmppHttpUpload::progressChanged, [stateRef = std::weak_ptr<State>(state), reportProgress = std::move(reportProgress)]() {
        if (auto state = stateRef.lock()) {
            reportProgress(state->upload->bytesSent(), state->upload->bytesTotal());
        }
    });

    return std::dynamic_pointer_cast<QXmppFileSharingProvider::Upload>(state);
}
