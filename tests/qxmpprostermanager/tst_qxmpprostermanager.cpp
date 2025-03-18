// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMovedManager.h"
#include "QXmppPubSubManager.h"
#include "QXmppRosterManager.h"

#include "TestClient.h"

class tst_QXmppRosterManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testDiscoFeatures();
    Q_SLOT void testRenameItem();
    Q_SLOT void testSubscriptionRequestReceived();
    Q_SLOT void testMovedSubscriptionRequestReceived_data();
    Q_SLOT void testMovedSubscriptionRequestReceived();
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
    QCOMPARE(manager->getRosterEntry("bob@qxmpp.org").subscriptionStatus(), u"subscribe"_s);

    // rename bob
    bool requestSent = false;
    connect(&logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            requestSent = true;

            QXmppRosterIq renameRequest;
            parsePacket(renameRequest, text.toUtf8());
            QCOMPARE(renameRequest.items().size(), 1);
            QCOMPARE(renameRequest.items().first().bareJid(), u"bob@qxmpp.org"_s);
            QCOMPARE(renameRequest.items().first().name(), u"Bob"_s);
            // check that subscription state ('ask') for bob is not included
            QVERIFY(renameRequest.items().first().subscriptionStatus().isNull());
        }
    });

    manager->renameItem("bob@qxmpp.org", "Bob");
    QVERIFY(requestSent);
}

void tst_QXmppRosterManager::testSubscriptionRequestReceived()
{
    QXmppPresence presence;
    presence.setType(QXmppPresence::Subscribe);
    presence.setFrom(u"alice@example.org/notebook"_s);
    presence.setStatusText(u"Hi, I'm Alice."_s);

    bool subscriptionRequestReceived = false;

    connect(manager, &QXmppRosterManager::subscriptionRequestReceived, this, [&](const QString &subscriberBareJid, const QXmppPresence &presence) {
        subscriptionRequestReceived = true;

        QCOMPARE(subscriberBareJid, u"alice@example.org"_s);
        QCOMPARE(presence.statusText(), u"Hi, I'm Alice."_s);
    });

    Q_EMIT client.presenceReceived(presence);
    QVERIFY(subscriptionRequestReceived);
}

void tst_QXmppRosterManager::testMovedSubscriptionRequestReceived_data()
{
    QTest::addColumn<bool>("movedManagerAdded");
    QTest::addColumn<QString>("oldJid");
    QTest::addColumn<QString>("oldJidResponse");
    QTest::addColumn<bool>("valid");

    QTest::newRow("noMovedManager")
        << false
        << QString()
        << QString()
        << false;
    QTest::newRow("oldJidEmpty")
        << true
        << QString()
        << QString()
        << false;
    QTest::newRow("oldJidNotInRoster")
        << true
        << u"old-invalid@example.org"_s
        << QString()
        << false;
    QTest::newRow("oldJidRespondingWithError")
        << true
        << u"old@example.org"_s
        << u"<iq id='qxmpp1' from='old@example.org' type='error'>"
           u"<error type='cancel'>"
           u"<not-allowed xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>"
           u"</error>"
           u"</iq>"_s
        << false;
    QTest::newRow("oldJidValid")
        << true
        << u"old@example.org"_s
        << u"<iq id='qxmpp1' from='old@example.org' type='result'>"
           "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
           "<items node='urn:xmpp:moved:1'>"
           "<item id='current'>"
           "<moved xmlns='urn:xmpp:moved:1'>"
           "<new-jid>new@example.org</new-jid>"
           "</moved>"
           "</item>"
           "</items>"
           "</pubsub>"
           "</iq>"_s
        << true;
}

void tst_QXmppRosterManager::testMovedSubscriptionRequestReceived()
{
    TestClient client(true);
    client.configuration().setJid(u"alice@example.org"_s);
    auto *rosterManager = client.addNewExtension<QXmppRosterManager>(&client);

    QFETCH(bool, movedManagerAdded);
    QFETCH(QString, oldJid);
    QFETCH(QString, oldJidResponse);
    QFETCH(bool, valid);

    if (movedManagerAdded) {
        client.addNewExtension<QXmppDiscoveryManager>();
        client.addNewExtension<QXmppPubSubManager>();
        client.addNewExtension<QXmppMovedManager>();

        QXmppRosterIq::Item rosterItem;
        rosterItem.setBareJid(u"old@example.org"_s);
        rosterItem.setSubscriptionType(QXmppRosterIq::Item::SubscriptionType::Both);

        QXmppRosterIq rosterIq;
        rosterIq.setType(QXmppIq::Set);
        rosterIq.setItems({ rosterItem });
        rosterManager->handleStanza(writePacketToDom(rosterIq));
    }

    QXmppPresence presence;
    presence.setType(QXmppPresence::Subscribe);
    presence.setFrom(u"new@example.org/notebook"_s);
    presence.setOldJid(oldJid);

    bool subscriptionRequestReceived = false;
    client.resetIdCount();

    connect(rosterManager, &QXmppRosterManager::subscriptionRequestReceived, this, [&subscriptionRequestReceived, &oldJid, &valid](const QString &subscriberBareJid, const QXmppPresence &presence) {
        subscriptionRequestReceived = true;
        QCOMPARE(subscriberBareJid, u"new@example.org"_s);
        if (valid) {
            QCOMPARE(oldJid, presence.oldJid());
        } else {
            QVERIFY(presence.oldJid().isEmpty());
        }
    });

    Q_EMIT client.presenceReceived(presence);

    if (!oldJidResponse.isEmpty()) {
        client.inject(oldJidResponse);
    }

    QVERIFY(subscriptionRequestReceived);
}

void tst_QXmppRosterManager::testAddItem()
{
    TestClient test;
    test.configuration().setJid(u"juliet@capulet.lit"_s);
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
    auto err = expectFutureVariant<QXmppError>(future);
    auto error = err.value<QXmppStanza::Error>().value();
    QCOMPARE(error.type(), QXmppStanza::Error::Modify);
    QCOMPARE(error.text(), u"This is not allowed"_s);
}

void tst_QXmppRosterManager::testRemoveItem()
{
    TestClient test;
    test.configuration().setJid(u"juliet@capulet.lit"_s);
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
    auto err = expectFutureVariant<QXmppError>(future);
    auto error = err.value<QXmppStanza::Error>().value();
    QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
    QCOMPARE(error.text(), u"Not found"_s);
}

QTEST_MAIN(tst_QXmppRosterManager)
#include "tst_qxmpprostermanager.moc"
