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
#include "QXmppStanza.h"
#include "util.h"

class tst_QXmppStanza : public QObject
{
    Q_OBJECT

private slots:
    void testExtendedAddress_data();
    void testExtendedAddress();
};

void tst_QXmppStanza::testExtendedAddress_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("delivered");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("type");

    QTest::newRow("simple")
        << QByteArray("<address jid=\"foo@example.com/QXmpp\" type=\"bcc\"/>")
        << false
        << QString()
        << QString("foo@example.com/QXmpp")
        << QString("bcc");

    QTest::newRow("full")
        << QByteArray("<address delivered=\"true\" desc=\"some description\" jid=\"foo@example.com/QXmpp\" type=\"bcc\"/>")
        << true
        << QString("some description")
        << QString("foo@example.com/QXmpp")
        << QString("bcc");
}

void tst_QXmppStanza::testExtendedAddress()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, delivered);
    QFETCH(QString, description);
    QFETCH(QString, jid);
    QFETCH(QString, type);

    QXmppExtendedAddress address;
    parsePacket(address, xml);
    QCOMPARE(address.isDelivered(), delivered);
    QCOMPARE(address.description(), description);
    QCOMPARE(address.jid(), jid);
    QCOMPARE(address.type(), type);
    serializePacket(address, xml);
}

QTEST_MAIN(tst_QXmppStanza)
#include "tst_qxmppstanza.moc"
