/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppRosterIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppRosterIq : public QObject
{
    Q_OBJECT

private slots:
    void testItem_data();
    void testItem();
    void testVersion_data();
    void testVersion();
};

void tst_QXmppRosterIq::testItem_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("subscriptionType");

    QTest::newRow("none")
        << QByteArray(R"(<item jid="foo@example.com" subscription="none"/>)")
        << ""
        << int(QXmppRosterIq::Item::None);
    QTest::newRow("from")
        << QByteArray(R"(<item jid="foo@example.com" subscription="from"/>)")
        << ""
        << int(QXmppRosterIq::Item::From);
    QTest::newRow("to")
        << QByteArray(R"(<item jid="foo@example.com" subscription="to"/>)")
        << ""
        << int(QXmppRosterIq::Item::To);
    QTest::newRow("both")
        << QByteArray(R"(<item jid="foo@example.com" subscription="both"/>)")
        << ""
        << int(QXmppRosterIq::Item::Both);
    QTest::newRow("remove")
        << QByteArray(R"(<item jid="foo@example.com" subscription="remove"/>)")
        << ""
        << int(QXmppRosterIq::Item::Remove);
    QTest::newRow("notset")
        << QByteArray("<item jid=\"foo@example.com\"/>")
        << ""
        << int(QXmppRosterIq::Item::NotSet);

    QTest::newRow("name")
        << QByteArray(R"(<item jid="foo@example.com" name="foo bar"/>)")
        << "foo bar"
        << int(QXmppRosterIq::Item::NotSet);
}

void tst_QXmppRosterIq::testItem()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, name);
    QFETCH(int, subscriptionType);

    QXmppRosterIq::Item item;
    parsePacket(item, xml);
    QCOMPARE(item.bareJid(), QLatin1String("foo@example.com"));
    QCOMPARE(item.groups(), QSet<QString>());
    QCOMPARE(item.name(), name);
    QCOMPARE(int(item.subscriptionType()), subscriptionType);
    QCOMPARE(item.subscriptionStatus(), QString());
    serializePacket(item, xml);
}

void tst_QXmppRosterIq::testVersion_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("version");

    QTest::newRow("noversion")
        << QByteArray(R"(<iq id="woodyisacat" to="woody@zam.tw/cat" type="result"><query xmlns="jabber:iq:roster"/></iq>)")
        << "";

    QTest::newRow("version")
        << QByteArray(R"(<iq id="woodyisacat" to="woody@zam.tw/cat" type="result"><query xmlns="jabber:iq:roster" ver="3345678"/></iq>)")
        << "3345678";
}

void tst_QXmppRosterIq::testVersion()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, version);

    QXmppRosterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.version(), version);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppRosterIq)
#include "tst_qxmpprosteriq.moc"
