/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Ian Reinhart Geiser
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include <QDebug>
#include <QTimer>

#include "QXmppRemoteMethod.h"
#include "QXmppUtils.h"

#include "rpcClient.h"

rpcClient::rpcClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check = connect(this, SIGNAL(presenceReceived(QXmppPresence)),
                         this, SLOT(slotPresenceReceived(QXmppPresence)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

rpcClient::~rpcClient()
{
}

void rpcClient::slotInvokeRemoteMethod()
{
    QXmppRemoteMethodResult methodResult = callRemoteMethod(
            m_remoteJid, "RemoteInterface.echoString", "This is a test" );
    if( methodResult.hasError )
        qDebug() << "Error:" << methodResult.code << methodResult.errorMessage;
    else
        qDebug() << "Result:" << methodResult.result;
}

/// A presence was received.

void rpcClient::slotPresenceReceived(const QXmppPresence &presence)
{
    const QLatin1String recipient("qxmpp.test1@gmail.com");

    // if we are the recipient, or if the presence is not from the recipient,
    // do nothing
    if (jidToBareJid(configuration().jid()) == recipient ||
        jidToBareJid(presence.from()) != recipient ||
        presence.type() != QXmppPresence::Available)
        return;

    // invoke the remote method in 1 second
    m_remoteJid = presence.from();
    QTimer::singleShot(1000, this, SLOT(slotInvokeRemoteMethod()));
}

