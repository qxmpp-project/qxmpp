/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPUPLOADREQUESTMANAGER_H
#define QXMPPUPLOADREQUESTMANAGER_H

#include <QXmppClientExtension.h>

#include <QSharedDataPointer>

class QFileInfo;
class QMimeType;
class QXmppHttpUploadRequestIq;
class QXmppHttpUploadSlotIq;
class QXmppUploadServicePrivate;
class QXmppUploadRequestManagerPrivate;

/// \brief QXmppUploadService represents an HTTP File Upload service.
///
/// It is used to store the JID and maximum file size for uploads.

class QXMPP_EXPORT QXmppUploadService
{
public:
    QXmppUploadService();
    QXmppUploadService(const QXmppUploadService &);
    ~QXmppUploadService();

    QXmppUploadService &operator=(const QXmppUploadService &);

    QString jid() const;
    void setJid(const QString &jid);

    qint64 sizeLimit() const;
    void setSizeLimit(qint64 sizeLimit);

private:
    QSharedDataPointer<QXmppUploadServicePrivate> d;
};

///
/// \brief The QXmppUploadRequestManager implements the core of \xep{0369}: HTTP
/// File Upload.
///
/// It handles the discovery of QXmppUploadServices and can send upload
/// requests and outputs the upload slots. It doesn't do the actual upload via.
/// HTTP.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// auto *manager = new QXmppUploadRequestManager;
/// client->addExtension(manager);
/// \endcode
///
/// Apart from that, you also need to discover HTTP File Upload service(s) by
/// requesting the Service Discovery info for each Service Discovery item of
/// the server. The QXmppUploadManager will then automatically recognize upload
/// services and add them to the list of discovered services
/// \c uploadServices().
///
/// Keep in mind that theoretically any XMPP entity could promote to be an
/// upload service and so is recognized by this manager. A potential attacker
/// could exploit this vulnerability, so the client could be uploading files to
/// the attacker (e.g. a normal user JID) instead of the own server.
///
/// As soon as at least one upload service has been discovered, you can start
/// to request upload slots by using \c requestUploadSlot(). Alternatively you
/// can provide the JID of the upload service which should be used for
/// uploading.
///
/// \since QXmpp 1.1
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppUploadRequestManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppUploadRequestManager();
    ~QXmppUploadRequestManager();

    QString requestUploadSlot(const QFileInfo &file,
                              const QString &uploadService = QString());
    QString requestUploadSlot(const QFileInfo &file,
                              const QString &customFileName,
                              const QString &uploadService = QString());
    QString requestUploadSlot(const QString &fileName,
                              qint64 fileSize,
                              const QMimeType &mimeType,
                              const QString &uploadService = QString());

    bool serviceFound() const;

    QVector<QXmppUploadService> uploadServices() const;

    bool handleStanza(const QDomElement &stanza) override;

Q_SIGNALS:
    /// Emitted when an upload slot was received.
    void slotReceived(const QXmppHttpUploadSlotIq &slot);

    /// Emitted when the slot request failed.
    ///
    /// \param request The sent IQ with an QXmppStanza::Error from the server.
    void requestFailed(const QXmppHttpUploadRequestIq &request);

    /// Emitted when the first upload service has been found.
    void serviceFoundChanged();

protected:
    void setClient(QXmppClient *client) override;

private:
    void handleDiscoInfo(const QXmppDiscoveryIq &iq);

    QSharedDataPointer<QXmppUploadRequestManagerPrivate> d;
};

#endif  // QXMPPUPLOADREQUESTMANAGER_H
