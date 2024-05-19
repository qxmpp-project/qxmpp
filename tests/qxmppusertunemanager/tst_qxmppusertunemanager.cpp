// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubManager.h"
#include "QXmppUserTuneItem.h"
#include "QXmppUserTuneManager.h"

#include "TestClient.h"

using PSManager = QXmppPubSubManager;

class tst_QXmppUserTuneManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();
    Q_SLOT void testRequest();
    Q_SLOT void testPublish();
    Q_SLOT void testEvents();
};

void tst_QXmppUserTuneManager::initTestCase()
{
    qRegisterMetaType<QXmppTuneItem>();
}

void tst_QXmppUserTuneManager::testRequest()
{
    TestClient test;
    test.addNewExtension<QXmppPubSubManager>();
    auto *tuneManager = test.addNewExtension<QXmppUserTuneManager>();

    auto future = tuneManager->request("anthony@qxmpp.org");
    test.expect("<iq id=\"qxmpp1\" to=\"anthony@qxmpp.org\" type=\"get\"><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"><items node=\"http://jabber.org/protocol/tune\"/></pubsub></iq>");
    test.inject(QStringLiteral("<iq id=\"qxmpp1\" from=\"anthony@qxmpp.org\" type=\"result\">"
                               "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
                               "<items node=\"http://jabber.org/protocol/tune\">"
                               "<item id='abc3'><tune xmlns='http://jabber.org/protocol/tune'><title>I Kiste girl</title></tune></item>"
                               "</items>"
                               "</pubsub></iq>"));

    QCoreApplication::processEvents();
    auto item = expectFutureVariant<QXmppTuneItem>(future);
    QCOMPARE(item.id(), u"abc3"_s);
    QCOMPARE(item.title(), u"I Kiste girl"_s);
}

void tst_QXmppUserTuneManager::testPublish()
{
    TestClient test;
    test.configuration().setJid("stpeter@jabber.org");
    test.addNewExtension<QXmppPubSubManager>();
    auto *tuneManager = test.addNewExtension<QXmppUserTuneManager>();

    QXmppTuneItem item;
    item.setArtist("Yes");
    item.setLength(686);
    item.setRating(8);
    item.setSource("Yessongs");
    item.setTitle("Heart of the Sunrise");
    item.setTrack("3");
    item.setUri(QUrl("http://www.yesworld.com/lyrics/Fragile.html#9"));

    auto future = tuneManager->publish(item);
    test.expect("<iq id='qxmpp1' to='stpeter@jabber.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                "<publish node='http://jabber.org/protocol/tune'>"
                "<item><tune xmlns='http://jabber.org/protocol/tune'>"
                "<artist>Yes</artist><length>686</length><rating>8</rating><source>Yessongs</source><title>Heart of the Sunrise</title><track>3</track><uri>http://www.yesworld.com/lyrics/Fragile.html#9</uri></tune></item>"
                "</publish>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='stpeter@jabber.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<publish node='http://jabber.org/protocol/tune'>"
                               "<item id='abcdf'/>"
                               "</publish></pubsub></iq>"));

    QCOMPARE(expectFutureVariant<QString>(future), u"abcdf"_s);
}

void tst_QXmppUserTuneManager::testEvents()
{
    TestClient test;
    test.configuration().setJid("stpeter@jabber.org");
    auto *psManager = test.addNewExtension<QXmppPubSubManager>();
    auto *tuneManager = test.addNewExtension<QXmppUserTuneManager>();

    QSignalSpy spy(tuneManager, &QXmppUserTuneManager::itemReceived);

    psManager->handleStanza(xmlToDom(QStringLiteral("<message from='stpeter@jabber.org' to='maineboy@jabber.org'>"
                                                    "<event xmlns='http://jabber.org/protocol/pubsub#event'>"
                                                    "<items node='http://jabber.org/protocol/tune'>"
                                                    "<item id='bffe6584-0f9c-11dc-84ba-001143d5d5db'>"
                                                    "<tune xmlns='http://jabber.org/protocol/tune'>"
                                                    "<artist>Yes</artist><length>686</length><rating>8</rating><source>Yessongs</source><title>Heart of the Sunrise</title><track>3</track><uri>http://www.yesworld.com/lyrics/Fragile.html#9</uri>"
                                                    "</tune></item></items>"
                                                    "</event></message>")));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.constFirst().at(0).toString(), u"stpeter@jabber.org"_s);
    QCOMPARE(spy.constFirst().at(1).value<QXmppTuneItem>().artist(), u"Yes"_s);
}

QTEST_MAIN(tst_QXmppUserTuneManager)
#include "tst_qxmppusertunemanager.moc"
