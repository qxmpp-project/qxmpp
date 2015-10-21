/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
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

#include <QBuffer>
#include <QObject>

#include "QXmppClient.h"
#include "QXmppServer.h"
#include "QXmppTransferManager.h"
#include "util.h"

Q_DECLARE_METATYPE(QXmppTransferJob::Method)

class tst_QXmppTransferManager : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testSendFile_data();
    void testSendFile();

    void acceptFile(QXmppTransferJob *job);

private:
    QBuffer receiverBuffer;
    QXmppTransferJob *receiverJob;
};

void tst_QXmppTransferManager::init()
{
    receiverBuffer.close();
    receiverBuffer.setData(QByteArray());
    receiverJob = 0;
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
    //logger.setLoggingType(QXmppLogger::StdoutLogging);

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
    QXmppTransferManager *senderManager = new QXmppTransferManager;
    senderManager->setSupportedMethods(senderMethods);
    sender.addExtension(senderManager);
    sender.setLogger(&logger);

    QEventLoop senderLoop;
    connect(&sender, SIGNAL(connected()), &senderLoop, SLOT(quit()));
    connect(&sender, SIGNAL(disconnected()), &senderLoop, SLOT(quit()));

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
    QXmppTransferManager *receiverManager = new QXmppTransferManager;
    receiverManager->setSupportedMethods(receiverMethods);
    connect(receiverManager, SIGNAL(fileReceived(QXmppTransferJob*)),
            this, SLOT(acceptFile(QXmppTransferJob*)));
    receiver.addExtension(receiverManager);
    receiver.setLogger(&logger);

    QEventLoop receiverLoop;
    connect(&receiver, SIGNAL(connected()), &receiverLoop, SLOT(quit()));
    connect(&receiver, SIGNAL(disconnected()), &receiverLoop, SLOT(quit()));

    config.setUser("receiver");
    config.setPassword("testpwd");
    receiver.connectToServer(config);
    receiverLoop.exec();
    QCOMPARE(receiver.isConnected(), true);

    // send file
    QEventLoop loop;
    QXmppTransferJob *senderJob = senderManager->sendFile("receiver@localhost/QXmpp", ":/test.svg");
    QVERIFY(senderJob);
    QCOMPARE(senderJob->localFileUrl(), QUrl::fromLocalFile(":/test.svg"));
    connect(senderJob, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (works) {
        QCOMPARE(senderJob->state(), QXmppTransferJob::FinishedState);
        QCOMPARE(senderJob->error(), QXmppTransferJob::NoError);

        // finish receiving file
        QVERIFY(receiverJob);
        connect(receiverJob, SIGNAL(finished()), &loop, SLOT(quit()));
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
