// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFileTransfer.h"

///
/// \class QXmppFileTransfer
///
/// \brief Provides progress information about ongoing file uploads and downloads.
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppFileTransfer::cancel()
///
/// \brief Cancels the file transfer. finished() will be emitted.
///

///
/// \fn QXmppFileTransfer::isFinished()
///
/// \brief Returns whether the file transfer is finished.
///

///
/// \fn QXmppFileTransfer::bytesTransferred()
///
/// \brief Returns the number of bytes that have been uploaded or downloaded.
///

///
/// \fn QXmppFileTransfer::bytesTotal()
///
/// \brief Returns the number of bytes that totally need to be transferred.
///

///
/// \fn QXmppFileTransfer::progressChanged()
///
/// \brief Emitted when new bytes have been transferred.
///
