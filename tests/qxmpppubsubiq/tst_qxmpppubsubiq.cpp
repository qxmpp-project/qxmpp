// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubBaseItem.h"
#include "QXmppPubSubIq_p.h"
#include "QXmppPubSubSubscription.h"
#include "QXmppResultSet.h"

#include "pubsubutil.h"
#include "util.h"
#include <QObject>

using namespace QXmpp::Private;

class tst_QXmppPubSubIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testItems();
    Q_SLOT void testItemsResponse();
    Q_SLOT void testCreateNode();
    Q_SLOT void testDeleteNode();
    Q_SLOT void testPublish();
    Q_SLOT void testRetractItem();
    Q_SLOT void testSubscribe();
    Q_SLOT void testSubscription();
    Q_SLOT void testSubscriptions();
    Q_SLOT void testIsPubSubIq_data();
    Q_SLOT void testIsPubSubIq();

    Q_SLOT void testCustomItem();
};

void tst_QXmppPubSubIq::testItems()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"get\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<items node=\"storage:bookmarks\"/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), PubSubIq<>::Items);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);

    iq = PubSubIq();
    iq.setId(QLatin1String("items1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("francisco@denmark.lit/barracks"));
    iq.setType(QXmppIq::Get);
    iq.setQueryType(PubSubIq<>::Items);
    iq.setQueryJid({});
    iq.setQueryNode(QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testItemsResponse()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"francisco@denmark.lit/barracks\""
        " from=\"pubsub.shakespeare.lit\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<items node=\"storage:bookmarks\">"
        "<item id=\"current\"/>"
        "</items>"
        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
        "<first index=\"0\">current</first>"
        "<last>otheritemid</last>"
        "<count>19</count>"
        "</set>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), PubSubIq<>::Items);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    QVERIFY(iq.itemsContinuation().has_value());
    QCOMPARE(iq.itemsContinuation()->count(), 19);
    QCOMPARE(iq.itemsContinuation()->index(), 0);
    QCOMPARE(iq.itemsContinuation()->first(), QStringLiteral("current"));
    QCOMPARE(iq.itemsContinuation()->last(), QStringLiteral("otheritemid"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testCreateNode()
{
    const QByteArray xml(
        "<iq id=\"create1\" to=\"pubsub.shakespeare.lit\" from=\"hamlet@denmark.lit/elsinore\" type=\"set\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<create node=\"princely_musings\"/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QString("create1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("hamlet@denmark.lit/elsinore"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), PubSubIq<>::Create);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    serializePacket(iq, xml);

    iq = PubSubIq();
    iq.setId(QLatin1String("create1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("hamlet@denmark.lit/elsinore"));
    iq.setType(QXmppIq::Set);
    iq.setQueryType(PubSubIq<>::Create);
    iq.setQueryNode(QLatin1String("princely_musings"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testDeleteNode()
{
    const QByteArray xml(
        "<iq id=\"delete1\" to=\"pubsub.shakespeare.lit\" from=\"hamlet@denmark.lit/elsinore\" type=\"set\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub#owner\">"
        "<delete node=\"princely_musings\"/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QString("delete1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("hamlet@denmark.lit/elsinore"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), PubSubIq<>::Delete);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    serializePacket(iq, xml);

    iq = PubSubIq<>();
    iq.setId(QLatin1String("delete1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("hamlet@denmark.lit/elsinore"));
    iq.setType(QXmppIq::Set);
    iq.setQueryType(PubSubIq<>::Delete);
    iq.setQueryNode(QLatin1String("princely_musings"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testPublish()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<publish node=\"storage:bookmarks\">"
        "<item id=\"current\"/>"
        "</publish>"
        "</pubsub>"
        "</iq>");

    PubSubIq<> iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), PubSubIq<>::Publish);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);

    // serialize using setters
    QXmppPubSubBaseItem item(QStringLiteral("current"));

    iq = PubSubIq();
    iq.setId(QLatin1String("items1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("francisco@denmark.lit/barracks"));
    iq.setType(QXmppIq::Result);
    iq.setQueryType(PubSubIq<>::Publish);
    iq.setQueryJid({});
    iq.setQueryNode(QLatin1String("storage:bookmarks"));
    iq.setItems({ item });

    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testRetractItem()
{
    const QByteArray xml(
        "<iq"
        " id=\"retract1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"hamlet@denmark.lit/elsinore\""
        " type=\"set\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<retract node=\"princely_musings\">"
        "<item id=\"ae890ac52d0df67ed7cfdf51b644e901\"/>"
        "</retract>"
        "</pubsub>"
        "</iq>");

    PubSubIq<> iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QString("retract1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("hamlet@denmark.lit/elsinore"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), PubSubIq<>::Retract);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    QCOMPARE(iq.items().size(), 1);
    QCOMPARE(iq.items().first().id(), QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
    serializePacket(iq, xml);

    iq = PubSubIq();
    iq.setId(QLatin1String("retract1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("hamlet@denmark.lit/elsinore"));
    iq.setType(QXmppIq::Set);
    iq.setQueryType(PubSubIq<>::Retract);
    iq.setQueryJid({});
    iq.setQueryNode(QLatin1String("princely_musings"));

    QXmppPubSubBaseItem item;
    item.setId(QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
    iq.setItems({ item });

    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testSubscribe()
{
    const QByteArray xml(
        "<iq"
        " id=\"sub1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"set\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscribe jid=\"francisco@denmark.lit\" node=\"princely_musings\"/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("sub1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), PubSubIq<>::Subscribe);
    QCOMPARE(iq.queryJid(), QLatin1String("francisco@denmark.lit"));
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testSubscription()
{
    const QByteArray xml(
        "<iq"
        " id=\"sub1\""
        " to=\"francisco@denmark.lit/barracks\""
        " from=\"pubsub.shakespeare.lit\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscription jid=\"francisco@denmark.lit\""
        " node=\"princely_musings\""
        " subid=\"ba49252aaa4f5d320c24d3766f0bdcade78c78d3\"/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QString("sub1"));
    QCOMPARE(iq.to(), QString("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QString("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), PubSubIq<>::Subscription);
    QCOMPARE(iq.subscription()->jid(), QString("francisco@denmark.lit"));
    QCOMPARE(iq.subscription()->node(), QString("princely_musings"));
    QCOMPARE(iq.subscription()->subId(), QStringLiteral("ba49252aaa4f5d320c24d3766f0bdcade78c78d3"));
    serializePacket(iq, xml);

    iq = PubSubIq();
    iq.setId("sub1");
    iq.setTo("francisco@denmark.lit/barracks");
    iq.setFrom("pubsub.shakespeare.lit");
    iq.setType(QXmppIq::Result);
    iq.setQueryType(PubSubIq<>::Subscription);
    iq.setSubscription(QXmppPubSubSubscription(
        "francisco@denmark.lit",
        "princely_musings",
        "ba49252aaa4f5d320c24d3766f0bdcade78c78d3"));
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testSubscriptions()
{
    const QByteArray xml(
        "<iq"
        " id=\"subscriptions1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"get\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscriptions/>"
        "</pubsub>"
        "</iq>");

    PubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("subscriptions1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), PubSubIq<>::Subscriptions);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QString());
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testIsPubSubIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid-pubsub-iq")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"><items node=\"smth\"/></pubsub></iq>")
        << true;

    QTest::newRow("items-missing-node-name")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"><items/></pubsub></iq>")
        << false;

    QTest::newRow("unknown-query-type")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"><shuffle/></pubsub></iq>")
        << false;

    QTest::newRow("wrong-element")
        << QByteArrayLiteral("<iq><pubsub2 xmlns=\"http://jabber.org/protocol/pubsub\"><items node=\"smth\"/></pubsub2></iq>")
        << false;

    QTest::newRow("wrong-namespace")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"urn:xmpp:pubsub2:0\"><items node=\"smth\"/></pubsub></iq>")
        << false;
}

void tst_QXmppPubSubIq::testIsPubSubIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QVERIFY(doc.setContent(xml, true));
    QDomElement element = doc.documentElement();

    QCOMPARE(PubSubIq<>::isPubSubIq(element), isValid);
}

void tst_QXmppPubSubIq::testCustomItem()
{
    const QByteArray xml(
        "<iq id=\"a1\" type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<items node=\"blah\">"
        "<item id=\"42\"><test-payload/></item>"
        "<item id=\"23\"><test-payload/></item>"
        "</items>"
        "</pubsub>"
        "</iq>");

    // test isPubSubIq also checks item validity
    TestItem::isItemCalled = false;
    QVERIFY(PubSubIq<TestItem>::isPubSubIq(xmlToDom(xml)));
    QVERIFY(TestItem::isItemCalled);

    PubSubIq<TestItem> iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), PubSubIq<>::Items);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QStringLiteral("blah"));
    QCOMPARE(iq.items().size(), 2);
    QCOMPARE(iq.items().at(0).id(), QStringLiteral("42"));
    QCOMPARE(iq.items().at(1).id(), QStringLiteral("23"));
    QCOMPARE(iq.items().at(0).publisher(), QString());
    QCOMPARE(iq.items().at(1).publisher(), QString());

    QVERIFY(iq.items().at(0).parseCalled);
    QVERIFY(iq.items().at(1).parseCalled);
    QVERIFY(!iq.items().at(0).serializeCalled);
    QVERIFY(!iq.items().at(1).serializeCalled);

    serializePacket(iq, xml);

    iq = PubSubIq<TestItem>();
    iq.setId("a1");
    iq.setType(QXmppIq::Result);
    iq.setQueryType(PubSubIq<>::Items);
    iq.setQueryNode("blah");
    iq.setItems({ TestItem("42"), TestItem("23") });
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppPubSubIq)
#include "tst_qxmpppubsubiq.moc"
