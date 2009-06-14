/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include <QObject>
#include "QXmppConfiguration.h"

class QXmppStream;
class QXmppPresence;
class QXmppMessage;
class QXmppPacket;
class QXmppIq;
class QXmppRoster;

class QXmppClient : public QObject
{
    Q_OBJECT

public:
    QXmppClient(QObject *parent = 0);
    ~QXmppClient();
    void connectToServer(const QString& host, const QString& user, const QString& passwd,
                         const QString& domain, int port = 5222);
    void connectToServer(const QXmppConfiguration&);
    void disconnect();
    QXmppRoster& getRoster();
    QXmppConfiguration& getConfigurgation();

signals:
    void messageReceived(const QXmppMessage&);
    void presenceReceived(const QXmppPresence&);
    void iqReceived(const QXmppIq&);

public slots:
    void sendPacket(const QXmppPacket&);

private:
    QXmppStream* m_stream;
    QXmppConfiguration m_config;
};

#endif // QXMPPCLIENT_H
