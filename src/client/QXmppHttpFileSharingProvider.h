// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppFileSharingProvider.h"
#include "QXmppHttpFileSource.h"

class QXmppHttpUploadManager;
class QNetworkAccessManager;

class QXmppHttpFileSharingProviderPrivate;

class QXMPP_EXPORT QXmppHttpFileSharingProvider : public QXmppFileSharingProvider
{
public:
    /// \cond
    using SourceType = QXmppHttpFileSource;
    /// \endcond

    QXmppHttpFileSharingProvider(QXmppHttpUploadManager *manager, QNetworkAccessManager *netManager);
    ~QXmppHttpFileSharingProvider() override;

    auto downloadFile(const std::any &source,
                      std::unique_ptr<QIODevice> target,
                      std::function<void(quint64, quint64)> reportProgress,
                      std::function<void(DownloadResult)> reportFinished) -> std::shared_ptr<Download> override;
    auto uploadFile(std::unique_ptr<QIODevice> source,
                    const QXmppFileMetadata &info,
                    std::function<void(quint64, quint64)> reportProgress,
                    std::function<void(UploadResult)> reportFinished) -> std::shared_ptr<Upload> override;

private:
    std::unique_ptr<QXmppHttpFileSharingProviderPrivate> d;
};
