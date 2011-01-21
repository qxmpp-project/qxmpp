/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPRECONNECTIONMANAGER_H
#define QXMPPRECONNECTIONMANAGER_H

#include <QObject>
#include <QTimer>
#include "QXmppClient.h"

class QXmppReconnectionManager : public QObject
{
    Q_OBJECT

public:
    QXmppReconnectionManager(QXmppClient* client);

signals:
    void reconnectingIn(int);
    void reconnectingNow();

public slots:
    void cancelReconnection();

private slots:
    void connected();
    void error(QXmppClient::Error);
    void reconnect();

private:
    int getNextReconnectingInTime();
    bool m_receivedConflict;
    int m_reconnectionTries;
    QTimer m_timer;

    // reference to to client object (no ownership)
    QXmppClient* m_client;
};

#endif // QXMPPRECONNECTIONMANAGER_H
