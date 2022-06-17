// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPGLOBAL_P_H
#define QXMPPGLOBAL_P_H

#include "QXmppGlobal.h"

#include <optional>

namespace QXmpp::Private {

std::optional<Encryption> encryptionFromString(const QString &str);
QString encryptionToString(Encryption);
QString encryptionToName(Encryption);

}  // namespace QXmpp::Private

#endif  // QXMPPGLOBAL_P_H
