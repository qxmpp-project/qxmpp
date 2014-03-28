/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#include "QXmppNonSASLAuth.h"
#include "util.h"

class tst_QXmppNonSASLAuthIq : public QObject
{
    Q_OBJECT

private slots:
    void testGet();
    void testSetPlain();
    void testSetDigest();
};

void tst_QXmppNonSASLAuthIq::testGet()
{
    // Client requests authentication fields from server
    const QByteArray xml(
        "<iq id=\"auth1\" to=\"shakespeare.lit\" type=\"get\">"
        "<query xmlns=\"jabber:iq:auth\"/>"
        "</iq>");

    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    serializePacket(iq, xml);
}

void tst_QXmppNonSASLAuthIq::testSetPlain()
{
    // Client provides required information (plain)
    const QByteArray xml(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<password>Calli0pe</password>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.digest(), QByteArray());
    QCOMPARE(iq.password(), QLatin1String("Calli0pe"));
    QCOMPARE(iq.resource(), QLatin1String("globe"));
    serializePacket(iq, xml);
}

void tst_QXmppNonSASLAuthIq::testSetDigest()
{
    // Client provides required information (digest)
    const QByteArray xml(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<digest>48fc78be9ec8f86d8ce1c39c320c97c21d62334d</digest>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.digest(), QByteArray("\x48\xfc\x78\xbe\x9e\xc8\xf8\x6d\x8c\xe1\xc3\x9c\x32\x0c\x97\xc2\x1d\x62\x33\x4d"));
    QCOMPARE(iq.password(), QString());
    QCOMPARE(iq.resource(), QLatin1String("globe"));
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppNonSASLAuthIq)
#include "tst_qxmppnonsaslauthiq.moc"
