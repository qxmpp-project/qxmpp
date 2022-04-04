// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
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

}

#endif  // QXMPPGLOBAL_H
