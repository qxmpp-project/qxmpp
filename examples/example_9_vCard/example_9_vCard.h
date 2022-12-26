// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include "QXmppClient.h"

class QXmppRosterManager;
class QXmppVCardIq;
class QXmppVCardManager;

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    xmppClient(QObject *parent = nullptr);
    ~xmppClient() override;

    void clientConnected();
    void rosterReceived();
    void vCardReceived(const QXmppVCardIq &);

private:
    QXmppRosterManager *m_rosterManager;
    QXmppVCardManager *m_vCardManager;
};

#endif  // XMPPCLIENT_H
