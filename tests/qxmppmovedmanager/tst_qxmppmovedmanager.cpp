// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDiscoveryManager.h"
#include "QXmppMovedManager.h"
#include "QXmppConstants_p.h"
#include "QXmppPubSubManager.h"
#ifdef BUILD_INTERNAL_TESTS
#include "QXmppMovedItem_p.h"
#endif

#include "TestClient.h"

struct Tester {
    Tester()
    {
        client.addNewExtension<QXmppDiscoveryManager>();
        client.addNewExtension<QXmppPubSubManager>();
        manager = client.addNewExtension<QXmppMovedManager>();
    }

    Tester(const QString &jid)
        : Tester()
    {
        client.configuration().setJid(jid);
    }

    TestClient client;
    QXmppMovedManager *manager;
};

class tst_QXmppMovedManager : public QObject
{
    Q_OBJECT

private:
#ifdef BUILD_INTERNAL_TESTS
    Q_SLOT void testMovedItem();
#endif
    Q_SLOT void testMovedPresence();
    Q_SLOT void testDiscoveryFeatures();
    Q_SLOT void testSupportedByServer();
    Q_SLOT void testResetCachedData();
    Q_SLOT void testHandleDiscoInfo();
    Q_SLOT void testOnRegistered();
    Q_SLOT void testOnUnregistered();
    Q_SLOT void testPublishMoved();
    Q_SLOT void testVerifyMoved();
    Q_SLOT void testNotify();

    template<typename T>
    void testError(QXmppTask<T> &task, TestClient &client, const QString &id, const QString &from);
};

#ifdef BUILD_INTERNAL_TESTS
void tst_QXmppMovedManager::testMovedItem()
{
    const auto expected = u"<item id=\"current\"><moved xmlns=\"urn:xmpp:moved:1\"><new-jid>new@shakespeare.example</new-jid></moved></item>"_s;
    const QDomElement expectedElement = xmlToDom(expected);

    {
        QXmppMovedItem packet;
        packet.setNewJid(u"new@shakespeare.example"_s);

        QCOMPARE(packetToXml(packet), expected);
    }

    {
        QXmppMovedItem packet;
        packet.parse(expectedElement);

        QVERIFY(!packet.newJid().isEmpty());
    }
}
#endif

void tst_QXmppMovedManager::testMovedPresence()
{
    const auto expected =
        u"<presence to=\"contact@shakespeare.example\" type=\"subscribe\">"
        "<moved xmlns=\"urn:xmpp:moved:1\"><old-jid>old@shakespeare.example</old-jid></moved>"
        "</presence>"_s;
    const QDomElement expectedElement = xmlToDom(expected);

    {
        QXmppPresence packet;
        packet.setTo(u"contact@shakespeare.example"_s);
        packet.setType(QXmppPresence::Subscribe);
        packet.setOldJid(u"old@shakespeare.example"_s);

        QCOMPARE(packetToXml(packet), expected);
    }

    {
        QXmppPresence packet;
        packet.parse(expectedElement);

        QVERIFY(!packet.oldJid().isEmpty());
    }
}

void tst_QXmppMovedManager::testDiscoveryFeatures()
{
    QXmppMovedManager manager;

    QCOMPARE(manager.discoveryFeatures(), QStringList { ns_moved.toString() });
}

void tst_QXmppMovedManager::testSupportedByServer()
{
    QXmppMovedManager manager;
    QSignalSpy spy(&manager, &QXmppMovedManager::supportedByServerChanged);

    QVERIFY(!manager.supportedByServer());

    manager.setSupportedByServer(true);

    QVERIFY(manager.supportedByServer());
    QCOMPARE(spy.size(), 1);
}

void tst_QXmppMovedManager::testResetCachedData()
{
    QXmppMovedManager manager;

    manager.setSupportedByServer(true);
    manager.resetCachedData();

    QVERIFY(!manager.supportedByServer());
}

void tst_QXmppMovedManager::testHandleDiscoInfo()
{
    auto [client, manager] = Tester(u"hag66@shakespeare.example"_s);

    QXmppDiscoveryIq iq;
    iq.setFeatures({ ns_moved.toString() });

    manager->handleDiscoInfo(iq);

    QVERIFY(manager->supportedByServer());

    iq.setFeatures({});

    manager->handleDiscoInfo(iq);

    QVERIFY(!manager->supportedByServer());
}

void tst_QXmppMovedManager::testOnRegistered()
{
    TestClient client;
    QXmppMovedManager manager;

    client.addNewExtension<QXmppDiscoveryManager>();
    client.addNewExtension<QXmppPubSubManager>();
    client.configuration().setJid(u"hag66@shakespeare.example"_s);
    client.addExtension(&manager);

    manager.setSupportedByServer(true);

    client.setStreamManagementState(QXmppClient::NewStream);
    Q_EMIT client.connected();

    QVERIFY(!manager.supportedByServer());

    QXmppDiscoveryIq iq;
    iq.setFeatures({ ns_moved.toString() });
    Q_EMIT manager.client()->findExtension<QXmppDiscoveryManager>()->infoReceived(iq);

    QVERIFY(manager.supportedByServer());
}

void tst_QXmppMovedManager::testOnUnregistered()
{
    QXmppClient client;
    QXmppMovedManager manager;

    client.addNewExtension<QXmppDiscoveryManager>();
    client.addNewExtension<QXmppPubSubManager>();
    client.configuration().setJid(u"hag66@shakespeare.example"_s);
    client.addExtension(&manager);

    manager.setSupportedByServer(true);
    manager.onUnregistered(&client);

    QXmppDiscoveryIq iq;
    iq.setFeatures({ ns_moved.toString() });
    Q_EMIT manager.client()->findExtension<QXmppDiscoveryManager>()->infoReceived(iq);

    QVERIFY(!manager.supportedByServer());

    manager.setSupportedByServer(true);
    Q_EMIT client.connected();

    QVERIFY(manager.supportedByServer());
}

void tst_QXmppMovedManager::testPublishMoved()
{
    auto tester = Tester(u"old@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->publishStatement(u"moved@shakespeare.example"_s);
    };

    auto task = call();

    client.expect(u"<iq id='qxmpp1' to='old@shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:moved:1'>"
                                 "<item id='current'>"
                                 "<moved xmlns='urn:xmpp:moved:1'>"
                                 "<new-jid>moved@shakespeare.example</new-jid>"
                                 "</moved>"
                                 "</item>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"_s);
    client.inject(u"<iq id='qxmpp1' from='old@shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='uurn:xmpp:moved:1'>"
                                 "<item id='current'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testError(task = call(), client, u"qxmpp1"_s, u"old@shakespeare.example"_s);
}

void tst_QXmppMovedManager::testVerifyMoved()
{
    auto tester = Tester(u"contact@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->verifyStatement(u"old@shakespeare.example"_s, u"moved@shakespeare.example"_s);
    };

    auto task = call();

    client.expect(u"<iq id='qxmpp1' to='old@shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:moved:1'>"
                                 "<item id='current'/>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"_s);
    client.inject(u"<iq id='qxmpp1' from='old@shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:moved:1'>"
                                 "<item id='current'>"
                                 "<moved xmlns='urn:xmpp:moved:1'>"
                                 "<new-jid>moved@shakespeare.example</new-jid>"
                                 "</moved>"
                                 "</item>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testError(task = call(), client, u"qxmpp1"_s, u"old@shakespeare.example"_s);
}

void tst_QXmppMovedManager::testNotify()
{
    auto tester = Tester(u"moved@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->notifyContact(u"contact@shakespeare.example"_s, u"old@shakespeare.example"_s, true, u"I moved."_s);
    };

    auto task = call();

    client.expect(u"<presence to='contact@shakespeare.example' type='subscribe'>"
                                 "<status>I moved.</status>"
                                 "<moved xmlns='urn:xmpp:moved:1'>"
                                 "<old-jid>old@shakespeare.example</old-jid>"
                                 "</moved>"
                                 "</presence>"_s);
}

template<typename T>
void tst_QXmppMovedManager::testError(QXmppTask<T> &task, TestClient &client, const QString &id, const QString &from)
{
    client.ignore();
    client.inject(u"<iq id='%1' from='%2' type='error'>"
                                 "<error type='cancel'>"
                                 "<not-allowed xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>"
                                 "</error>"
                                 "</iq>"_s
                      .arg(id, from));

    expectFutureVariant<QXmppError>(task);
}

QTEST_MAIN(tst_QXmppMovedManager)
#include "tst_qxmppmovedmanager.moc"
