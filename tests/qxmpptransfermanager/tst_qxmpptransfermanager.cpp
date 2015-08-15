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

class tst_QXmppTransferManager : public QObject
{
    Q_OBJECT

private slots:
    void testByteStreams();

    void acceptFile(QXmppTransferJob *job);

private:
    QBuffer receiverBuffer;
};

void tst_QXmppTransferManager::acceptFile(QXmppTransferJob *job)
{
    receiverBuffer.open(QIODevice::WriteOnly);
    job->accept(&receiverBuffer);
}

void tst_QXmppTransferManager::testByteStreams()
{
   QCOMPARE(1, 1);

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
    connect(receiverManager, SIGNAL(jobFinished(QXmppTransferJob*)),
            &loop, SLOT(quit()));
    QVERIFY(senderManager->sendFile("receiver@localhost/QXmpp", ":/test.svg"));
    loop.exec();

    // check received file
    QFile expectedFile(":/test.svg");
    QVERIFY(expectedFile.open(QIODevice::ReadOnly));
    const QByteArray expectedData = expectedFile.readAll();
    QCOMPARE(receiverBuffer.data(), expectedData);
}

QTEST_MAIN(tst_QXmppTransferManager)
#include "tst_qxmpptransfermanager.moc"
