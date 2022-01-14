// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
