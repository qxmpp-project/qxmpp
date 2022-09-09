// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPHTTPFILESHARINGPROVIDER_H
#define QXMPPHTTPFILESHARINGPROVIDER_H

#include "QXmppFileSharingProvider.h"
#include "QXmppHttpFileSource.h"

#include <any>
#include <memory>

class QXmppClient;
class QIODevice;
class QXmppHttpUploadManager;
class QNetworkAccessManager;

class QXmppHttpFileSharingProviderPrivate;

class QXMPP_EXPORT QXmppHttpFileSharingProvider : public QXmppFileSharingProvider
{
public:
    /// \cond
    using SourceType = QXmppHttpFileSource;
    /// \endcond

    QXmppHttpFileSharingProvider(QXmppClient *client, QNetworkAccessManager *netManager);
    ~QXmppHttpFileSharingProvider() override;

    auto downloadFile(const std::any &source,
                      std::unique_ptr<QIODevice> &&target) -> std::shared_ptr<QXmppDownload> override;
    auto uploadFile(std::unique_ptr<QIODevice> data,
                    const QXmppFileMetadata &info) -> std::shared_ptr<QXmppUpload> override;

private:
    std::unique_ptr<QXmppHttpFileSharingProviderPrivate> d;
};

#endif  // QXMPPHTTPFILESHARINGPROVIDER_H
