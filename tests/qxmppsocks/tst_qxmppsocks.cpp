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

#include <QTcpSocket>
#include "QXmppSocks.h"
#include "util.h"

class tst_QXmppSocks : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void newConnectionSlot(QTcpSocket *socket, QString hostName, quint16 port);

    void testClientAndServer();
    void testServer();

private:
    QTcpSocket *m_connectionSocket;
    QString m_connectionHostName;
    quint16 m_connectionPort;
};

void tst_QXmppSocks::init()
{
    m_connectionSocket = 0;
    m_connectionHostName = QString();
    m_connectionPort = 0;
}

void tst_QXmppSocks::newConnectionSlot(QTcpSocket *socket, QString hostName, quint16 port)
{
    m_connectionSocket = socket;
    m_connectionHostName = hostName;
    m_connectionPort = port;
}

void tst_QXmppSocks::testClientAndServer()
{
    QXmppSocksServer server;
    QVERIFY(server.listen());
    QVERIFY(server.serverPort() != 0);
    connect(&server, SIGNAL(newConnection(QTcpSocket*,QString,quint16)),
            this, SLOT(newConnectionSlot(QTcpSocket*,QString,quint16)));

    QXmppSocksClient client("127.0.0.1", server.serverPort());

    QEventLoop loop;
    connect(&client, SIGNAL(ready()), &loop, SLOT(quit()));
  
    client.connectToHost("www.google.com", 80);
    loop.exec();

    // check server
    QVERIFY(m_connectionSocket);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionHostName, QLatin1String("www.google.com"));
    QCOMPARE(m_connectionPort, quint16(80));

    // disconnect
    client.disconnectFromHost();
}

void tst_QXmppSocks::testServer()
{
    QXmppSocksServer server;
    QVERIFY(server.listen());
    QVERIFY(server.serverPort() != 0);
    connect(&server, SIGNAL(newConnection(QTcpSocket*,QString,quint16)),
            this, SLOT(newConnectionSlot(QTcpSocket*,QString,quint16)));

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, server.serverPort());
    QVERIFY2(client.waitForConnected(), qPrintable(client.errorString()));

    QEventLoop loop;
    connect(&client, SIGNAL(readyRead()), &loop, SLOT(quit()));
  
    // SOCKS5, no authentication 
    client.write(QByteArray::fromHex("050100"));
    loop.exec();
    QCOMPARE(client.readAll(), QByteArray::fromHex("0500"));

    // request connect to www.google.com port 80
    client.write(QByteArray::fromHex("050100030e7777772e676f6f676c652e636f6d0050"));
    loop.exec();
    QCOMPARE(client.readAll(), QByteArray::fromHex("050000030e7777772e676f6f676c652e636f6d0050"));

    // check server
    QVERIFY(m_connectionSocket);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionHostName, QLatin1String("www.google.com"));
    QCOMPARE(m_connectionPort, quint16(80));

    // disconnect
    client.disconnectFromHost();
}

QTEST_MAIN(tst_QXmppSocks)
#include "tst_qxmppsocks.moc"
