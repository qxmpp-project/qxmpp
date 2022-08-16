// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppGeolocItem.h"
#include "QXmppPubSubManager.h"
#include "QXmppUserLocationManager.h"

#include "TestClient.h"

using PSManager = QXmppPubSubManager;

#define COMPARE_OPT(ACTUAL, EXPECTED) \
    QVERIFY(ACTUAL.has_value());      \
    QCOMPARE(ACTUAL.value(), EXPECTED);

class tst_QXmppUserLocationManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();
    Q_SLOT void testRequest();
    Q_SLOT void testPublish();
    Q_SLOT void testEvents();
};

void tst_QXmppUserLocationManager::initTestCase()
{
    qRegisterMetaType<QXmppGeolocItem>();
}

void tst_QXmppUserLocationManager::testRequest()
{
    TestClient test;
    test.addNewExtension<QXmppPubSubManager>();
    auto *tuneManager = test.addNewExtension<QXmppUserLocationManager>();

    auto future = tuneManager->request("anthony@qxmpp.org");
    test.expect("<iq id=\"qxmpp1\" to=\"anthony@qxmpp.org\" type=\"get\"><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"><items node=\"http://jabber.org/protocol/geoloc\"/></pubsub></iq>");
    test.inject<QString>("<iq id=\"qxmpp1\" from=\"anthony@qxmpp.org\" type=\"result\">"
                         "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
                         "<items node=\"http://jabber.org/protocol/geoloc\">"
                         "<item id='abc3'><geoloc xmlns='http://jabber.org/protocol/geoloc'>"
                         "<accuracy>20</accuracy>"
                         "<country>Italy</country>"
                         "<lat>45.44</lat>"
                         "<locality>Venice</locality>"
                         "<lon>12.33</lon>"
                         "</geoloc></item>"
                         "</items>"
                         "</pubsub></iq>");

    QCoreApplication::processEvents();

    auto item = expectFutureVariant<QXmppGeolocItem>(future);
    QCOMPARE(item.id(), QString("abc3"));
    COMPARE_OPT(item.accuracy(), 20.0);
    COMPARE_OPT(item.longitude(), 12.33);
    COMPARE_OPT(item.latitude(), 45.44);
    QCOMPARE(item.locality(), QStringLiteral("Venice"));
    QCOMPARE(item.country(), QStringLiteral("Italy"));
}

void tst_QXmppUserLocationManager::testPublish()
{
    TestClient test;
    test.configuration().setJid("stpeter@jabber.org");
    test.addNewExtension<QXmppPubSubManager>();
    auto *manager = test.addNewExtension<QXmppUserLocationManager>();

    QXmppGeolocItem item;
    item.setId("abc3");
    item.setAccuracy(20);
    item.setCountry("Italy");
    item.setLatitude(45.44);
    item.setLongitude(12.33);
    item.setLocality("Venice");

    auto future = manager->publish(item);
    test.expect("<iq id='qxmpp1' to='stpeter@jabber.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                "<publish node='http://jabber.org/protocol/geoloc'>"
                "<item id='abc3'><geoloc xmlns='http://jabber.org/protocol/geoloc'>"
                "<accuracy>20</accuracy>"
                "<country>Italy</country>"
                "<lat>45.44</lat>"
                "<locality>Venice</locality>"
                "<lon>12.33</lon>"
                "</geoloc></item>"
                "</publish>"
                "</pubsub></iq>");
    test.inject<QString>("<iq type='result' from='stpeter@jabber.org' id='qxmpp1'>"
                         "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                         "<publish node='http://jabber.org/protocol/tune'>"
                         "<item id='some-id'/>"
                         "</publish></pubsub></iq>");

    QCOMPARE(expectFutureVariant<QString>(future), QString("some-id"));
}

void tst_QXmppUserLocationManager::testEvents()
{
    TestClient test;
    test.configuration().setJid("stpeter@jabber.org");
    auto *psManager = test.addNewExtension<QXmppPubSubManager>();
    auto *manager = test.addNewExtension<QXmppUserLocationManager>();

    QSignalSpy spy(manager, &QXmppUserLocationManager::itemReceived);

    const QString event = "<message from='stpeter@jabber.org' to='maineboy@jabber.org'>"
                          "<event xmlns='http://jabber.org/protocol/pubsub#event'>"
                          "<items node='http://jabber.org/protocol/geoloc'>"
                          "<item id='bffe6584-0f9c-11dc-84ba-001143d5d5db'>"
                          "<geoloc xmlns='http://jabber.org/protocol/geoloc'>"
                          "<accuracy>20</accuracy>"
                          "<country>Italy</country>"
                          "<lat>45.44</lat>"
                          "<locality>Venice</locality>"
                          "<lon>12.33</lon>"
                          "</geoloc></item></items>"
                          "</event></message>";
    psManager->handleStanza(xmlToDom(event));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.constFirst().at(0).toString(), QString("stpeter@jabber.org"));
    QCOMPARE(spy.constFirst().at(1).value<QXmppGeolocItem>().id(), QString("bffe6584-0f9c-11dc-84ba-001143d5d5db"));
    QCOMPARE(spy.constFirst().at(1).value<QXmppGeolocItem>().country(), QString("Italy"));
}

QTEST_MAIN(tst_QXmppUserLocationManager)
#include "tst_qxmppuserlocationmanager.moc"
