// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTANZA_P_H
#define QXMPPSTANZA_P_H

#include "QXmppStanza.h"

#include <optional>

//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API. It exists for the convenience
// of the QXmppStanza class.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

namespace QXmpp::Private {

// defined in QXmppStanza.cpp
auto conditionToString(QXmppStanza::Error::Condition condition) -> QString;
auto conditionFromString(const QString &string) -> std::optional<QXmppStanza::Error::Condition>;
auto typeToString(QXmppStanza::Error::Type type) -> QString;
auto typeFromString(const QString &string) -> std::optional<QXmppStanza::Error::Type>;

}

#endif
