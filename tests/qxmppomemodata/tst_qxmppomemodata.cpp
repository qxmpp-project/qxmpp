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

#include "QXmppOmemoDeviceElement.h"

#include "util.h"
#include <QObject>

class tst_QXmppOmemoData : public QObject
{
    Q_OBJECT

private slots:
    void testIsOmemoDeviceElement_data();
    void testIsOmemoDeviceElement();
    void testOmemoDeviceElement_data();
    void testOmemoDeviceElement();
};

void tst_QXmppOmemoData::testIsOmemoDeviceElement_data()
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

void tst_QXmppOmemoData::testIsOmemoDeviceElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceElement::isOmemoDeviceElement(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<uint32_t>("id");
    QTest::addColumn<QString>("label");

    QTest::newRow("id")
        << QByteArrayLiteral("<device id=\"12345\"/>")
        << uint32_t(12345)
        << QString();
    QTest::newRow("idAndLabel")
        << QByteArrayLiteral("<device id=\"4223\" label=\"Gajim on Ubuntu Linux\"/>")
        << uint32_t(4223)
        << QStringLiteral("Gajim on Ubuntu Linux");
}

void tst_QXmppOmemoData::testOmemoDeviceElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(uint32_t, id);
    QFETCH(QString, label);

    QXmppOmemoDeviceElement deviceElement1;
    parsePacket(deviceElement1, xml);
    QCOMPARE(deviceElement1.id(), id);
    QCOMPARE(deviceElement1.label(), label);
    serializePacket(deviceElement1, xml);

    QXmppOmemoDeviceElement deviceElement2;
    deviceElement2.setId(id);
    deviceElement2.setLabel(label);
    QCOMPARE(deviceElement2.id(), id);
    QCOMPARE(deviceElement2.label(), label);
    serializePacket(deviceElement2, xml);
}

QTEST_MAIN(tst_QXmppOmemoData)
#include "tst_qxmppomemodata.moc"
