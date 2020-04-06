/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#ifndef QXMPPCLIENT_P_H
#define QXMPPCLIENT_P_H

#include "QXmppPresence.h"

class QXmppClient;
class QXmppClientExtension;
class QXmppLogger;
class QXmppOutgoingClient;
class QTimer;

class QXmppClientPrivate
{
public:
    QXmppClientPrivate(QXmppClient *qq);

    /// Current presence of the client
    QXmppPresence clientPresence;
    QList<QXmppClientExtension *> extensions;
    QXmppLogger *logger;
    /// Pointer to the XMPP stream
    QXmppOutgoingClient *stream;

    // reconnection
    bool receivedConflict;
    int reconnectionTries;
    QTimer *reconnectionTimer;

    // Client state indication
    bool isActive;

    void addProperCapability(QXmppPresence &presence);
    int getNextReconnectTime() const;

    static QStringList discoveryFeatures();

private:
    QXmppClient *q;
};

#endif  // QXMPPCLIENT_P_H
