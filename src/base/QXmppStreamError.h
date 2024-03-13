// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTREAMERROR_H
#define QXMPPSTREAMERROR_H

namespace QXmpp {

///
/// All XMPP top-level stream errors
///
/// \since QXmpp 1.7
///
enum class StreamError {
    BadFormat,
    BadNamespacePrefix,
    Conflict,
    ConnectionTimeout,
    HostGone,
    HostUnknown,
    ImproperAddressing,
    InternalServerError,
    InvalidFrom,
    InvalidNamespace,
    InvalidXml,
    NotAuthorized,
    NotWellFormed,
    PolicyViolation,
    RemoteConnectionFailed,
    Reset,
    ResourceConstraint,
    RestrictedXml,
    SystemShutdown,
    UndefinedCondition,
    UnsupportedEncoding,
    UnsupportedFeature,
    UnsupportedStanzaType,
    UnsupportedVersion,
};

}  // namespace QXmpp

#endif  // QXMPPSTREAMERROR_H
