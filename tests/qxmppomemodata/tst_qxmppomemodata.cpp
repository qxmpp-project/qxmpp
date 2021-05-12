/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Germán Márquez Mejía
 *  Melvin Keskin
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

#include "QXmppOmemoDeviceBundle.h"
#include "QXmppOmemoDeviceList.h"
#include "QXmppOmemoDeviceListElement.h"

#include "util.h"

class tst_QXmppOmemoData : public QObject
{
    Q_OBJECT

private slots:
    void testIsOmemoDeviceListElement_data();
    void testIsOmemoDeviceListElement();
    void testOmemoDeviceListElement_data();
    void testOmemoDeviceListElement();
    void testIsOmemoDeviceList_data();
    void testIsOmemoDeviceList();
    void testOmemoDeviceList();
    void testIsOmemoDeviceBundle_data();
    void testIsOmemoDeviceBundle();
    void testOmemoDeviceBundle();
};

void tst_QXmppOmemoData::testIsOmemoDeviceListElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<device xmlns=\"urn:xmpp:omemo:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<device xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppOmemoData::testIsOmemoDeviceListElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceListElement::isOmemoDeviceListElement(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceListElement_data() {
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("id");
    QTest::addColumn<QString>("label");

    QTest::newRow("id")
        << QByteArrayLiteral("<device id=\"12345\"/>")
        << int(12345)
        << QString();
    QTest::newRow("idAndLabel")
        << QByteArrayLiteral("<device id=\"4223\" label=\"Gajim on Ubuntu Linux\"/>")
        << int(4223)
        << QStringLiteral("Gajim on Ubuntu Linux");
}

void tst_QXmppOmemoData::testOmemoDeviceListElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, id);
    QFETCH(QString, label);

    QXmppOmemoDeviceListElement deviceListElement1;
    parsePacket(deviceListElement1, xml);
    QCOMPARE(deviceListElement1.id(), id);
    QCOMPARE(deviceListElement1.label(), label);
    serializePacket(deviceListElement1, xml);

    QXmppOmemoDeviceListElement deviceListElement2;
    deviceListElement2.setId(id);
    deviceListElement2.setLabel(label);
    QCOMPARE(deviceListElement2.id(), id);
    QCOMPARE(deviceListElement2.label(), label);
}

void tst_QXmppOmemoData::testIsOmemoDeviceList_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<devices xmlns=\"urn:xmpp:omemo:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<devices xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppOmemoData::testIsOmemoDeviceList()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceList::isOmemoDeviceList(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceList()
{
    const QByteArray xml(QByteArrayLiteral(
        "<devices xmlns=\"urn:xmpp:omemo:1\">"
        "<device id=\"12345\"/>"
        "<device id=\"4223\" label=\"Gajim on Ubuntu Linux\"/>"
        "</devices>"
    ));

    QXmppOmemoDeviceListElement deviceListElement1;
    deviceListElement1.setId(12345);

    QXmppOmemoDeviceListElement deviceListElement2;
    deviceListElement2.setId(4223);
    deviceListElement2.setLabel(QStringLiteral("Gajim on Ubuntu Linux"));

    QXmppOmemoDeviceList deviceList1;
    parsePacket(deviceList1, xml);
    QCOMPARE(deviceList1.size(), 2);
    QVERIFY(deviceList1.contains(deviceListElement1));
    QVERIFY(deviceList1.contains(deviceListElement2));
    serializePacket(deviceList1, xml);

    QXmppOmemoDeviceList deviceList2;
    deviceList2.append(deviceListElement1);
    deviceList2.append(deviceListElement2);
    QCOMPARE(deviceList2.size(), 2);
    QVERIFY(deviceList2.contains(deviceListElement1));
    QVERIFY(deviceList2.contains(deviceListElement2));
}

void tst_QXmppOmemoData::testIsOmemoDeviceBundle_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<bundle xmlns=\"urn:xmpp:omemo:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<bundle xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppOmemoData::testIsOmemoDeviceBundle()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceBundle::isOmemoDeviceBundle(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceBundle()
{
    const QByteArray xml(QByteArrayLiteral(
        "<bundle xmlns=\"urn:xmpp:omemo:1\">"
        "<ik>a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</ik>"
        "<spk id=\"1\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</spk>"
        "<spks>PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</spks>"
        "<prekeys>"
        "<pk id=\"1\">eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK</pk>"
        "<pk id=\"2\">aDRHdkcxNDNYUmJSNWVObnNWd0RCSzE1QlVKVGQ1RVEK</pk>"
        "</prekeys>"
        "</bundle>"
    ));

    const QMap<int, QByteArray> expectedPublicPreKeys = {
        {1, QByteArray::fromBase64(QByteArrayLiteral("eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK"))},
        {2, QByteArray::fromBase64(QByteArrayLiteral("aDRHdkcxNDNYUmJSNWVObnNWd0RCSzE1QlVKVGQ1RVEK"))}
    };

    QXmppOmemoDeviceBundle deviceBundle1;
    parsePacket(deviceBundle1, xml);
    QCOMPARE(deviceBundle1.publicIdentityKey().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    QCOMPARE(deviceBundle1.signedPublicPreKeyId(), 1);
    QCOMPARE(deviceBundle1.signedPublicPreKey().toBase64(), QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    QCOMPARE(deviceBundle1.signedPublicPreKeySignature().toBase64(), QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));
    QCOMPARE(deviceBundle1.publicPreKeys(), expectedPublicPreKeys);
    serializePacket(deviceBundle1, xml);

    QXmppOmemoDeviceBundle deviceBundle2;
    deviceBundle2.setPublicIdentityKey(QByteArray::fromBase64(QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK")));
    deviceBundle2.setSignedPublicPreKeyId(1);
    deviceBundle2.setSignedPublicPreKey(QByteArray::fromBase64(QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK")));
    deviceBundle2.setSignedPublicPreKeySignature(QByteArray::fromBase64(QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K")));
    deviceBundle2.setPublicPreKeys(expectedPublicPreKeys);
    QCOMPARE(deviceBundle2.publicIdentityKey().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    QCOMPARE(deviceBundle2.signedPublicPreKeyId(), 1);
    QCOMPARE(deviceBundle2.signedPublicPreKey().toBase64(), QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    QCOMPARE(deviceBundle2.signedPublicPreKeySignature().toBase64(), QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));
    QCOMPARE(deviceBundle2.publicPreKeys(), expectedPublicPreKeys);
}

QTEST_MAIN(tst_QXmppOmemoData)
#include "tst_qxmppomemodata.moc"
