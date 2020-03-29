/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPCALLMANAGER_P_H
#define QXMPPCALLMANAGER_P_H

#include "QXmppCall.h"

#include <QHostAddress>
#include <QList>

class QXmppCallManager;

//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

class QXmppCallManagerPrivate
{
public:
    QXmppCallManagerPrivate(QXmppCallManager *qq);
    QXmppCall *findCall(const QString &sid) const;
    QXmppCall *findCall(const QString &sid, QXmppCall::Direction direction) const;

    QList<QXmppCall *> calls;
    QList<QPair<QHostAddress, quint16>> stunServers;
    QHostAddress turnHost;
    quint16 turnPort;
    QString turnUser;
    QString turnPassword;

private:
    QXmppCallManager *q;
};

#endif
