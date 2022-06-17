// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPGLOBAL_H
#define QXMPPGLOBAL_H

#include "QXmppBuildConstants.h"

#if QXMPP_BUILD_SHARED
#if defined(QXMPP_BUILD)
#define QXMPP_EXPORT Q_DECL_EXPORT
#else
#define QXMPP_EXPORT Q_DECL_IMPORT
#endif
#else
#define QXMPP_EXPORT
#endif

#define QXMPP_AUTOTEST_EXPORT

// This sets which deprecated functions should still be usable
// It works exactly like QT_DISABLE_DEPRECATED_BEFORE
#ifndef QXMPP_DISABLE_DEPRECATED_BEFORE
#define QXMPP_DISABLE_DEPRECATED_BEFORE 0x0
#endif

// This works exactly like QT_DEPRECATED_SINCE, but checks QXMPP_DISABLE_DEPRECATED_BEFORE instead.
#define QXMPP_DEPRECATED_SINCE(major, minor) (QT_VERSION_CHECK(major, minor, 0) > QXMPP_DISABLE_DEPRECATED_BEFORE)

// workaround for Qt < 5.12
#ifndef Q_DECL_ENUMERATOR_DEPRECATED_X
#define Q_DECL_ENUMERATOR_DEPRECATED_X(msg)
#endif

#ifndef QT_WARNING_DISABLE_DEPRECATED
#define QT_WARNING_DISABLE_DEPRECATED
#endif

///
/// \namespace QXmpp
///
/// Contains global functions and enumerations.
///
/// \since QXmpp 1.5
///
namespace QXmpp {

///
/// This enum describes different end-to-end encryption methods. These can
/// be used to mark a stanza as encrypted or decrypted with a specific algorithm
/// (e.g., for \xep{0380, Explicit Message Encryption}).
///
/// \since QXmpp 1.5
///
enum Encryption {
    /// No encryption
    NoEncryption,
    /// Unknown encryption
    UnknownEncryption,
    /// \xep{0364, Current Off-the-Record Messaging Usage}
    Otr,
    /// \xep{0027, Current Jabber OpenPGP Usage}
    LegacyOpenPgp,
    /// \xep{0373, OpenPGP for XMPP}
    Ox,
    /// \xep{0384, OMEMO Encryption}
    Omemo,
    /// \xep{0384, OMEMO Encryption} since version 0.4
    Omemo1,
    /// \xep{0384, OMEMO Encryption} since version 0.8
    Omemo2,

// Keep in sync with namespaces and names in Global.cpp!

#if QXMPP_DEPRECATED_SINCE(1, 5)
    /// \xep{0364, Current Off-the-Record Messaging Usage}
    /// \deprecated This enum is deprecated since QXmpp 1.5. Use
    /// \c QXmpp::Otr instead.
    OTR = Otr,
    /// \xep{0027, Current Jabber OpenPGP Usage}
    /// \deprecated This enum is deprecated since QXmpp 1.5. Use
    /// \c QXmpp::LegacyOpenPgp instead.
    LegacyOpenPGP = LegacyOpenPgp,
    /// \xep{0373, OpenPGP for XMPP}
    /// \deprecated This enum is deprecated since QXmpp 1.5. Use
    /// \c QXmpp::Ox instead.
    OX = Ox,
    /// \xep{0384, OMEMO Encryption}
    /// \deprecated This enum is deprecated since QXmpp 1.5. Use
    /// \c QXmpp::Omemo instead.
    OMEMO = Omemo,
#endif
};

///
/// Parsing/serialization mode when using Stanza Content Encryption.
///
/// \sa \xep{0420, Stanza Content Encryption}
///
/// \since QXmpp 1.5
///
enum SceMode : uint8_t {
    SceAll,        ///< Processes all known elements.
    ScePublic,     ///< Only processes 'public' elements (e.g. needed for routing).
    SceSensitive,  ///< Only processes sensitive elements that should be encrypted.
};

///
/// Checks whether a mode is enabled.
///
/// When an SceMode is given you can use this to check whether Public or Private
/// elements are enabled.
///
/// \since QXmpp 1.5
///
inline constexpr bool operator&(SceMode mode1, SceMode mode2)
{
    return mode1 == SceAll || mode1 == mode2;
}

///
/// An empty struct indicating success in results.
///
/// \since QXmpp 1.5
///
struct Success
{
};

}  // namespace QXmpp

#endif  // QXMPPGLOBAL_H
