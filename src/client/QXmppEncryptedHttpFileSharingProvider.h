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
                      std::unique_ptr<QIODevice> target,
                      std::function<void(quint64, quint64)> reportProgress,
                      std::function<void(DownloadResult)> reportFinished) -> std::shared_ptr<Download> override;

    auto uploadFile(std::unique_ptr<QIODevice> source,
                    const QXmppFileMetadata &info,
                    std::function<void(quint64, quint64)> reportProgress,
                    std::function<void(UploadResult)> reportFinished) -> std::shared_ptr<Upload> override;

private:
    std::unique_ptr<QXmppEncryptedHttpFileSharingProviderPrivate> d;
};

#endif  // QXMPPENCRYPTEDHTTPFILESHARINGPROVIDER_H
