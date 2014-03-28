/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QObject>

#include "QXmppIq.h"
#include "util.h"

class tst_QXmppIq : public QObject
{
    Q_OBJECT

private slots:
    void testBasic_data();
    void testBasic();
};

void tst_QXmppIq::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");

    QTest::newRow("get")
        << QByteArray("<iq to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"get\"/>")
        << int(QXmppIq::Get);

    QTest::newRow("set")
        << QByteArray("<iq to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"set\"/>")
        << int(QXmppIq::Set);

    QTest::newRow("result")
        << QByteArray("<iq to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"result\"/>")
        << int(QXmppIq::Result);

    QTest::newRow("error")
        << QByteArray("<iq to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"error\"/>")
        << int(QXmppIq::Error);
}

void tst_QXmppIq::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);

    QXmppIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(iq.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(int(iq.type()), type);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppIq)
#include "tst_qxmppiq.moc"
