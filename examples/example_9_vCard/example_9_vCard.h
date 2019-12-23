/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
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

public slots:
    void clientConnected();
    void rosterReceived();
    void vCardReceived(const QXmppVCardIq &);

private:
    QXmppRosterManager *m_rosterManager;
    QXmppVCardManager *m_vCardManager;
};

#endif  // XMPPCLIENT_H
