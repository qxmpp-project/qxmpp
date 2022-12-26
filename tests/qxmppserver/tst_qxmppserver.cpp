// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppServer.h"

#include "util.h"

class tst_QXmppServer : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testConnect_data();
    Q_SLOT void testConnect();
};

void tst_QXmppServer::testConnect_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<QString>("mechanism");
    QTest::addColumn<bool>("connected");

    QTest::newRow("plain-good") << "testuser"
                                << "testpwd"
                                << "PLAIN" << true;
    QTest::newRow("plain-bad-username") << "baduser"
                                        << "testpwd"
                                        << "PLAIN" << false;
    QTest::newRow("plain-bad-password") << "testuser"
                                        << "badpwd"
                                        << "PLAIN" << false;

    QTest::newRow("digest-good") << "testuser"
                                 << "testpwd"
                                 << "DIGEST-MD5" << true;
    QTest::newRow("digest-bad-username") << "baduser"
                                         << "testpwd"
                                         << "DIGEST-MD5" << false;
    QTest::newRow("digest-bad-password") << "testuser"
                                         << "badpwd"
                                         << "DIGEST-MD5" << false;
}

void tst_QXmppServer::testConnect()
{
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(QString, mechanism);
    QFETCH(bool, connected);

    const QString testDomain("localhost");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    QXmppLogger logger;
    // logger.setLoggingType(QXmppLogger::StdoutLogging);

    // prepare server
    TestPasswordChecker passwordChecker;
    passwordChecker.addCredentials("testuser", "testpwd");

    QXmppServer server;
    server.setDomain(testDomain);
    server.setLogger(&logger);
    server.setPasswordChecker(&passwordChecker);
    server.listenForClients(testHost, testPort);

    // prepare client
    QXmppClient client;
    client.setLogger(&logger);

    QEventLoop loop;
    connect(&client, &QXmppClient::connected,
            &loop, &QEventLoop::quit);
    connect(&client, &QXmppClient::disconnected,
            &loop, &QEventLoop::quit);

    QXmppConfiguration config;
    config.setDomain(testDomain);
    config.setHost(testHost.toString());
    config.setPort(testPort);
    config.setUser(username);
    config.setPassword(password);
    config.setSaslAuthMechanism(mechanism);
    client.connectToServer(config);
    loop.exec();
    QCOMPARE(client.isConnected(), connected);
}

QTEST_MAIN(tst_QXmppServer)
#include "tst_qxmppserver.moc"
