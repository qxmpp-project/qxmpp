// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <stdint.h>

#include <QByteArray>

namespace QXmpp::Private {

QXMPP_EXPORT QByteArray generateRandomBytes(uint32_t minimumByteCount, uint32_t maximumByteCount);
QXMPP_EXPORT void generateRandomBytes(uint8_t *bytes, uint32_t byteCount);
float calculateProgress(qint64 transferred, qint64 total);

}  // namespace QXmpp::Private
