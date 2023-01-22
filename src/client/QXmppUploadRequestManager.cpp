// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUploadRequestManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDataForm.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppHttpUploadIq.h"

#include <QDomElement>
#include <QFileInfo>
#include <QMimeDatabase>

using namespace QXmpp::Private;

class QXmppUploadServicePrivate : public QSharedData
{
public:
    QString jid;
    qint64 sizeLimit = -1;
};

QXmppUploadService::QXmppUploadService()
    : d(new QXmppUploadServicePrivate)
{
}

/// Copy constructor

QXmppUploadService::QXmppUploadService(const QXmppUploadService &) = default;

QXmppUploadService::~QXmppUploadService() = default;

/// Equal operator

QXmppUploadService &QXmppUploadService::operator=(const QXmppUploadService &) = default;

/// Returns the JID of the HTTP File Upload service.

QString QXmppUploadService::jid() const
{
    return d->jid;
}

/// Sets the JID of the HTTP File Upload service.

void QXmppUploadService::setJid(const QString &jid)
{
    d->jid = jid;
}

/// Returns the size limit of files that can be uploaded to this upload
/// service.
///
/// A size limit of -1 means that there is no file size limit or it could not
/// be determined.

qint64 QXmppUploadService::sizeLimit() const
{
    return d->sizeLimit;
}

/// Sets the size limit of files that can be uploaded to this upload service.

void QXmppUploadService::setSizeLimit(qint64 sizeLimit)
{
    d->sizeLimit = sizeLimit;
}

class QXmppUploadRequestManagerPrivate : public QSharedData
{
public:
    QVector<QXmppUploadService> uploadServices;
};

///
/// \typedef QXmppUploadRequestManager::SlotResult
///
/// Contains the requested upload slot from the service or a QXmppStanza::Error
/// in case the request failed.
///
/// \since QXmpp 1.5
///

QXmppUploadRequestManager::QXmppUploadRequestManager()
    : d(std::make_unique<QXmppUploadRequestManagerPrivate>())
{
}

QXmppUploadRequestManager::~QXmppUploadRequestManager() = default;

/// Requests an upload slot from the server.
///
/// \param file The info of the file to be uploaded.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
/// \return The id of the sent IQ. If sendPacket() isn't successful or no
///         upload service has been discovered yet, an empty string will be
///         returned.

QString QXmppUploadRequestManager::requestUploadSlot(const QFileInfo &file,
                                                     const QString &uploadService)
{
    return requestUploadSlot(file, file.fileName(), uploadService);
}

/// Requests an upload slot from the server.
///
/// \param file The info of the file to be uploaded.
/// \param customFileName This name is used instead of the file's actual name
///                       for requesting the upload slot.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
/// \return The id of the sent IQ. If sendPacket() isn't successful or no
///         upload service has been discovered yet, an empty string will be
///         returned.

QString QXmppUploadRequestManager::requestUploadSlot(const QFileInfo &file,
                                                     const QString &customFileName,
                                                     const QString &uploadService)
{
    return requestUploadSlot(customFileName, file.size(),
                             QMimeDatabase().mimeTypeForFile(file),
                             uploadService);
}

/// Requests an upload slot from the server.
///
/// \param fileName The name of the file to be uploaded. This may be used by
///                 the server to generate the URL.
/// \param fileSize The size of the file to be uploaded. The server may reject
///                 too large files.
/// \param mimeType The content-type of the file. This can be used by the
///                 server to set the HTTP MIME-type of the URL.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
/// \return The id of the sent IQ. If sendPacket() isn't successful or no
///         upload service has been discovered yet, an empty string will be
///         returned.

QString QXmppUploadRequestManager::requestUploadSlot(const QString &fileName,
                                                     qint64 fileSize,
                                                     const QMimeType &mimeType,
                                                     const QString &uploadService)
{
    if (!serviceFound() && uploadService.isEmpty()) {
        return {};
    }

    QXmppHttpUploadRequestIq iq;
    if (uploadService.isEmpty()) {
        iq.setTo(d->uploadServices.first().jid());
    } else {
        iq.setTo(uploadService);
    }
    iq.setType(QXmppIq::Get);
    iq.setFileName(fileName);
    iq.setSize(fileSize);
    iq.setContentType(mimeType);

    if (client()->sendPacket(iq)) {
        return iq.id();
    }
    return {};
}

///
/// Requests an upload slot from the server.
///
/// \param file The info of the file to be uploaded.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
auto QXmppUploadRequestManager::requestSlot(const QFileInfo &file,
                                            const QString &uploadService) -> QXmppTask<SlotResult>
{
    return requestSlot(file, file.fileName(), uploadService);
}

///
/// Requests an upload slot from the server.
///
/// \param file The info of the file to be uploaded.
/// \param customFileName This name is used instead of the file's actual name
///                       for requesting the upload slot.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
auto QXmppUploadRequestManager::requestSlot(const QFileInfo &file,
                                            const QString &customFileName,
                                            const QString &uploadService) -> QXmppTask<SlotResult>
{
    return requestSlot(customFileName, file.size(),
                       QMimeDatabase().mimeTypeForFile(file),
                       uploadService);
}

///
/// Requests an upload slot from the server.
///
/// \param fileName The name of the file to be uploaded. This may be used by
///                 the server to generate the URL.
/// \param fileSize The size of the file to be uploaded. The server may reject
///                 too large files.
/// \param mimeType The content-type of the file. This can be used by the
///                 server to set the HTTP MIME-type of the URL.
/// \param uploadService The HTTP File Upload service that is used to request
///                      the upload slot. If this is empty, the first
///                      discovered one is used.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
auto QXmppUploadRequestManager::requestSlot(const QString &fileName,
                                            qint64 fileSize,
                                            const QMimeType &mimeType,
                                            const QString &uploadService) -> QXmppTask<SlotResult>
{
    if (!serviceFound() && uploadService.isEmpty()) {
        return makeReadyTask(SlotResult(QXmppError {
            QStringLiteral("Couldn't request upload slot: No service found."), {} }));
    }

    QXmppHttpUploadRequestIq iq;
    if (uploadService.isEmpty()) {
        iq.setTo(d->uploadServices.first().jid());
    } else {
        iq.setTo(uploadService);
    }
    iq.setType(QXmppIq::Get);
    iq.setFileName(fileName);
    iq.setSize(fileSize);
    iq.setContentType(mimeType);

    return chainIq<SlotResult>(client()->sendIq(std::move(iq)), this);
}

/// Returns true if an HTTP File Upload service has been discovered.

bool QXmppUploadRequestManager::serviceFound() const
{
    return !d->uploadServices.isEmpty();
}

/// Returns all discovered HTTP File Upload services.

QVector<QXmppUploadService> QXmppUploadRequestManager::uploadServices() const
{
    return d->uploadServices;
}

bool QXmppUploadRequestManager::handleStanza(const QDomElement &element)
{
    if (QXmppHttpUploadSlotIq::isHttpUploadSlotIq(element)) {
        QXmppHttpUploadSlotIq slot;
        slot.parse(element);

        Q_EMIT slotReceived(slot);
        return true;
    } else if (QXmppHttpUploadRequestIq::isHttpUploadRequestIq(element)) {
        QXmppHttpUploadRequestIq requestError;
        requestError.parse(element);

        Q_EMIT requestFailed(requestError);
        return true;
    }
    return false;
}

void QXmppUploadRequestManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    if (!iq.features().contains(ns_http_upload)) {
        return;
    }

    const auto identities = iq.identities();
    for (const QXmppDiscoveryIq::Identity &identity : identities) {
        if (identity.category() == QStringLiteral("store") &&
            identity.type() == QStringLiteral("file")) {
            QXmppUploadService service;
            service.setJid(iq.from());

            // get size limit
            bool isFormNsCorrect = false;
            const auto fields = iq.form().fields();
            for (const QXmppDataForm::Field &field : fields) {
                if (field.key() == QStringLiteral("FORM_TYPE")) {
                    isFormNsCorrect = field.value() == ns_http_upload;
                } else if (isFormNsCorrect && field.key() == QStringLiteral("max-file-size")) {
                    service.setSizeLimit(field.value().toLongLong());
                }
            }

            d->uploadServices.append(service);
            Q_EMIT serviceFoundChanged();
        }
    }
    return;
}

void QXmppUploadRequestManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);
    // connect to service discovery manager
    auto *disco = client->findExtension<QXmppDiscoveryManager>();
    if (disco) {
        // scan info of all entities for upload services
        connect(disco, &QXmppDiscoveryManager::infoReceived,
                this, &QXmppUploadRequestManager::handleDiscoInfo);

        // on client disconnect remove all upload services
        connect(client, &QXmppClient::disconnected, this, [this]() {
            d->uploadServices.clear();
            Q_EMIT serviceFoundChanged();
        });
    }
}
