// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUpload.h"

///
/// \class QXmppUpload
///
/// \brief Provides progress of stateless file sharing uploads.
///
/// \since QXmpp 1.5
///

///
/// \class QXmppUpload::FileResult
///
/// \brief Contains QXmppFileShare of the uploaded file and possible data blobs containing
/// referenced thumbnails.
///

///
/// \var QXmppUpload::FileResult::fileShare
///
/// \brief File share with file metadata and file shares of the uploaded file.
///

///
/// \var QXmppUpload::FileResult::dataBlobs
///
/// \brief Data blobs of possibly in the metadata referenced thumbnails.
///
/// The QXmppFileSharingManager may generate file thumbnails.
///

///
/// \typedef QXmppUpload::Result
///
/// \brief Contains FileResult (successfully finished), QXmpp::Cancelled (manually cancelled)
/// or QXmppError (an error occured while downloading).
///

///
/// \fn QXmppUpload::finished
///
/// Emitted when the upload has finished.
///
