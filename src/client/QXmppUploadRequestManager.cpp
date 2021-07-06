/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

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
    : d(new QXmppUploadRequestManagerPrivate)
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
    if (!serviceFound() && uploadService.isEmpty())
        return {};

    QXmppHttpUploadRequestIq iq;
    if (uploadService.isEmpty())
        iq.setTo(d->uploadServices.first().jid());
    else
        iq.setTo(uploadService);
    iq.setType(QXmppIq::Get);
    iq.setFileName(fileName);
    iq.setSize(fileSize);
    iq.setContentType(mimeType);

    if (client()->sendPacket(iq))
        return iq.id();
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
                                            const QString &uploadService) -> QFuture<SlotResult>
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
                                            const QString &uploadService) -> QFuture<SlotResult>
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
                                            const QString &uploadService) -> QFuture<SlotResult>
{
    if (!serviceFound() && uploadService.isEmpty()) {
        using Error = QXmppStanza::Error;
        const auto errorMessage = QStringLiteral("Couldn't request upload slot: No service found.");
        return makeReadyFuture(SlotResult(Error(Error::Cancel, Error::FeatureNotImplemented, errorMessage)));
    }

    QXmppHttpUploadRequestIq iq;
    if (uploadService.isEmpty())
        iq.setTo(d->uploadServices.first().jid());
    else
        iq.setTo(uploadService);
    iq.setType(QXmppIq::Get);
    iq.setFileName(fileName);
    iq.setSize(fileSize);
    iq.setContentType(mimeType);

    return chainIq<SlotResult>(client()->sendIq(iq), this);
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

        emit slotReceived(slot);
        return true;
    } else if (QXmppHttpUploadRequestIq::isHttpUploadRequestIq(element)) {
        QXmppHttpUploadRequestIq requestError;
        requestError.parse(element);

        emit requestFailed(requestError);
        return true;
    }
    return false;
}

void QXmppUploadRequestManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    if (!iq.features().contains(ns_http_upload))
        return;

    const auto identities = iq.identities();
    for (const QXmppDiscoveryIq::Identity &identity : identities) {
        if (identity.category() == QStringLiteral("store") &&
            identity.type() == QStringLiteral("file")) {
            QXmppUploadService service;
            service.setJid(iq.from());

            // get size limit
            bool isFormNsCorrect = false;
            const auto fields = iq.form().constFields();
            for (const auto &field : fields) {
                if (field.key() == QStringLiteral("FORM_TYPE")) {
                    isFormNsCorrect = field.value() == ns_http_upload;
                } else if (isFormNsCorrect && field.key() == QStringLiteral("max-file-size")) {
                    service.setSizeLimit(field.value().toLongLong());
                }
            }

            d->uploadServices.append(service);
            emit serviceFoundChanged();
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
        connect(client, &QXmppClient::disconnected, [=]() {
            d->uploadServices.clear();
            emit serviceFoundChanged();
        });
    }
}
