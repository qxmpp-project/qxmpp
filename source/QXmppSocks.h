/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
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

#ifndef QXMPPSOCKS_H
#define QXMPPSOCKS_H

#include <QHostAddress>
#include <QObject>

class QTcpServer;
class QTcpSocket;

class QXmppSocksClient : public QObject
{
    Q_OBJECT

public:
    QXmppSocksClient(const QHostAddress &proxyAddress, quint16 proxyPort, QObject *parent=0);
    void close();
    void connectToHost(const QString &hostName, quint16 hostPort);
    QString errorString() const;
    QByteArray readAll();
    bool waitForConnected(int msecs = 30000);

signals:
    void connected();
    void disconnected();
    void readyRead();

private slots:
    void slotConnected();
    void slotReadyRead();

private:
    QHostAddress m_proxyAddress;
    quint16 m_proxyPort;
    QString m_hostName;
    quint16 m_hostPort;
    QTcpSocket *m_socket;
    int m_step;
};

class QXmppSocksServer : public QObject
{
    Q_OBJECT

public:
    QXmppSocksServer(QObject *parent=0);
    void close();
    bool listen(const QHostAddress &address, quint16 port = 0);
    QHostAddress serverAddress() const;
    quint16 serverPort() const;
    void setHostName(const QString &hostName);
    void setHostPort(quint16 hostPort);
    void write(const QByteArray &data);

signals:
    void bytesWritten(qint64);
    void disconnected();

private slots:
    void slotNewConnection();
    void slotReadyRead();

private:
    QString m_hostName;
    quint16 m_hostPort;
    QTcpServer *m_server;
    QTcpSocket *m_socket;
    int m_step;
};

#endif
