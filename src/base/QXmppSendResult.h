// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSENDRESULT_H
#define QXMPPSENDRESULT_H

#include "QXmppError.h"

#include <variant>

namespace QXmpp {

///
/// Describes the type of a packet sending error.
///
/// \since QXmpp 1.5
///
enum class SendError : uint8_t {
    /// The packet was written to the socket with no success (only happens when Stream Management is disabled).
    SocketWriteError,
    /// The packet couldn't be sent because the connection hasn't been (re)established.
    Disconnected,
    /// The packet couldn't be sent because prior encryption failed.
    EncryptionError,
};

///
/// A struct indicating success when sending packets
///
/// \since QXmpp 1.5
///
struct SendSuccess
{
    /// Indicates whether the packet has been acknowledged by the other peer.
    bool acknowledged = false;
};

///
/// A variant containing either a SendSuccess object or a QXmppError.
///
/// The QXmppError will likely contain a SendError.
///
using SendResult = std::variant<SendSuccess, QXmppError>;

}  // namespace QXmpp

#endif  // QXMPPSENDRESULT_H
