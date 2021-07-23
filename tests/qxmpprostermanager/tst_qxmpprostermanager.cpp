/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *  Melvin Keskin
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
#include "QXmppDiscoveryManager.h"
#include "QXmppRosterManager.h"

#include "TestClient.h"

class tst_QXmppRosterManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testDiscoFeatures();
    void testRenameItem();
    void subscriptionRequestReceived();
    Q_SLOT void testAddItem();
    Q_SLOT void testRemoveItem();

private:
    QXmppClient client;
    QXmppLogger logger;
    QXmppRosterManager *manager;
};

void tst_QXmppRosterManager::initTestCase()
{
    logger.setLoggingType(QXmppLogger::SignalLogging);
    client.setLogger(&logger);

    manager = client.findExtension<QXmppRosterManager>();
}

void tst_QXmppRosterManager::testDiscoFeatures()
{
    QCOMPARE(manager->discoveryFeatures(), QStringList());
}

void tst_QXmppRosterManager::testRenameItem()
{
    // used to clean up lambda signal connections
    QObject context;

    auto createItem = [](const QString &jid, const QString &ask = {}) -> QXmppRosterIq::Item {
        QXmppRosterIq::Item item;
        item.setBareJid(jid);
        item.setSubscriptionStatus(ask);
        return item;
    };

    // fill roster with initial contacts to rename
    QXmppRosterIq initialItems;
    initialItems.setType(QXmppIq::Result);
    initialItems.addItem(createItem("stpeter@jabber.org"));
    initialItems.addItem(createItem("bob@qxmpp.org"));

    QVERIFY(manager->handleStanza(writePacketToDom(initialItems)));

    // set a subscription state for bob (the subscription state MUST NOT be
    // sent when renaming an item, so we need to check that it's not)
    QXmppRosterIq bobAsk;
    bobAsk.setType(QXmppIq::Set);
    bobAsk.addItem(createItem("bob@qxmpp.org", "subscribe"));

    QVERIFY(manager->handleStanza(writePacketToDom(bobAsk)));
    QCOMPARE(manager->getRosterEntry("bob@qxmpp.org").subscriptionStatus(), QStringLiteral("subscribe"));

    // rename bob
    bool requestSent = false;
    connect(&logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            requestSent = true;

            QXmppRosterIq renameRequest;
            parsePacket(renameRequest, text.toUtf8());
            QCOMPARE(renameRequest.items().size(), 1);
            QCOMPARE(renameRequest.items().first().bareJid(), QStringLiteral("bob@qxmpp.org"));
            QCOMPARE(renameRequest.items().first().name(), QStringLiteral("Bob"));
            // check that subscription state ('ask') for bob is not included
            QVERIFY(renameRequest.items().first().subscriptionStatus().isNull());
        }
    });

    manager->renameItem("bob@qxmpp.org", "Bob");
    QVERIFY(requestSent);
}

void tst_QXmppRosterManager::subscriptionRequestReceived()
{
    QXmppPresence presence;
    presence.setType(QXmppPresence::Subscribe);
    presence.setFrom(QStringLiteral("alice@example.org/notebook"));
    presence.setStatusText(QStringLiteral("Hi, I'm Alice."));

    bool subscriptionRequestReceived = false;

    connect(manager, &QXmppRosterManager::subscriptionRequestReceived, this, [&](const QString &subscriberBareJid, const QXmppPresence &presence) {
        subscriptionRequestReceived = true;

        QCOMPARE(subscriberBareJid, QStringLiteral("alice@example.org"));
        QCOMPARE(presence.statusText(), QStringLiteral("Hi, I'm Alice."));
    });

    emit client.presenceReceived(presence);
    QVERIFY(subscriptionRequestReceived);
}

void tst_QXmppRosterManager::testAddItem()
{
    TestClient test;
    auto *rosterManager = test.addNewExtension<QXmppRosterManager>(&test);

    auto future = rosterManager->addRosterItem("contact@example.org");
    test.expect("<iq id='qxmpp1' type='set'><query xmlns='jabber:iq:roster'><item jid='contact@example.org'/></query></iq>");
    test.inject<QString>("<iq id='qxmpp1' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);

    future = rosterManager->addRosterItem("contact@example.org");
    test.expect("<iq id='qxmpp1' type='set'><query xmlns='jabber:iq:roster'><item jid='contact@example.org'/></query></iq>");
    test.inject<QString>(R"(
<iq id='qxmpp1' type='error'>
    <error type='modify'>
        <not-authorized xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>
        <text xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'>This is not allowed</text>
    </error>
</iq>)");
    auto error = expectFutureVariant<QXmppStanza::Error>(future);
    QCOMPARE(error.type(), QXmppStanza::Error::Modify);
    QCOMPARE(error.text(), QStringLiteral("This is not allowed"));
}

void tst_QXmppRosterManager::testRemoveItem()
{
    TestClient test;
    auto *rosterManager = test.addNewExtension<QXmppRosterManager>(&test);

    auto future = rosterManager->removeRosterItem("contact@example.org");
    test.expect("<iq id='qxmpp1' type='set'><query xmlns='jabber:iq:roster'><item jid='contact@example.org' subscription='remove'/></query></iq>");
    test.inject<QString>("<iq id='qxmpp1' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);

    future = rosterManager->removeRosterItem("contact@example.org");
    test.expect("<iq id='qxmpp1' type='set'><query xmlns='jabber:iq:roster'><item jid='contact@example.org' subscription='remove'/></query></iq>");
    test.inject<QString>(R"(
<iq id='qxmpp1' type='error'>
    <error type='cancel'>
        <item-not-found xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>
        <text xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'>Not found</text>
    </error>
</iq>)");
    auto error = expectFutureVariant<QXmppStanza::Error>(future);
    QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
    QCOMPARE(error.text(), QStringLiteral("Not found"));
}

QTEST_MAIN(tst_QXmppRosterManager)
#include "tst_qxmpprostermanager.moc"
