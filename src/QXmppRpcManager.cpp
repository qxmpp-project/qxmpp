/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "QXmppConstants.h"
#include "QXmppRpcIq.h"
#include "QXmppRpcManager.h"

/// Constructs a QXmppRpcManager.

QXmppRpcManager::QXmppRpcManager()
{
}

QStringList QXmppRpcManager::discoveryFeatures() const
{
    // XEP-0009: Jabber-RPC
    return QStringList() << ns_rpc;
}

bool QXmppRpcManager::handleStanza(const QDomElement &element)
{
    // XEP-0009: Jabber-RPC
    if (QXmppRpcInvokeIq::isRpcInvokeIq(element))
    {
        QXmppRpcInvokeIq rpcIqPacket;
        rpcIqPacket.parse(element);
        //emit rpcCallInvoke(rpcIqPacket);
        return true;
    }
    else if(QXmppRpcResponseIq::isRpcResponseIq(element))
    {
        QXmppRpcResponseIq rpcResponseIq;
        rpcResponseIq.parse(element);
        //emit rpcCallResponse(rpcResponseIq);
        return true;
    }
    else if(QXmppRpcErrorIq::isRpcErrorIq(element))
    {
        QXmppRpcErrorIq rpcErrorIq;
        rpcErrorIq.parse(element);
        //emit rpcCallError(rpcErrorIq);
        return true;
    }
    return false;
}

