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

#ifndef QXMPPSOCKS_H
#define QXMPPSOCKS_H

#include <QHostAddress>
#include <QTcpSocket>

#include "QXmppGlobal.h"

class QTcpServer;

class QXMPP_EXPORT QXmppSocksClient : public QTcpSocket
{
    Q_OBJECT

public:
    QXmppSocksClient(const QString &proxyHost, quint16 proxyPort, QObject *parent=0);
    void connectToHost(const QString &hostName, quint16 hostPort);

signals:
    void ready();

private slots:
    void slotConnected();
    void slotReadyRead();

private:
    QString m_proxyHost;
    quint16 m_proxyPort;
    QString m_hostName;
    quint16 m_hostPort;
    int m_step;
};

class QXMPP_EXPORT QXmppSocksServer : public QObject
{
    Q_OBJECT

public:
    QXmppSocksServer(QObject *parent=0);
    void close();
    bool listen(quint16 port = 0);

    quint16 serverPort() const;

signals:
    void newConnection(QTcpSocket *socket, QString hostName, quint16 port);

private slots:
    void slotNewConnection();
    void slotReadyRead();

private:
    QTcpServer *m_server;
    QTcpServer *m_server_v6;
    QMap<QTcpSocket*, int> m_states;
};

#endif
