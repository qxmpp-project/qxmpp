// SPDX-FileCopyrightText: 2010 Ian Reinhart Geiser <geiseri@kde.org>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "rpcClient.h"

#include "QXmppRpcManager.h"
#include "QXmppUtils.h"

#include <QDebug>
#include <QTimer>

rpcClient::rpcClient(QObject *parent)
    : QXmppClient(parent)
{
    // add RPC manager
    m_rpcManager = new QXmppRpcManager;
    addExtension(m_rpcManager);

    // observe incoming presences
    connect(this, &QXmppClient::presenceReceived,
            this, &rpcClient::slotPresenceReceived);
}

rpcClient::~rpcClient()
{
}

void rpcClient::slotInvokeRemoteMethod()
{
    QXmppRemoteMethodResult methodResult = m_rpcManager->callRemoteMethod(
        m_remoteJid, "RemoteInterface.echoString", "This is a test");
    if (methodResult.hasError) {
        qDebug() << "Error:" << methodResult.code << methodResult.errorMessage;
    } else {
        qDebug() << "Result:" << methodResult.result;
    }
}

/// A presence was received.

void rpcClient::slotPresenceReceived(const QXmppPresence &presence)
{
    QStringView recipient = u"qxmpp.test1@qxmpp.org";

    // if we are the recipient, or if the presence is not from the recipient,
    // do nothing
    if (QXmppUtils::jidToBareJid(configuration().jid()) == recipient ||
        QXmppUtils::jidToBareJid(presence.from()) != recipient ||
        presence.type() != QXmppPresence::Available) {
        return;
    }

    // invoke the remote method in 1 second
    m_remoteJid = presence.from();
    QTimer::singleShot(1000, this, &rpcClient::slotInvokeRemoteMethod);
}
