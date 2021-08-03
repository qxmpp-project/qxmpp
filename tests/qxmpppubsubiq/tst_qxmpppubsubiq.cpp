/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include "QXmppPubSubIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppPubSubIq : public QObject
{
    Q_OBJECT

private slots:
    void testItems();
    void testItemsResponse();
    void testPublish();
    void testRetractItem();
    void testSubscribe();
    void testSubscription();
    void testSubscriptions();
    void testIsPubSubIq_data();
    void testIsPubSubIq();
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

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::ItemsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);

    iq = QXmppPubSubIq();
    iq.setId(QLatin1String("items1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("francisco@denmark.lit/barracks"));
    iq.setType(QXmppIq::Get);
    iq.setQueryType(QXmppPubSubIq::ItemsQuery);
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
        "<item id=\"current\">"
        "<storage xmlns=\"storage:bookmarks\">"
        "<conference"
        " autojoin=\"true\""
        " jid=\"theplay@conference.shakespeare.lit\""
        " name=\"The Play&apos;s the Thing\">"
        "<nick>JC</nick>"
        "</conference>"
        "</storage>"
        "</item>"
        "</items>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::ItemsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
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
        "<item id=\"current\">"
        "<storage xmlns=\"storage:bookmarks\">"
        "<conference"
        " autojoin=\"true\""
        " jid=\"theplay@conference.shakespeare.lit\""
        " name=\"The Play&apos;s the Thing\">"
        "<nick>JC</nick>"
        "</conference>"
        "</storage>"
        "</item>"
        "</publish>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::PublishQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    QCOMPARE(iq.items().first().contents().tagName(), QLatin1String("storage"));
    serializePacket(iq, xml);

    // serialize using setters

    QXmppElement itemContentNick;
    itemContentNick.setTagName(QStringLiteral("nick"));
    itemContentNick.setValue(QStringLiteral("JC"));

    QXmppElement itemContentConference;
    itemContentConference.setTagName(QStringLiteral("conference"));
    itemContentConference.setAttribute(QStringLiteral("autojoin"), QStringLiteral("true"));
    itemContentConference.setAttribute(QStringLiteral("jid"), QStringLiteral("theplay@conference.shakespeare.lit"));
    itemContentConference.setAttribute(QStringLiteral("name"), QStringLiteral("The Play's the Thing"));
    itemContentConference.appendChild(itemContentNick);

    QXmppElement itemContent;
    itemContent.setTagName(QStringLiteral("storage"));
    itemContent.setAttribute(QStringLiteral("xmlns"), QStringLiteral("storage:bookmarks"));
    itemContent.appendChild(itemContentConference);

    QXmppPubSubItem item;
    item.setId(QStringLiteral("current"));
    item.setContents(itemContent);

    iq = QXmppPubSubIq();
    iq.setId(QLatin1String("items1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("francisco@denmark.lit/barracks"));
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppPubSubIq::PublishQuery);
    iq.setQueryJid({});
    iq.setQueryNode(QLatin1String("storage:bookmarks"));
    iq.setItems(QList<QXmppPubSubItem>() << item);

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

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QString("retract1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("hamlet@denmark.lit/elsinore"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::RetractQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    QCOMPARE(iq.items().size(), 1);
    QCOMPARE(iq.items().first().id(), QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
    serializePacket(iq, xml);

    iq = QXmppPubSubIq();
    iq.setId(QLatin1String("retract1"));
    iq.setTo(QLatin1String("pubsub.shakespeare.lit"));
    iq.setFrom(QLatin1String("hamlet@denmark.lit/elsinore"));
    iq.setType(QXmppIq::Set);
    iq.setQueryType(QXmppPubSubIq::RetractQuery);
    iq.setQueryJid({});
    iq.setQueryNode(QLatin1String("princely_musings"));

    QXmppPubSubItem item;
    item.setId(QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
    iq.setItems(QList<QXmppPubSubItem>() << item);

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

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("sub1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscribeQuery);
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

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("sub1"));
    QCOMPARE(iq.to(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscriptionQuery);
    QCOMPARE(iq.queryJid(), QLatin1String("francisco@denmark.lit"));
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    QCOMPARE(iq.subscriptionId(), QLatin1String("ba49252aaa4f5d320c24d3766f0bdcade78c78d3"));
    serializePacket(iq, xml);

    iq = QXmppPubSubIq();
    iq.setId(QLatin1String("sub1"));
    iq.setTo(QLatin1String("francisco@denmark.lit/barracks"));
    iq.setFrom(QLatin1String("pubsub.shakespeare.lit"));
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppPubSubIq::SubscriptionQuery);
    iq.setQueryJid(QLatin1String("francisco@denmark.lit"));
    iq.setQueryNode(QLatin1String("princely_musings"));
    iq.setSubscriptionId(QLatin1String("ba49252aaa4f5d320c24d3766f0bdcade78c78d3"));
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

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("subscriptions1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscriptionsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QString());
    serializePacket(iq, xml);
}

void tst_QXmppPubSubIq::testIsPubSubIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid-pubsub-iq")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"http://jabber.org/protocol/pubsub\"></pubsub></iq>")
        << true;

    QTest::newRow("wrong-element")
        << QByteArrayLiteral("<iq><pubsub2 xmlns=\"http://jabber.org/protocol/pubsub\"></pubsub2></iq>")
        << false;

    QTest::newRow("wrong-namespace")
        << QByteArrayLiteral("<iq><pubsub xmlns=\"urn:xmpp:pubsub2:0\"></pubsub></iq>")
        << false;
}

void tst_QXmppPubSubIq::testIsPubSubIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    QCOMPARE(QXmppPubSubIq::isPubSubIq(element), isValid);
}

QTEST_MAIN(tst_QXmppPubSubIq)
#include "tst_qxmpppubsubiq.moc"
