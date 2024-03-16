// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPAUTHENTICATIONERROR_H
#define QXMPPAUTHENTICATIONERROR_H

#include "QXmppGlobal.h"

#include <any>

namespace QXmpp {

///
/// Indicates an authentication error
///
/// \since QXmpp 1.7
///
struct AuthenticationError {
    /// Describes the type of the authentication error.
    enum Type {
        /// The provided credentials have been rejected by the server.
        NotAuthorized,
        /// The server did not allow authentication because the account is currently disabled.
        AccountDisabled,
        /// Used credentials are not valid anymore.
        CredentialsExpired,
        /// Authentication is only allowed with an encrypted connection.
        EncryptionRequired,
        /// Authenticated could not be completed because the server did not offer a compatible
        /// authentication mechanism.
        MechanismMismatch,
        /// Local error while processing authentication challenges
        ProcessingError,
        /// The server requested for tasks that are not supported
        RequiredTasks,
    };
    /// Type of the authentication error
    Type type;
    /// Error message from the server
    QString text;
    /// Protocol-specific details provided to the error
    std::any details;
};

}  // namespace QXmpp

#endif  // QXMPPAUTHENTICATIONERROR_H
