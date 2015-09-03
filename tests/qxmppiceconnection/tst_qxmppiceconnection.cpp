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

#include <QHostInfo>
#include "QXmppStun.h"
#include "util.h"

class tst_QXmppIceConnection : public QObject
{
    Q_OBJECT

private slots:
    void testBind();
    void testBindStun();
    void testConnect();
};

void tst_QXmppIceConnection::testBind()
{
    const int componentId = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppIceConnection client;
    connect(&client, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    client.setIceControlling(true);
    client.addComponent(componentId);

    QXmppIceComponent *component = client.component(componentId);
    QVERIFY(component);

    QCOMPARE(client.gatheringState(), QXmppIceConnection::NewGatheringState);
    client.bind(QXmppIceComponent::discoverAddresses());
    QCOMPARE(client.gatheringState(), QXmppIceConnection::CompleteGatheringState);
    QCOMPARE(client.localCandidates().size(), component->localCandidates().size());
    QVERIFY(!client.localCandidates().isEmpty());
    foreach (const QXmppJingleCandidate &c, client.localCandidates()) {
        QCOMPARE(c.component(), componentId);
        QCOMPARE(c.type(), QXmppJingleCandidate::HostType);
    }
}

void tst_QXmppIceConnection::testBindStun()
{
    const int componentId = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QHostInfo stunInfo = QHostInfo::fromName("stun.l.google.com");
    QVERIFY(!stunInfo.addresses().isEmpty());

    QXmppIceConnection client;
    connect(&client, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    client.setIceControlling(true);
    client.setStunServer(stunInfo.addresses().first(), 19302);
    client.addComponent(componentId);

    QXmppIceComponent *component = client.component(componentId);
    QVERIFY(component);

    QCOMPARE(client.gatheringState(), QXmppIceConnection::NewGatheringState);
    client.bind(QXmppIceComponent::discoverAddresses());
    QCOMPARE(client.gatheringState(), QXmppIceConnection::BusyGatheringState);

    QEventLoop loop;
    connect(&client, SIGNAL(gatheringStateChanged()),
            &loop, SLOT(quit()));
    loop.exec();

    bool foundReflexive = false;
    QCOMPARE(client.gatheringState(), QXmppIceConnection::CompleteGatheringState);
    QCOMPARE(client.localCandidates().size(), component->localCandidates().size());
    QVERIFY(!client.localCandidates().isEmpty());
    foreach (const QXmppJingleCandidate &c, client.localCandidates()) {
        QCOMPARE(c.component(), componentId);
        if (c.type() == QXmppJingleCandidate::ServerReflexiveType)
            foundReflexive = true;
        else
            QCOMPARE(c.type(), QXmppJingleCandidate::HostType);
    }
    QVERIFY(foundReflexive);
}

void tst_QXmppIceConnection::testConnect()
{
    const int componentId = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppIceConnection clientL;
    connect(&clientL, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    clientL.setIceControlling(true);
    clientL.addComponent(componentId);
    clientL.bind(QXmppIceComponent::discoverAddresses());

    QXmppIceConnection clientR;
    connect(&clientR, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
            &logger, SLOT(log(QXmppLogger::MessageType,QString)));
    clientR.setIceControlling(false);
    clientR.addComponent(componentId);
    clientR.bind(QXmppIceComponent::discoverAddresses());

    // exchange credentials
    clientL.setRemoteUser(clientR.localUser());
    clientL.setRemotePassword(clientR.localPassword());
    clientR.setRemoteUser(clientL.localUser());
    clientR.setRemotePassword(clientL.localPassword());

    // exchange candidates
    foreach (const QXmppJingleCandidate &candidate, clientR.localCandidates())
        clientL.addRemoteCandidate(candidate);
    foreach (const QXmppJingleCandidate &candidate, clientL.localCandidates())
        clientR.addRemoteCandidate(candidate);

    // start ICE
    QEventLoop loop;
    connect(&clientL, SIGNAL(connected()), &loop, SLOT(quit()));
    connect(&clientR, SIGNAL(connected()), &loop, SLOT(quit()));

    clientL.connectToHost();
    clientR.connectToHost();

    // check both clients are connected
    loop.exec();
    loop.exec();
    QVERIFY(clientL.isConnected());
    QVERIFY(clientR.isConnected());
}

QTEST_MAIN(tst_QXmppIceConnection)
#include "tst_qxmppiceconnection.moc"
