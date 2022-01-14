// SPDX-FileCopyrightText: 2010 Ian Reinhart Geiser <geiseri@kde.org>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "QXmppClient.h"

class QXmppRpcManager;

class rpcClient : public QXmppClient
{
    Q_OBJECT

public:
    rpcClient(QObject *parent = nullptr);
    ~rpcClient() override;

private slots:
    void slotInvokeRemoteMethod();
    void slotPresenceReceived(const QXmppPresence &presence);

private:
    QString m_remoteJid;
    QXmppRpcManager *m_rpcManager;
};

#endif  // RPCCLIENT_H
