// SPDX-FileCopyrightText: 2015 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStun.h"

#include "util.h"
#include <QHostInfo>

class tst_QXmppIceConnection : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBind();
    Q_SLOT void testBindStun();
    Q_SLOT void testConnect();
};

void tst_QXmppIceConnection::testBind()
{
    const int componentId = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppIceConnection client;
    connect(&client, &QXmppLoggable::logMessage,
            &logger, &QXmppLogger::log);
    client.setIceControlling(true);
    client.addComponent(componentId);

    QXmppIceComponent *component = client.component(componentId);
    QVERIFY(component);

    QCOMPARE(client.gatheringState(), QXmppIceConnection::NewGatheringState);
    client.bind(QXmppIceComponent::discoverAddresses());
    QCOMPARE(client.gatheringState(), QXmppIceConnection::CompleteGatheringState);
    QCOMPARE(client.localCandidates().size(), component->localCandidates().size());
    QVERIFY(!client.localCandidates().isEmpty());
    const auto &localCandidates = client.localCandidates();
    for (const auto &c : localCandidates) {
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
    connect(&client, &QXmppLoggable::logMessage,
            &logger, &QXmppLogger::log);
    client.setIceControlling(true);
    QList<QPair<QHostAddress, quint16>> stunServers;
    for (auto &address : stunInfo.addresses()) {
        stunServers.push_back({ address, 19302 });
    }
    client.setStunServers(stunServers);
    client.addComponent(componentId);

    QXmppIceComponent *component = client.component(componentId);
    QVERIFY(component);

    QCOMPARE(client.gatheringState(), QXmppIceConnection::NewGatheringState);
    client.bind(QXmppIceComponent::discoverAddresses());
    QCOMPARE(client.gatheringState(), QXmppIceConnection::BusyGatheringState);

    QEventLoop loop;
    connect(&client, &QXmppIceConnection::gatheringStateChanged,
            &loop, &QEventLoop::quit);
    loop.exec();

    bool foundReflexive = false;
    QCOMPARE(client.gatheringState(), QXmppIceConnection::CompleteGatheringState);
    QCOMPARE(client.localCandidates().size(), component->localCandidates().size());
    QVERIFY(!client.localCandidates().isEmpty());
    const auto &localCandidates = client.localCandidates();
    for (const auto &c : localCandidates) {
        QCOMPARE(c.component(), componentId);
        if (c.type() == QXmppJingleCandidate::ServerReflexiveType) {
            foundReflexive = true;
        } else {
            QCOMPARE(c.type(), QXmppJingleCandidate::HostType);
        }
    }
    QVERIFY(foundReflexive);
}

void tst_QXmppIceConnection::testConnect()
{
    const int componentId = 1024;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    QXmppIceConnection clientL;
    connect(&clientL, &QXmppLoggable::logMessage,
            &logger, &QXmppLogger::log);
    clientL.setIceControlling(true);
    clientL.addComponent(componentId);
    clientL.bind(QXmppIceComponent::discoverAddresses());

    QXmppIceConnection clientR;
    connect(&clientR, &QXmppLoggable::logMessage,
            &logger, &QXmppLogger::log);
    clientR.setIceControlling(false);
    clientR.addComponent(componentId);
    clientR.bind(QXmppIceComponent::discoverAddresses());

    // exchange credentials
    clientL.setRemoteUser(clientR.localUser());
    clientL.setRemotePassword(clientR.localPassword());
    clientR.setRemoteUser(clientL.localUser());
    clientR.setRemotePassword(clientL.localPassword());

    // exchange candidates
    const auto &rLocalCandidates = clientR.localCandidates();
    for (const auto &candidate : rLocalCandidates) {
        clientL.addRemoteCandidate(candidate);
    }
    const auto &lLocalCandidates = clientL.localCandidates();
    for (const auto &candidate : lLocalCandidates) {
        clientR.addRemoteCandidate(candidate);
    }

    // start ICE
    QEventLoop loop;
    connect(&clientL, &QXmppIceConnection::connected, &loop, &QEventLoop::quit);
    connect(&clientR, &QXmppIceConnection::connected, &loop, &QEventLoop::quit);

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
