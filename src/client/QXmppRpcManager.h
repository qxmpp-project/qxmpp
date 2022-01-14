// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPRPCMANAGER_H
#define QXMPPRPCMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppInvokable.h"
#include "QXmppRemoteMethod.h"

#include <QMap>
#include <QVariant>

class QXmppRpcErrorIq;
class QXmppRpcInvokeIq;
class QXmppRpcResponseIq;

/// \brief The QXmppRpcManager class make it possible to invoke remote methods
/// and to expose local interfaces for remote procedure calls, as specified by
/// \xep{0009}: Jabber-RPC.
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

    void addInvokableInterface(QXmppInvokable *interface);
    QXmppRemoteMethodResult callRemoteMethod(const QString &jid,
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
                                             const QVariant &arg10 = QVariant());

    /// \cond
    QStringList discoveryFeatures() const override;
    QList<QXmppDiscoveryIq::Identity> discoveryIdentities() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

Q_SIGNALS:
    /// \cond
    void rpcCallResponse(const QXmppRpcResponseIq &result);
    void rpcCallError(const QXmppRpcErrorIq &err);
    /// \endcond

private:
    void invokeInterfaceMethod(const QXmppRpcInvokeIq &iq);

    QMap<QString, QXmppInvokable *> m_interfaces;
};

#endif
