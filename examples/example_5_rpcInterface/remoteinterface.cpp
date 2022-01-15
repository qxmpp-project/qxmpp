// SPDX-FileCopyrightText: 2009 Ian Geiser <ian.geiser@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "remoteinterface.h"

RemoteInterface::RemoteInterface(QObject *parent) : QXmppInvokable(parent)
{
}

bool RemoteInterface::isAuthorized(const QString &jid) const
{
    Q_UNUSED(jid);
    return true;
}

QString RemoteInterface::echoString(const QString &message)
{
    return "Echo: " + message;
}
