// SPDX-FileCopyrightText: 2015 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppSocks.h"

#include "util.h"
#include <QTcpServer>
#include <QTcpSocket>

class tst_QXmppSocks : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void init();
    Q_SLOT void newConnectionSlot(QTcpSocket *socket, QString hostName, quint16 port);

    Q_SLOT void testClient_data();
    Q_SLOT void testClient();
    Q_SLOT void testClientAndServer();
    Q_SLOT void testServer_data();
    Q_SLOT void testServer();

    QTcpSocket *m_connectionSocket;
    QString m_connectionHostName;
    quint16 m_connectionPort;
};

void tst_QXmppSocks::init()
{
    m_connectionSocket = nullptr;
    m_connectionHostName = QString();
    m_connectionPort = 0;
}

void tst_QXmppSocks::newConnectionSlot(QTcpSocket *socket, QString hostName, quint16 port)
{
    m_connectionSocket = socket;
    m_connectionHostName = hostName;
    m_connectionPort = port;
}

void tst_QXmppSocks::testClient_data()
{
    QTest::addColumn<QByteArray>("serverHandshake");
    QTest::addColumn<bool>("serverHandshakeWorks");
    QTest::addColumn<QByteArray>("serverConnect");
    QTest::addColumn<bool>("serverConnectWorks");
    QTest::addColumn<QByteArray>("clientReceivedData");

    QTest::newRow("no authentication - good connect")
        << QByteArray::fromHex("0500") << true
        << QByteArray::fromHex("050000030e7777772e676f6f676c652e636f6d0050") << true
        << QByteArray();
    QTest::newRow("no authentication - good connect and data")
        << QByteArray::fromHex("0500") << true
        << QByteArray::fromHex("050000030e7777772e676f6f676c652e636f6d0050001122") << true
        << QByteArray::fromHex("001122");

    QTest::newRow("no authentication - bad connect")
        << QByteArray::fromHex("0500") << true
        << QByteArray::fromHex("0500") << false
        << QByteArray();
    QTest::newRow("bad authentication")
        << QByteArray::fromHex("05ff") << false
        << QByteArray() << false
        << QByteArray();
}

void tst_QXmppSocks::testClient()
{
    QFETCH(QByteArray, serverHandshake);
    QFETCH(bool, serverHandshakeWorks);
    QFETCH(QByteArray, serverConnect);
    QFETCH(bool, serverConnectWorks);
    QFETCH(QByteArray, clientReceivedData);

    QTcpServer server;
    QVERIFY(server.listen());
    QVERIFY(server.serverPort() != 0);

    QXmppSocksClient client("127.0.0.1", server.serverPort());

    QEventLoop loop;
    connect(&server, &QTcpServer::newConnection, &loop, &QEventLoop::quit);

    client.connectToHost("www.google.com", 80);
    loop.exec();

    // receive client handshake
    m_connectionSocket = server.nextPendingConnection();
    QVERIFY(m_connectionSocket);

    connect(m_connectionSocket, &QAbstractSocket::disconnected, &loop, &QEventLoop::quit);
    connect(m_connectionSocket, &QIODevice::readyRead, &loop, &QEventLoop::quit);
    loop.exec();
    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionSocket->readAll(), QByteArray::fromHex("050100"));

    // receive client connect
    m_connectionSocket->write(serverHandshake);
    loop.exec();
    if (!serverHandshakeWorks) {
        QCOMPARE(client.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(m_connectionSocket->state(), QAbstractSocket::UnconnectedState);
        return;
    }

    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionSocket->readAll(), QByteArray::fromHex("050100030e7777772e676f6f676c652e636f6d0050"));

    // wait for client to be ready
    connect(&client, &QXmppSocksClient::ready, &loop, &QEventLoop::quit);
    m_connectionSocket->write(serverConnect);
    loop.exec();
    if (!serverConnectWorks) {
        QCOMPARE(client.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(m_connectionSocket->state(), QAbstractSocket::UnconnectedState);
        return;
    }

    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QByteArray received = client.readAll();
    QCOMPARE(received, clientReceivedData);

    // disconnect
    client.disconnectFromHost();
}

void tst_QXmppSocks::testClientAndServer()
{
    QXmppSocksServer server;
    QVERIFY(server.listen());
    QVERIFY(server.serverPort() != 0);
    connect(&server, &QXmppSocksServer::newConnection,
            this, &tst_QXmppSocks::newConnectionSlot);

    QXmppSocksClient client("127.0.0.1", server.serverPort());

    QEventLoop loop;
    connect(&client, &QXmppSocksClient::ready, &loop, &QEventLoop::quit);

    client.connectToHost("www.google.com", 80);
    loop.exec();

    // check client
    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);

    // check server
    QVERIFY(m_connectionSocket);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionHostName, QLatin1String("www.google.com"));
    QCOMPARE(m_connectionPort, quint16(80));

    // disconnect
    client.disconnectFromHost();
}

void tst_QXmppSocks::testServer_data()
{
    QTest::addColumn<QByteArray>("clientHandshake");
    QTest::addColumn<bool>("clientHandshakeWorks");
    QTest::addColumn<QByteArray>("clientConnect");
    QTest::addColumn<bool>("clientConnectWorks");

    QTest::newRow("no authentication - connect to www.google.com:80")
        << QByteArray::fromHex("050100") << true
        << QByteArray::fromHex("050100030e7777772e676f6f676c652e636f6d0050") << true;
    QTest::newRow("no authentication - bad connect")
        << QByteArray::fromHex("050100") << true
        << QByteArray::fromHex("0500") << false;
    QTest::newRow("no authentication or GSSAPI - connect to www.google.com:80")
        << QByteArray::fromHex("05020001") << true
        << QByteArray::fromHex("050100030e7777772e676f6f676c652e636f6d0050") << true;

    QTest::newRow("bad SOCKS version")
        << QByteArray::fromHex("060100") << false
        << QByteArray() << false;
    QTest::newRow("no methods")
        << QByteArray::fromHex("0500") << false
        << QByteArray() << false;
    QTest::newRow("GSSAPI only")
        << QByteArray::fromHex("050101") << false
        << QByteArray() << false;
}

void tst_QXmppSocks::testServer()
{
    QFETCH(QByteArray, clientHandshake);
    QFETCH(bool, clientHandshakeWorks);
    QFETCH(QByteArray, clientConnect);
    QFETCH(bool, clientConnectWorks);

    QXmppSocksServer server;
    QVERIFY(server.listen());
    QVERIFY(server.serverPort() != 0);
    connect(&server, &QXmppSocksServer::newConnection,
            this, &tst_QXmppSocks::newConnectionSlot);

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, server.serverPort());
    QVERIFY2(client.waitForConnected(), qPrintable(client.errorString()));

    QEventLoop loop;
    connect(&client, &QAbstractSocket::disconnected, &loop, &QEventLoop::quit);
    connect(&client, &QIODevice::readyRead, &loop, &QEventLoop::quit);

    // send client handshake
    client.write(clientHandshake);
    loop.exec();
    if (!clientHandshakeWorks) {
        // consume any last data
        QByteArray data = client.readAll();
        if (client.state() != QAbstractSocket::UnconnectedState) {
            loop.exec();
        }

        QCOMPARE(client.state(), QAbstractSocket::UnconnectedState);

        QVERIFY(!m_connectionSocket);
        QVERIFY(m_connectionHostName.isNull());
        QCOMPARE(m_connectionPort, quint16(0));
        return;
    }

    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(client.readAll(), QByteArray::fromHex("0500"));

    // request connect to www.google.com port 80
    client.write(clientConnect);
    loop.exec();
    if (!clientConnectWorks) {
        QCOMPARE(client.state(), QAbstractSocket::UnconnectedState);

        QVERIFY(!m_connectionSocket);
        QVERIFY(m_connectionHostName.isNull());
        QCOMPARE(m_connectionPort, quint16(0));
        return;
    }

    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(client.readAll(), QByteArray::fromHex("050000030e7777772e676f6f676c652e636f6d0050"));

    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QVERIFY(m_connectionSocket);
    QCOMPARE(m_connectionSocket->state(), QAbstractSocket::ConnectedState);
    QCOMPARE(m_connectionHostName, QLatin1String("www.google.com"));
    QCOMPARE(m_connectionPort, quint16(80));

    // disconnect
    client.disconnectFromHost();
}

QTEST_MAIN(tst_QXmppSocks)
#include "tst_qxmppsocks.moc"
