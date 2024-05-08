// SPDX-FileCopyrightText: 2015 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppServer.h"
#include "QXmppTransferManager.h"

#include "util.h"

#include <QBuffer>
#include <QObject>

class tst_QXmppTransferManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void init();
    Q_SLOT void testSendFile_data();
    Q_SLOT void testSendFile();

    Q_SLOT void acceptFile(QXmppTransferJob *job);

    QBuffer receiverBuffer;
    QXmppTransferJob *receiverJob;
};

void tst_QXmppTransferManager::init()
{
    receiverBuffer.close();
    receiverBuffer.setData(QByteArray());
    receiverJob = nullptr;
}

void tst_QXmppTransferManager::acceptFile(QXmppTransferJob *job)
{
    receiverJob = job;
    receiverBuffer.open(QIODevice::WriteOnly);
    job->accept(&receiverBuffer);
}

void tst_QXmppTransferManager::testSendFile_data()
{
    QTest::addColumn<QXmppTransferJob::Method>("senderMethods");
    QTest::addColumn<QXmppTransferJob::Method>("receiverMethods");
    QTest::addColumn<bool>("works");

    QTest::newRow("any - any") << QXmppTransferJob::AnyMethod << QXmppTransferJob::AnyMethod << true;
    QTest::newRow("any - inband") << QXmppTransferJob::AnyMethod << QXmppTransferJob::InBandMethod << true;
    QTest::newRow("any - socks") << QXmppTransferJob::AnyMethod << QXmppTransferJob::SocksMethod << true;

    QTest::newRow("inband - any") << QXmppTransferJob::InBandMethod << QXmppTransferJob::AnyMethod << true;
    QTest::newRow("inband - inband") << QXmppTransferJob::InBandMethod << QXmppTransferJob::InBandMethod << true;
    QTest::newRow("inband - socks") << QXmppTransferJob::InBandMethod << QXmppTransferJob::SocksMethod << false;

    QTest::newRow("socks - any") << QXmppTransferJob::SocksMethod << QXmppTransferJob::AnyMethod << true;
    QTest::newRow("socks - inband") << QXmppTransferJob::SocksMethod << QXmppTransferJob::InBandMethod << false;
    QTest::newRow("socks - socks") << QXmppTransferJob::SocksMethod << QXmppTransferJob::SocksMethod << true;
}

void tst_QXmppTransferManager::testSendFile()
{
    QFETCH(QXmppTransferJob::Method, senderMethods);
    QFETCH(QXmppTransferJob::Method, receiverMethods);
    QFETCH(bool, works);

    const QString testDomain("localhost");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    QXmppLogger logger;
    // logger.setLoggingType(QXmppLogger::StdoutLogging);

    // prepare server
    TestPasswordChecker passwordChecker;
    passwordChecker.addCredentials("sender", "testpwd");
    passwordChecker.addCredentials("receiver", "testpwd");

    QXmppServer server;
    server.setDomain(testDomain);
    server.setLogger(&logger);
    server.setPasswordChecker(&passwordChecker);
    server.listenForClients(testHost, testPort);

    // prepare sender
    QXmppClient sender;
    auto *senderManager = new QXmppTransferManager;
    senderManager->setSupportedMethods(senderMethods);
    sender.addExtension(senderManager);
    sender.setLogger(&logger);

    QEventLoop senderLoop;
    connect(&sender, &QXmppClient::connected, &senderLoop, &QEventLoop::quit);
    connect(&sender, &QXmppClient::disconnected, &senderLoop, &QEventLoop::quit);

    QXmppConfiguration config;
    config.setDomain(testDomain);
    config.setHost(testHost.toString());
    config.setPort(testPort);
    config.setUser("sender");
    config.setPassword("testpwd");
    sender.connectToServer(config);
    senderLoop.exec();
    QCOMPARE(sender.isConnected(), true);

    // prepare receiver
    QXmppClient receiver;
    auto *receiverManager = new QXmppTransferManager;
    receiverManager->setSupportedMethods(receiverMethods);
    connect(receiverManager, &QXmppTransferManager::fileReceived,
            this, &tst_QXmppTransferManager::acceptFile);
    receiver.addExtension(receiverManager);
    receiver.setLogger(&logger);

    QEventLoop receiverLoop;
    connect(&receiver, &QXmppClient::connected, &receiverLoop, &QEventLoop::quit);
    connect(&receiver, &QXmppClient::disconnected, &receiverLoop, &QEventLoop::quit);

    config.setUser("receiver");
    config.setPassword("testpwd");
    receiver.connectToServer(config);
    receiverLoop.exec();
    QCOMPARE(receiver.isConnected(), true);

    // send file
    QEventLoop loop;
    QXmppTransferJob *senderJob = senderManager->sendFile(receiver.configuration().jid(), ":/test.svg");
    QVERIFY(senderJob);
    QCOMPARE(senderJob->localFileUrl(), QUrl::fromLocalFile(":/test.svg"));
    connect(senderJob, &QXmppTransferJob::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (works) {
        QCOMPARE(senderJob->state(), QXmppTransferJob::FinishedState);
        QCOMPARE(senderJob->error(), QXmppTransferJob::NoError);

        // finish receiving file
        QVERIFY(receiverJob);
        connect(receiverJob, &QXmppTransferJob::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(receiverJob->state(), QXmppTransferJob::FinishedState);
        QCOMPARE(receiverJob->error(), QXmppTransferJob::NoError);

        // check received file
        QFile expectedFile(":/test.svg");
        QVERIFY(expectedFile.open(QIODevice::ReadOnly));
        const QByteArray expectedData = expectedFile.readAll();
        QCOMPARE(receiverBuffer.data(), expectedData);
    } else {
        QCOMPARE(senderJob->state(), QXmppTransferJob::FinishedState);
        QCOMPARE(senderJob->error(), QXmppTransferJob::AbortError);

        QVERIFY(!receiverJob);

        QCOMPARE(receiverBuffer.data(), QByteArray());
    }
}

QTEST_MAIN(tst_QXmppTransferManager)
#include "tst_qxmpptransfermanager.moc"
