// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPENCRYPTEDHTTPFILESHARINGPROVIDER_H
#define QXMPPENCRYPTEDHTTPFILESHARINGPROVIDER_H

#include "QXmppEncryptedFileSource.h"
#include "QXmppHttpFileSharingProvider.h"

class QXmppEncryptedHttpFileSharingProviderPrivate;

class QXMPP_EXPORT QXmppEncryptedHttpFileSharingProvider : public QXmppFileSharingProvider
{
public:
    /// \cond
    using SourceType = QXmppEncryptedFileSource;
    /// \endcond

    QXmppEncryptedHttpFileSharingProvider(QXmppClient *client, QNetworkAccessManager *netManager);
    ~QXmppEncryptedHttpFileSharingProvider() override;

    auto downloadFile(const std::any &source,
                      std::unique_ptr<QIODevice> &&target) -> std::shared_ptr<QXmppDownload> override;
    auto uploadFile(
        std::unique_ptr<QIODevice> data,
        const QXmppFileMetadata &info) -> std::shared_ptr<QXmppUpload> override;

private:
    std::unique_ptr<QXmppEncryptedHttpFileSharingProviderPrivate> d;
};

#endif  // QXMPPENCRYPTEDHTTPFILESHARINGPROVIDER_H
