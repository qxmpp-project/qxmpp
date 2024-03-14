// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBINDERROR_H
#define QXMPPBINDERROR_H

#include "QXmppStanza.h"

namespace QXmpp {

///
/// Indicates a resource binding error
///
/// \since QXmpp 1.7
///
struct BindError {
    /// Stanza error returned by the server
    QXmppStanza::Error stanzaError;
};

}  // namespace QXmpp

#endif  // QXMPPBINDERROR_H
