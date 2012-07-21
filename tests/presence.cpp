/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Olivier Goffart
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

#include "QXmppPresence.h"

#include "presence.h"
#include "tests.h"

void tst_QXmppPresence::testPresence_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("priority");
    QTest::addColumn<int>("statusType");
    QTest::addColumn<QString>("statusText");
    QTest::addColumn<int>("vcardUpdate");
    QTest::addColumn<QByteArray>("photoHash");

    QTest::newRow("empty") << QByteArray("<presence/>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("unavailable") << QByteArray("<presence type=\"unavailable\"/>") << int(QXmppPresence::Unavailable) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("error") << QByteArray("<presence type=\"error\"/>") << int(QXmppPresence::Error) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    
    QTest::newRow("full") << QByteArray("<presence><show>away</show><status>In a meeting</status><priority>5</priority></presence>") << int(QXmppPresence::Available) << 5 << int(QXmppPresence::Away) << "In a meeting" << int(QXmppPresence::VCardUpdateNone) << QByteArray();

    // status type
    QTest::newRow("away") << QByteArray("<presence><show>away</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Away) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("dnd") << QByteArray("<presence><show>dnd</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::DND) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("chat") << QByteArray("<presence><show>chat</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Chat) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("xa") << QByteArray("<presence><show>xa</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::XA) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("invisible") << QByteArray("<presence><show>invisible</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Invisible) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();

    QTest::newRow("vcard-photo") << QByteArray(
        "<presence>"
        "<x xmlns=\"vcard-temp:x:update\">"
        "<photo>73b908bc</photo>"
        "</x>"
        "</presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateValidPhoto) << QByteArray::fromHex("73b908bc");

    QTest::newRow("vard-not-ready") << QByteArray(
        "<presence>"
        "<x xmlns=\"vcard-temp:x:update\"/>"
        "</presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNotReady) << QByteArray();
}

void tst_QXmppPresence::testPresence()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);
    QFETCH(int, priority);
    QFETCH(int, statusType);
    QFETCH(QString, statusText);
    QFETCH(int, vcardUpdate);
    QFETCH(QByteArray, photoHash);

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(int(presence.type()), type);
    QCOMPARE(presence.priority(), priority);
    QCOMPARE(int(presence.availableStatusType()), statusType);
    QCOMPARE(presence.statusText(), statusText);
    QCOMPARE(int(presence.vCardUpdateType()), vcardUpdate);
    QCOMPARE(presence.photoHash(), photoHash);

    // legacy
    QCOMPARE(presence.status().priority(), priority);
    QCOMPARE(int(presence.status().type()), statusType);
    QCOMPARE(presence.status().statusText(), statusText);

    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithCapability()
{
    const QByteArray xml(
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\">"
        "<show>away</show>"
        "<status>In a meeting</status>"
        "<priority>5</priority>"
        "<x xmlns=\"vcard-temp:x:update\">"
        "<photo>73b908bc</photo>"
        "</x>"
        "<c xmlns=\"http://jabber.org/protocol/caps\" hash=\"sha-1\" node=\"http://code.google.com/p/qxmpp\" ver=\"QgayPKawpkPSDYmwT/WM94uAlu0=\"/>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(presence.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(presence.availableStatusType(), QXmppPresence::Away);
    QCOMPARE(presence.statusText(), QString("In a meeting"));
    QCOMPARE(presence.priority(), 5);
    QCOMPARE(presence.photoHash(), QByteArray::fromHex("73b908bc"));
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateValidPhoto);
    QCOMPARE(presence.capabilityHash(), QString("sha-1"));
    QCOMPARE(presence.capabilityNode(), QString("http://code.google.com/p/qxmpp"));
    QCOMPARE(presence.capabilityVer(), QByteArray::fromBase64("QgayPKawpkPSDYmwT/WM94uAlu0="));

    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithMuc()
{
    const QByteArray xml(
        "<presence "
            "to=\"pistol@shakespeare.lit/harfleur\" "
            "from=\"harfleur@henryv.shakespeare.lit/pistol\" "
            "type=\"unavailable\">"
            "<x xmlns=\"http://jabber.org/protocol/muc#user\">"
            "<item affiliation=\"none\" role=\"none\">"
                "<actor jid=\"fluellen@shakespeare.lit\"/>"
                "<reason>Avaunt, you cullion!</reason>"
            "</item>"
            "<status code=\"307\"/>"
            "</x>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QLatin1String("pistol@shakespeare.lit/harfleur"));
    QCOMPARE(presence.from(), QLatin1String("harfleur@henryv.shakespeare.lit/pistol"));
    QCOMPARE(presence.type(), QXmppPresence::Unavailable);
    QCOMPARE(presence.mucItem().actor(), QLatin1String("fluellen@shakespeare.lit"));
    QCOMPARE(presence.mucItem().affiliation(), QXmppMucItem::NoAffiliation);
    QCOMPARE(presence.mucItem().jid(), QString());
    QCOMPARE(presence.mucItem().reason(), QLatin1String("Avaunt, you cullion!"));
    QCOMPARE(presence.mucItem().role(), QXmppMucItem::NoRole);
    QCOMPARE(presence.mucStatusCodes(), QList<int>() << 307);
    serializePacket(presence, xml);
}

