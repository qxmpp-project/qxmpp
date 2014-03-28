/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppInvokable.h"
#include "QXmppRemoteMethod.h"
#include "QXmppRpcIq.h"
#include "QXmppRpcManager.h"

/// Constructs a QXmppRpcManager.

QXmppRpcManager::QXmppRpcManager()
{
}

/// Adds a local interface which can be queried using RPC.
///
/// \param interface

void QXmppRpcManager::addInvokableInterface( QXmppInvokable *interface )
{
    m_interfaces[ interface->metaObject()->className() ] = interface;
}

/// Invokes a remote interface using RPC.
///
/// \param iq

void QXmppRpcManager::invokeInterfaceMethod( const QXmppRpcInvokeIq &iq )
{
    QXmppStanza::Error error;

    const QStringList methodBits = iq.method().split('.');
    if (methodBits.size() != 2)
        return;
    const QString interface = methodBits.first();
    const QString method = methodBits.last();
    QXmppInvokable *iface = m_interfaces.value(interface);
    if (iface)
    {
        if ( iface->isAuthorized( iq.from() ) )
        {

            if ( iface->interfaces().contains(method) )
            {
                QVariant result = iface->dispatch(method.toLatin1(),
                                                  iq.arguments() );
                QXmppRpcResponseIq resultIq;
                resultIq.setId(iq.id());
                resultIq.setTo(iq.from());
                resultIq.setValues(QVariantList() << result);
                client()->sendPacket( resultIq );
                return;
            }
            else
            {
                error.setType(QXmppStanza::Error::Cancel);
                error.setCondition(QXmppStanza::Error::ItemNotFound);
            }
        }
        else
        {
            error.setType(QXmppStanza::Error::Auth);
            error.setCondition(QXmppStanza::Error::Forbidden);
        }
    }
    else
    {
        error.setType(QXmppStanza::Error::Cancel);
        error.setCondition(QXmppStanza::Error::ItemNotFound);
    }
    QXmppRpcErrorIq errorIq;
    errorIq.setId(iq.id());
    errorIq.setTo(iq.from());
    errorIq.setQuery(iq);
    errorIq.setError(error);
    client()->sendPacket(errorIq);
}

/// Calls a remote method using RPC with the specified arguments.
///
/// \note This method blocks until the response is received, and it may
/// cause XMPP stanzas to be lost!

QXmppRemoteMethodResult QXmppRpcManager::callRemoteMethod( const QString &jid,
                                          const QString &interface,
                                          const QVariant &arg1,
                                          const QVariant &arg2,
                                          const QVariant &arg3,
                                          const QVariant &arg4,
                                          const QVariant &arg5,
                                          const QVariant &arg6,
                                          const QVariant &arg7,
                                          const QVariant &arg8,
                                          const QVariant &arg9,
                                          const QVariant &arg10 )
{
    QVariantList args;
    if( arg1.isValid() ) args << arg1;
    if( arg2.isValid() ) args << arg2;
    if( arg3.isValid() ) args << arg3;
    if( arg4.isValid() ) args << arg4;
    if( arg5.isValid() ) args << arg5;
    if( arg6.isValid() ) args << arg6;
    if( arg7.isValid() ) args << arg7;
    if( arg8.isValid() ) args << arg8;
    if( arg9.isValid() ) args << arg9;
    if( arg10.isValid() ) args << arg10;

    QXmppRemoteMethod method( jid, interface, args, client() );
    connect(this, SIGNAL(rpcCallResponse(QXmppRpcResponseIq)),
            &method, SLOT(gotResult(QXmppRpcResponseIq)));
    connect(this, SIGNAL(rpcCallError(QXmppRpcErrorIq)),
            &method, SLOT(gotError(QXmppRpcErrorIq)));

    return method.call();
}

/// \cond
QStringList QXmppRpcManager::discoveryFeatures() const
{
    // XEP-0009: Jabber-RPC
    return QStringList() << ns_rpc;
}

QList<QXmppDiscoveryIq::Identity> QXmppRpcManager::discoveryIdentities() const
{
    QXmppDiscoveryIq::Identity identity;
    identity.setCategory("automation");
    identity.setType("rpc");
    return QList<QXmppDiscoveryIq::Identity>() << identity;
}

bool QXmppRpcManager::handleStanza(const QDomElement &element)
{
    // XEP-0009: Jabber-RPC
    if (QXmppRpcInvokeIq::isRpcInvokeIq(element))
    {
        QXmppRpcInvokeIq rpcIqPacket;
        rpcIqPacket.parse(element);
        invokeInterfaceMethod(rpcIqPacket);
        return true;
    }
    else if(QXmppRpcResponseIq::isRpcResponseIq(element))
    {
        QXmppRpcResponseIq rpcResponseIq;
        rpcResponseIq.parse(element);
        emit rpcCallResponse(rpcResponseIq);
        return true;
    }
    else if(QXmppRpcErrorIq::isRpcErrorIq(element))
    {
        QXmppRpcErrorIq rpcErrorIq;
        rpcErrorIq.parse(element);
        emit rpcCallError(rpcErrorIq);
        return true;
    }
    return false;
}
/// \endcond
