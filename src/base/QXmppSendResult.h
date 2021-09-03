#ifndef QXMPPSENDRESULT_H
#define QXMPPSENDRESULT_H

#include "QXmppGlobal.h"

#include <variant>

namespace QXmpp {

///
/// A struct containing a packet send error type and error message.
///
/// \since QXmpp 1.5
///
struct SendError
{
    /// Describes the type of an error.
    enum Type : uint8_t {
        SocketWriteError, ///< The packet was written to the socket with no success (only happens when Stream Management is disabled).
        Disconnected,     ///< The packet couldn't be sent because the connection hasn't been (re)established.
        EncryptionError,  ///< The packet couldn't be sent because prior encryption failed.
    };

    /// Text describing the error.
    QString text;
    /// Type of the occured error.
    Type type;
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
/// A variant containing either a SendSuccess object or a SendError.
///
using SendResult = std::variant<SendSuccess, SendError>;

}

#endif // QXMPPSENDRESULT_H
