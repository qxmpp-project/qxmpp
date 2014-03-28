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

#ifndef QXMPPRPCMANAGER_H
#define QXMPPRPCMANAGER_H

#include <QMap>
#include <QVariant>

#include "QXmppClientExtension.h"
#include "QXmppInvokable.h"
#include "QXmppRemoteMethod.h"

class QXmppRpcErrorIq;
class QXmppRpcInvokeIq;
class QXmppRpcResponseIq;

/// \brief The QXmppRpcManager class make it possible to invoke remote methods
/// and to expose local interfaces for remote procedure calls, as specified by
/// XEP-0009: Jabber-RPC.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppRpcManager *manager = new QXmppRpcManager;
/// client->addExtension(manager);
/// \endcode
///
/// \note THIS API IS NOT FINALIZED YET
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppRpcManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppRpcManager();

    void addInvokableInterface( QXmppInvokable *interface );
    QXmppRemoteMethodResult callRemoteMethod( const QString &jid,
                                              const QString &interface,
                                              const QVariant &arg1 = QVariant(),
                                              const QVariant &arg2 = QVariant(),
                                              const QVariant &arg3 = QVariant(),
                                              const QVariant &arg4 = QVariant(),
                                              const QVariant &arg5 = QVariant(),
                                              const QVariant &arg6 = QVariant(),
                                              const QVariant &arg7 = QVariant(),
                                              const QVariant &arg8 = QVariant(),
                                              const QVariant &arg9 = QVariant(),
                                              const QVariant &arg10 = QVariant() );

    /// \cond
    QStringList discoveryFeatures() const;
    virtual QList<QXmppDiscoveryIq::Identity> discoveryIdentities() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// \cond
    void rpcCallResponse(const QXmppRpcResponseIq& result);
    void rpcCallError(const QXmppRpcErrorIq &err);
    /// \endcond

private:
    void invokeInterfaceMethod(const QXmppRpcInvokeIq &iq);

    QMap<QString,QXmppInvokable*> m_interfaces;
};

#endif
