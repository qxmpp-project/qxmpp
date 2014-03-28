/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *	Ian Reinhart Geiser
 *	Jeremy Lainé
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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

#include "QXmppRpcManager.h"
#include "QXmppUtils.h"

#include "rpcClient.h"

rpcClient::rpcClient(QObject *parent)
    : QXmppClient(parent)
{
    // add RPC manager
    m_rpcManager = new QXmppRpcManager;
    addExtension(m_rpcManager);

    // observe incoming presences
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
    QXmppRemoteMethodResult methodResult = m_rpcManager->callRemoteMethod(
            m_remoteJid, "RemoteInterface.echoString", "This is a test" );
    if( methodResult.hasError )
        qDebug() << "Error:" << methodResult.code << methodResult.errorMessage;
    else
        qDebug() << "Result:" << methodResult.result;
}

/// A presence was received.

void rpcClient::slotPresenceReceived(const QXmppPresence &presence)
{
    const QLatin1String recipient("qxmpp.test1@qxmpp.org");

    // if we are the recipient, or if the presence is not from the recipient,
    // do nothing
    if (QXmppUtils::jidToBareJid(configuration().jid()) == recipient ||
        QXmppUtils::jidToBareJid(presence.from()) != recipient ||
        presence.type() != QXmppPresence::Available)
        return;

    // invoke the remote method in 1 second
    m_remoteJid = presence.from();
    QTimer::singleShot(1000, this, SLOT(slotInvokeRemoteMethod()));
}

