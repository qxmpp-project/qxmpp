// SPDX-FileCopyrightText: 2009 Ian Geiser <ian.geiser@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "QXmppRpcManager.h"

class RemoteInterface : public QXmppInvokable
{
    Q_OBJECT
public:
    RemoteInterface(QObject *parent = nullptr);

    bool isAuthorized(const QString &jid) const override;

    // RPC Interface
public:
    Q_SLOT QString echoString(const QString &message);
};

#endif  // REMOTEINTERFACE_H
