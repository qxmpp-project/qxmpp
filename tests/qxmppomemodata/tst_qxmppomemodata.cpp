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
#include <QXmppOmemoElement.h>
#include <QXmppOmemoEnvelope.h>

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
    void testIsOmemoEnvelope_data();
    void testIsOmemoEnvelope();
    void testOmemoEnvelope_data();
    void testOmemoEnvelope();
    void testIsOmemoElement_data();
    void testIsOmemoElement();
    void testOmemoElement();
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

void tst_QXmppOmemoData::testIsOmemoEnvelope_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<key xmlns=\"urn:xmpp:omemo:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<key xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppOmemoData::testIsOmemoEnvelope()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoEnvelope::isOmemoEnvelope(element), isValid);
}

void tst_QXmppOmemoData::testOmemoEnvelope_data() {
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("recipientDeviceId");
    QTest::addColumn<bool>("isUsedForKeyExchange");
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("keyAndHmac")
        << QByteArrayLiteral("<key rid=\"1337\">PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</key>")
        << 1337
        << false
        << QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K");
    QTest::newRow("keyExchange")
        << QByteArrayLiteral("<key rid=\"12321\" kex=\"true\">a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</key>")
        << 12321
        << true
        << QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK");
}

void tst_QXmppOmemoData::testOmemoEnvelope()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, recipientDeviceId);
    QFETCH(bool, isUsedForKeyExchange);
    QFETCH(QByteArray, data);

    QXmppOmemoEnvelope omemoEnvelope1;
    parsePacket(omemoEnvelope1, xml);
    QCOMPARE(omemoEnvelope1.recipientDeviceId(), recipientDeviceId);
    QCOMPARE(omemoEnvelope1.isUsedForKeyExchange(), isUsedForKeyExchange);
    QCOMPARE(omemoEnvelope1.data().toBase64(), data);
    serializePacket(omemoEnvelope1, xml);

    QXmppOmemoEnvelope omemoEnvelope2;
    omemoEnvelope2.setRecipientDeviceId(recipientDeviceId);
    omemoEnvelope2.setIsUsedForKeyExchange(isUsedForKeyExchange);
    omemoEnvelope2.setData(data);
    QCOMPARE(omemoEnvelope2.recipientDeviceId(), recipientDeviceId);
    QCOMPARE(omemoEnvelope2.isUsedForKeyExchange(), isUsedForKeyExchange);
    QCOMPARE(omemoEnvelope2.data(), data);
}

void tst_QXmppOmemoData::testIsOmemoElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<encrypted xmlns=\"urn:xmpp:omemo:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<encrypted xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppOmemoData::testIsOmemoElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoElement::isOmemoElement(element), isValid);
}

void tst_QXmppOmemoData::testOmemoElement()
{
    const QByteArray xmlIn(QByteArrayLiteral(
        "<encrypted xmlns=\"urn:xmpp:omemo:1\">"
        "<header sid=\"27183\">"
        "<keys jid=\"juliet@capulet.lit\">"
        "<key rid=\"31415\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</key>"
        "</keys>"
        "<keys jid=\"romeo@montague.lit\">"
        "<key rid=\"1337\">PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</key>"
        "<key rid=\"12321\" kex=\"true\">a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</key>"
        "</keys>"
        "</header>"
        "<payload>"
        "Vk9NPi99bHFWKmErOUVTTkAwW1VcZjJvPlElZWUoOk90Kz03YUF7OHc/WjpaQz9ieFdsZjBsSH1w"
        "R1d2Zzt1bEFAMSZqP0dVJj9oaygmcWRPKGU3Kjc8aV4sJSlpSXBqaENCT2NUVFFmaFNXbCxQaHsj"
        "OnthQDJyUW9qNjwoZCtpLzpzLGpbKlJRY1NtMVVeRzdsOWRQciNnXV9tajEyWztnKiEhRHs5K2hX"
        "ZFloaEZtUENTQWIxM0tcVkxIVWY+aGYoeEk/SldZcyNlTzk2Q2NHW1NqWEhEPmhPXl1WZV5xNE9p"
        "WDZuck8zPGE2Rk4vKWJXd3F1YV0mSXA/NVNGNEQsK18mTlJNbl9WcGJXcVE5e1E0dlFAPVQ8THM+"
        "QjdcdjZSNDVJclo0QVo6cDBMQDtVcUFnNDpcd1ZXSkcsXz82QjhXLl9NSVBFdipeOmF4NC5YKnNx"
        "K2dxMGx1MDkrdnJhWTovUjk1ZCZUUSNTKHIvJUgmTyE4bjJbZlZAPl9IZi8ucSM7a2FAQWUzXUJO"
        "LmpALilFWGRqYlh1Siw2MzJqbipsWlZRMG91MGVQVlExLCFeayMuM3dfSn1ONiU8LixZWSx3YUlV"
        "bGtIcnVWP2Y0LGwvTzFIQy8qZVVBSVZLS1peSW0xNTRPcXRDIXBkXnhmWyNxQFxHQ19cYXVAO214"
        "RWw1P0AmIUAlQjk7ZFBWXW1RbWxoTFE+cUxMbk5UCg=="
        "</payload>"
        "</encrypted>"));

    // An OMEMO element having its OMEMO envelopes sorted in reverse order is
    // needed since they are serialized in the reverse order in which they are
    // deserialized.
    const QByteArray xmlOut(QByteArrayLiteral(
        "<encrypted xmlns=\"urn:xmpp:omemo:1\">"
        "<header sid=\"27183\">"
        "<keys jid=\"juliet@capulet.lit\">"
        "<key rid=\"31415\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</key>"
        "</keys>"
        "<keys jid=\"romeo@montague.lit\">"
        "<key rid=\"12321\" kex=\"true\">a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</key>"
        "<key rid=\"1337\">PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</key>"
        "</keys>"
        "</header>"
        "<payload>"
        "Vk9NPi99bHFWKmErOUVTTkAwW1VcZjJvPlElZWUoOk90Kz03YUF7OHc/WjpaQz9ieFdsZjBsSH1w"
        "R1d2Zzt1bEFAMSZqP0dVJj9oaygmcWRPKGU3Kjc8aV4sJSlpSXBqaENCT2NUVFFmaFNXbCxQaHsj"
        "OnthQDJyUW9qNjwoZCtpLzpzLGpbKlJRY1NtMVVeRzdsOWRQciNnXV9tajEyWztnKiEhRHs5K2hX"
        "ZFloaEZtUENTQWIxM0tcVkxIVWY+aGYoeEk/SldZcyNlTzk2Q2NHW1NqWEhEPmhPXl1WZV5xNE9p"
        "WDZuck8zPGE2Rk4vKWJXd3F1YV0mSXA/NVNGNEQsK18mTlJNbl9WcGJXcVE5e1E0dlFAPVQ8THM+"
        "QjdcdjZSNDVJclo0QVo6cDBMQDtVcUFnNDpcd1ZXSkcsXz82QjhXLl9NSVBFdipeOmF4NC5YKnNx"
        "K2dxMGx1MDkrdnJhWTovUjk1ZCZUUSNTKHIvJUgmTyE4bjJbZlZAPl9IZi8ucSM7a2FAQWUzXUJO"
        "LmpALilFWGRqYlh1Siw2MzJqbipsWlZRMG91MGVQVlExLCFeayMuM3dfSn1ONiU8LixZWSx3YUlV"
        "bGtIcnVWP2Y0LGwvTzFIQy8qZVVBSVZLS1peSW0xNTRPcXRDIXBkXnhmWyNxQFxHQ19cYXVAO214"
        "RWw1P0AmIUAlQjk7ZFBWXW1RbWxoTFE+cUxMbk5UCg=="
        "</payload>"
        "</encrypted>"));

    QXmppOmemoElement omemoElement1;
    parsePacket(omemoElement1, xmlIn);

    QCOMPARE(omemoElement1.senderDeviceId(), 27183);

    const auto omemoEnvelope1 = omemoElement1.searchEnvelope(QStringLiteral("romeo@montague.lit"), 12321);
    QVERIFY(omemoEnvelope1);
    QCOMPARE(omemoEnvelope1->recipientDeviceId(), 12321);
    QVERIFY(omemoEnvelope1->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope1->data().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));

    const auto omemoEnvelope2 = omemoElement1.searchEnvelope(("juliet@capulet.lit"), 31415);
    QVERIFY(omemoEnvelope2);
    QVERIFY(!omemoEnvelope2->isUsedForKeyExchange());

    QCOMPARE(
        omemoElement1.payload().toBase64(),
        QByteArrayLiteral(
             "Vk9NPi99bHFWKmErOUVTTkAwW1VcZjJvPlElZWUoOk90Kz03YUF7OHc/WjpaQz9ieFdsZjBsSH1w"
             "R1d2Zzt1bEFAMSZqP0dVJj9oaygmcWRPKGU3Kjc8aV4sJSlpSXBqaENCT2NUVFFmaFNXbCxQaHsj"
             "OnthQDJyUW9qNjwoZCtpLzpzLGpbKlJRY1NtMVVeRzdsOWRQciNnXV9tajEyWztnKiEhRHs5K2hX"
             "ZFloaEZtUENTQWIxM0tcVkxIVWY+aGYoeEk/SldZcyNlTzk2Q2NHW1NqWEhEPmhPXl1WZV5xNE9p"
             "WDZuck8zPGE2Rk4vKWJXd3F1YV0mSXA/NVNGNEQsK18mTlJNbl9WcGJXcVE5e1E0dlFAPVQ8THM+"
             "QjdcdjZSNDVJclo0QVo6cDBMQDtVcUFnNDpcd1ZXSkcsXz82QjhXLl9NSVBFdipeOmF4NC5YKnNx"
             "K2dxMGx1MDkrdnJhWTovUjk1ZCZUUSNTKHIvJUgmTyE4bjJbZlZAPl9IZi8ucSM7a2FAQWUzXUJO"
             "LmpALilFWGRqYlh1Siw2MzJqbipsWlZRMG91MGVQVlExLCFeayMuM3dfSn1ONiU8LixZWSx3YUlV"
             "bGtIcnVWP2Y0LGwvTzFIQy8qZVVBSVZLS1peSW0xNTRPcXRDIXBkXnhmWyNxQFxHQ19cYXVAO214"
             "RWw1P0AmIUAlQjk7ZFBWXW1RbWxoTFE+cUxMbk5UCg=="
        )
    );

    serializePacket(omemoElement1, xmlOut);

    QXmppOmemoElement omemoElement2;
    omemoElement2.setSenderDeviceId(27138);

    QXmppOmemoEnvelope omemoEnvelope3;
    omemoEnvelope3.setRecipientDeviceId(12321);
    omemoEnvelope3.setIsUsedForKeyExchange(true);
    omemoEnvelope3.setData(QByteArray::fromBase64("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    omemoElement2.addEnvelope(QStringLiteral("romeo@montague.lit"), omemoEnvelope3);

    QXmppOmemoEnvelope omemoEnvelope4;
    omemoEnvelope4.setRecipientDeviceId(31415);
    omemoEnvelope4.setData(QByteArray::fromBase64("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    omemoElement2.addEnvelope(QStringLiteral("juliet@capulet.lit"), omemoEnvelope4);

    QCOMPARE(omemoElement2.senderDeviceId(), 27138);

    const auto omemoEnvelope5 = omemoElement2.searchEnvelope(QStringLiteral("romeo@montague.lit"), 12321);
    QVERIFY(omemoEnvelope5);
    QCOMPARE(omemoEnvelope5->recipientDeviceId(), 12321);
    QVERIFY(omemoEnvelope5->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope5->data().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));

    const auto omemoEnvelope6 = omemoElement2.searchEnvelope(QStringLiteral("juliet@capulet.lit"), 31415);
    QVERIFY(omemoEnvelope6);
    QVERIFY(!omemoEnvelope6->isUsedForKeyExchange());
}

QTEST_MAIN(tst_QXmppOmemoData)
#include "tst_qxmppomemodata.moc"
