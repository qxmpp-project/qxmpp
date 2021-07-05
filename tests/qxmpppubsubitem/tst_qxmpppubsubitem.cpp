/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppPubSubItem.h"

#include "pubsubutil.h"
#include "util.h"
#include <QObject>

class tst_QXmppPubSubItem : public QObject
{
    Q_OBJECT

private slots:
    void testItem();
    void testIsItem_data();
    void testIsItem();
    void testTestItem();
};

void tst_QXmppPubSubItem::testItem()
{
    const auto xml = QByteArrayLiteral("<item id=\"abc1337\" publisher=\"lnj@qxmpp.org\"/>");

    QXmppPubSubItem item;
    parsePacket(item, xml);

    QCOMPARE(item.id(), QStringLiteral("abc1337"));
    QCOMPARE(item.publisher(), QStringLiteral("lnj@qxmpp.org"));

    // test serialization with parsed item
    serializePacket(item, xml);

    // test serialization with constructor values
    item = QXmppPubSubItem("abc1337", "lnj@qxmpp.org");
    serializePacket(item, xml);

    // test serialization with setters
    item = QXmppPubSubItem();
    item.setId("abc1337");
    item.setPublisher("lnj@qxmpp.org");
    serializePacket(item, xml);
}

void tst_QXmppPubSubItem::testIsItem_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid-id-publisher")
        << QByteArrayLiteral("<item id=\"abc1337\" publisher=\"lnj@qxmpp.org\"/>")
        << true;
    QTest::newRow("valid-id")
        << QByteArrayLiteral("<item id=\"abc1337\"/>")
        << true;
    QTest::newRow("valid-publisher")
        << QByteArrayLiteral("<item publisher=\"lnj@qxmpp.org\"/>")
        << true;
    QTest::newRow("valid")
        << QByteArrayLiteral("<item/>")
        << true;
    QTest::newRow("valid-payload")
        << QByteArrayLiteral("<item><payload xmlns=\"blah\"/></item>")
        << true;
    QTest::newRow("invalid-tag-name")
        << QByteArrayLiteral("<pubsub-item id=\"abc1337\" publisher=\"lnj@qxmpp.org\"/>")
        << false;
}

void tst_QXmppPubSubItem::testIsItem()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, valid);

    QCOMPARE(QXmppPubSubItem::isItem(xmlToDom(xml)), valid);
}

void tst_QXmppPubSubItem::testTestItem()
{
    const auto xml = QByteArrayLiteral("<item id=\"abc1337\" publisher=\"lnj@qxmpp.org\"><test-payload/></item>");

    TestItem item;
    parsePacket(item, xml);
    serializePacket(item, xml);

    QVERIFY(item.parseCalled);
    QVERIFY(item.serializeCalled);

    const auto invalidXml = QByteArrayLiteral("<item id=\"abc1337\"><tune/></item>");
    QVERIFY(TestItem::isItem(xmlToDom(xml)));
    QVERIFY(!TestItem::isItem(xmlToDom(invalidXml)));
}

QTEST_MAIN(tst_QXmppPubSubItem)
#include "tst_qxmpppubsubitem.moc"
