// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDownload.h"

///
/// \class QXmppDownload
///
/// \brief Provides progress of stateless file sharing uploads.
///
/// \since QXmpp 1.5
///

///
/// \enum QXmppDownload::HashVerificationResult
///
/// Describes the result of the hash verification.
///

///
/// \struct QXmppDownload::Downloaded
///
/// Indicates that the file could be downloaded.
///

///
/// \var QXmppDownload::Downloaded::hashVerificationResult
///
/// Describes the result of the hash verification.
///

///
/// \typedef QXmppDownload::Result
///
/// \brief Contains QXmpp::Success (successfully finished), QXmpp::Cancelled (manually cancelled)
/// or QXmppError (an error occured while downloading).
///

///
/// \fn QXmppDownload::finished
///
/// Emitted when the download has finished.
///
