// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILESHARINGPROVIDER_H
#define QXMPPFILESHARINGPROVIDER_H

#include "QXmppGlobal.h"

#include <any>
#include <memory>

class QIODevice;
class QXmppFileMetadata;
class QXmppUpload;
class QXmppDownload;

///
/// \brief The interface of a provider for the FileSharingManager
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
    /// \cond
    virtual ~QXmppFileSharingProvider() = default;
    /// \endcond

    ///
    /// \brief Handles the download of files for this provider
    /// \param source A type-erased source object. The provider will only ever have to handle its own sources,
    ///         so this can be safely casted to a concrete type.
    /// \param target QIODevice into which the received data should be written
    /// \return A subclass of QXmppDownload
    ///
    virtual auto downloadFile(const std::any &source, std::unique_ptr<QIODevice> &&target) -> std::shared_ptr<QXmppDownload> = 0;

    ///
    /// \brief Handles the upload of a file for this provider
    /// \param source A QIODevice from which data for uploading should be read.
    /// \param info Metadata of the file
    /// \return A subclass of QXmppUpload
    ///
    virtual auto uploadFile(std::unique_ptr<QIODevice> source, const QXmppFileMetadata &info) -> std::shared_ptr<QXmppUpload> = 0;
};

#endif  // QXMPPFILESHARINGPROVIDER_H
