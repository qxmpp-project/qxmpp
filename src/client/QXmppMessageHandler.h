// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMESSAGEHANDLER_H
#define QXMPPMESSAGEHANDLER_H

#include "QXmppExtension.h"
#include "QXmppMessage.h"

#include <variant>

#include <QFuture>

class QXMPP_EXPORT QXmppMessageHandler : public QXmppExtension
{
public:
    virtual bool handleMessage(const QXmppMessage &) = 0;
};

#endif  // QXMPPMESSAGEHANDLER_H
