// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSOCKS_H
#define QXMPPSOCKS_H

#include "QXmppGlobal.h"

#include <QHostAddress>
#include <QTcpSocket>

class QTcpServer;

class QXMPP_EXPORT QXmppSocksClient : public QTcpSocket
{
    Q_OBJECT

public:
    QXmppSocksClient(const QString &proxyHost, quint16 proxyPort, QObject *parent = nullptr);
    void connectToHost(const QString &hostName, quint16 hostPort);

Q_SIGNALS:
    void ready();

private Q_SLOTS:
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
    QXmppSocksServer(QObject *parent = nullptr);
    void close();
    bool listen(quint16 port = 0);

    quint16 serverPort() const;

Q_SIGNALS:
    void newConnection(QTcpSocket *socket, QString hostName, quint16 port);

private Q_SLOTS:
    void slotNewConnection();
    void slotReadyRead();

private:
    QTcpServer *m_server;
    QTcpServer *m_server_v6;
    QMap<QTcpSocket *, int> m_states;
};

#endif
