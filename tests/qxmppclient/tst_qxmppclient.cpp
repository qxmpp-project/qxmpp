/*
 * Copyright (C) 2008-2018 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn <lnj@kaidan.im>
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

#include "QXmppClient.h"
#include "QXmppServer.h"
#include "QXmppServerExtension.h"
#include "QXmppIncomingClient.cpp"

#include "util.h"

class tst_QXmppClient : public QObject
{
    Q_OBJECT

private slots:
    void setupConnection(QXmppServer &server, QXmppClient &client, QEventLoop &loop);
    void stopConnection(QXmppServer &server, QXmppClient &client, QEventLoop &loop);

    void testClientStateIndication_data();
    void testClientStateIndication();
};

void tst_QXmppClient::setupConnection(QXmppServer &server, QXmppClient &client, QEventLoop &loop)
{
    const QString testDomain("localhost");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    // prepare server
    TestPasswordChecker passwordChecker;
    passwordChecker.addCredentials("client", "testpwd");

    server.setDomain(testDomain);
    server.setPasswordChecker(&passwordChecker);
    server.listenForClients(testHost, testPort);

    connect(&client, &QXmppClient::connected, &loop, &QEventLoop::quit);
    connect(&client, &QXmppClient::disconnected, &loop, &QEventLoop::quit);

    QXmppConfiguration config;
    config.setDomain(testDomain);
    config.setHost(testHost.toString());
    config.setPort(testPort);
    config.setUser("client");
    config.setPassword("testpwd");
    client.connectToServer(config);
    loop.exec();

    QCOMPARE(client.isConnected(), true);
    QCOMPARE(client.state(), QXmppClient::ConnectedState);
}

void tst_QXmppClient::stopConnection(QXmppServer& server, QXmppClient& client, QEventLoop& loop)
{
    connect(&server, &QXmppServer::clientDisconnected, &loop, &QEventLoop::quit);

    client.disconnectFromServer();
    loop.exec();
    server.close();

    QCOMPARE(client.isConnected(), false);
    QCOMPARE(client.state(), QXmppClient::DisconnectedState);
}

void tst_QXmppClient::testClientStateIndication_data()
{
    QTest::addColumn<bool>("csiEnabled");

    QTest::newRow("csiEnabled") << false;
    QTest::newRow("csiEnabled") << true;
}

void tst_QXmppClient::testClientStateIndication()
{
    QFETCH(bool, csiEnabled);

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppServer server;
    server.setLogger(&logger);
    QXmppClient client;
    client.setLogger(&logger);
    QEventLoop loop;

    if (csiEnabled)
        server.setClientStateIndication(true);

    setupConnection(server, client, loop);

    // client should be active by default
    QVERIFY(client.isActive());

    // making the client inactive only works with CSI enabled by the server
    client.setActive(false);
    QCOMPARE(client.isActive(), !csiEnabled);

    client.setActive(true);
    QCOMPARE(client.isActive(), true);

    stopConnection(server, client, loop);
}

QTEST_MAIN(tst_QXmppClient)
#include "tst_qxmppclient.moc"
