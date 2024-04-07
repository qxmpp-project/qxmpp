// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTLEVEL_H
#define QXMPPTRUSTLEVEL_H

#include <QFlags>
#include <QHashFunctions>

namespace QXmpp {

///
/// Trust level of public long-term keys used by end-to-end encryption
/// protocols
///
/// \since QXmpp 1.5
///
enum class TrustLevel {
    /// The key's trust is not decided.
    Undecided = 1,
    /// The key is automatically distrusted (e.g., by the security policy TOAKAFA).
    /// \see SecurityPolicy
    AutomaticallyDistrusted = 2,
    /// The key is manually distrusted (e.g., by clicking a button or \xep{0450, Automatic Trust
    /// Management (ATM)}).
    ManuallyDistrusted = 4,
    /// The key is automatically trusted (e.g., by the client for all keys of a bare JID until one
    /// of it is authenticated).
    AutomaticallyTrusted = 8,
    /// The key is manually trusted (e.g., by clicking a button).
    ManuallyTrusted = 16,
    /// The key is authenticated (e.g., by QR code scanning or \xep{0450, Automatic Trust
    /// Management (ATM)}).
    Authenticated = 32,
};

Q_DECLARE_FLAGS(TrustLevels, TrustLevel)
Q_DECLARE_OPERATORS_FOR_FLAGS(TrustLevels)

/// \cond
// Scoped enums (enum class) are not implicitly converted to int
inline uint qHash(QXmpp::TrustLevel key, uint seed) noexcept { return ::qHash(std::underlying_type_t<QXmpp::TrustLevel>(key), seed); }
/// \endcond

}  // namespace QXmpp

#endif  // QXMPPTRUSTLEVEL_H
