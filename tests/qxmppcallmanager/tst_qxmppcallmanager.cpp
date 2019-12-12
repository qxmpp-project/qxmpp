/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppCallManager.h"
#include "QXmppClient.h"
#include "QXmppServer.h"
#include "util.h"

class tst_QXmppCallManager : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testCall();

    void acceptCall(QXmppCall *call);

private:
    QXmppCall *receiverCall;
};

void tst_QXmppCallManager::init()
{
    receiverCall = nullptr;
}

void tst_QXmppCallManager::acceptCall(QXmppCall *call)
{
    receiverCall = call;
    call->accept();
}

void tst_QXmppCallManager::testCall()
{
    const QString testDomain("localhost");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    // prepare server
    TestPasswordChecker passwordChecker;
    passwordChecker.addCredentials("sender", "testpwd");
    passwordChecker.addCredentials("receiver", "testpwd");

    QXmppServer server;
    server.setDomain(testDomain);
    server.setPasswordChecker(&passwordChecker);
    server.listenForClients(testHost, testPort);

    // prepare sender
    QXmppClient sender;
    auto *senderManager = new QXmppCallManager;
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
    auto *receiverManager = new QXmppCallManager;
    connect(receiverManager, &QXmppCallManager::callReceived,
            this, &tst_QXmppCallManager::acceptCall);
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

    // connect call
    qDebug() << "======== CONNECT ========";
    QEventLoop loop;
    QXmppCall *senderCall = senderManager->call("receiver@localhost/QXmpp");
    QVERIFY(senderCall);
    connect(senderCall, &QXmppCall::connected, &loop, &QEventLoop::quit);
    loop.exec();
    QVERIFY(receiverCall);

    QCOMPARE(senderCall->direction(), QXmppCall::OutgoingDirection);
    QCOMPARE(senderCall->state(), QXmppCall::ActiveState);

    QCOMPARE(receiverCall->direction(), QXmppCall::IncomingDirection);
    QCOMPARE(receiverCall->state(), QXmppCall::ActiveState);

    // exchange some media
    qDebug() << "======== TALK ========";
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    // hangup call
    qDebug() << "======== HANGUP ========";
    connect(senderCall, &QXmppCall::finished, &loop, &QEventLoop::quit);
    senderCall->hangup();
    loop.exec();

    QCOMPARE(senderCall->direction(), QXmppCall::OutgoingDirection);
    QCOMPARE(senderCall->state(), QXmppCall::FinishedState);

    QCOMPARE(receiverCall->direction(), QXmppCall::IncomingDirection);
    QCOMPARE(receiverCall->state(), QXmppCall::FinishedState);
}

QTEST_MAIN(tst_QXmppCallManager)
#include "tst_qxmppcallmanager.moc"
