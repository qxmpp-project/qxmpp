// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppError.h"
#include "QXmppGlobal.h"

#include <any>
#include <functional>
#include <memory>
#include <variant>

class QIODevice;
class QXmppFileMetadata;
class QXmppUpload;
class QXmppDownload;

///
/// \brief The interface of a provider for the QXmppFileSharingManager
///
/// To use it, implement all the pure virtual functions,
/// and add a using declaration for the type of source you want to handle.
/// ```
/// using SourceType = QXmppHttpFileSource;
/// ```
///
class QXMPP_EXPORT QXmppFileSharingProvider
{
public:
    /// Contains QXmpp::Success (successfully finished), QXmpp::Cancelled (manually cancelled) or
    /// QXmppError (an error occured while downloading).
    using DownloadResult = std::variant<QXmpp::Success, QXmpp::Cancelled, QXmppError>;

    /// Contains std::any (created file source), QXmpp::Cancelled (manually cancelled) or
    /// QXmppError (an error occured while uploading).
    using UploadResult = std::variant<std::any /* source */, QXmpp::Cancelled, QXmppError>;

    /// Used to control ongoing downloads
    class Download
    {
    public:
        virtual ~Download() = default;
        /// Cancels the download.
        virtual void cancel() = 0;
    };

    /// Used to control ongoing uploads
    class Upload
    {
    public:
        virtual ~Upload() = default;
        /// Cancels the upload.
        virtual void cancel() = 0;
    };

    /// \cond
    virtual ~QXmppFileSharingProvider() = default;
    /// \endcond

    ///
    /// \brief Handles the download of files for this provider
    /// \param source A type-erased source object. The provider will only ever have to handle
    ///        its own sources, so this can safely be casted to the defined source type.
    /// \param target QIODevice into which the received data should be written
    /// \param reportProgress Can be called to report received bytes and total bytes
    /// \param reportFinished Finalizes the download, no more progress must be reported after this
    ///
    virtual auto downloadFile(const std::any &source,
                              std::unique_ptr<QIODevice> target,
                              std::function<void(quint64, quint64)> reportProgress,
                              std::function<void(DownloadResult)> reportFinished) -> std::shared_ptr<Download> = 0;

    ///
    /// \brief Handles the upload of a file for this provider
    /// \param source A QIODevice from which data for uploading should be read.
    /// \param info Metadata of the file
    /// \param reportProgress Can be called to report sent bytes and total bytes
    /// \param reportFinished Finalizes the upload, no more progress must be reported after this
    ///
    virtual auto uploadFile(std::unique_ptr<QIODevice> source,
                            const QXmppFileMetadata &info,
                            std::function<void(quint64, quint64)> reportProgress,
                            std::function<void(UploadResult)> reportFinished) -> std::shared_ptr<Upload> = 0;
};
