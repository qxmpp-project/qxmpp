/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "roster.h"
#include "tests.h"

void tst_QXmppRosterIq::testItem_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("subscriptionType");

    QTest::newRow("notset")
        << QByteArray("<item/>")
        << int(QXmppRosterIq::Item::NotSet);
    QTest::newRow("from")
        << QByteArray("<item subscription=\"from\"/>")
        << int(QXmppRosterIq::Item::From);
    QTest::newRow("to")
        << QByteArray("<item subscription=\"to\"/>")
        << int(QXmppRosterIq::Item::To);
    QTest::newRow("both")
        << QByteArray("<item subscription=\"both\"/>")
        << int(QXmppRosterIq::Item::Both);
}

void tst_QXmppRosterIq::testItem()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, subscriptionType);

    QXmppRosterIq::Item item;
    parsePacket(item, xml);
    QCOMPARE(int(item.subscriptionType()), subscriptionType);
    serializePacket(item, xml);
}

