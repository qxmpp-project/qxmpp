// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include "QXmppClient.h"

class echoClient : public QXmppClient
{
    Q_OBJECT

public:
    echoClient(QObject *parent = nullptr);
    ~echoClient() override;

    void messageReceived(const QXmppMessage &);
};

#endif  // ECHOCLIENT_H
