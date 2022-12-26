// SPDX-FileCopyrightText: 2021 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessage.h"
#include "QXmppOmemoDeviceBundle_p.h"
#include "QXmppOmemoDeviceElement_p.h"
#include "QXmppOmemoDeviceList_p.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoEnvelope_p.h"
#include "QXmppOmemoIq_p.h"

#include "util.h"
#include <QObject>

///
/// Serializes data and compares it to multiple XML representations.
/// It fails if no comparison succeeds.
///
template<class T>
static void serializePacket(T &packet, const QVector<QByteArray> &xmls)
{
    auto isSerializationSuccessful = false;
    const auto data = packetToXml(packet);

    for (const auto &xml : xmls) {
        auto processedXml = xml;
        processedXml.replace(u'\'', u'"');

        qDebug() << "expect " << processedXml;
        qDebug() << "writing" << data;

        if (data == processedXml) {
            isSerializationSuccessful = true;
            break;
        }
    }

    QVERIFY2(isSerializationSuccessful, "No XML data equals the serialized packet");
}

class tst_QXmppOmemoData : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testIsOmemoDeviceElement_data();
    Q_SLOT void testIsOmemoDeviceElement();
    Q_SLOT void testOmemoDeviceElement_data();
    Q_SLOT void testOmemoDeviceElement();
    Q_SLOT void testIsOmemoDeviceList_data();
    Q_SLOT void testIsOmemoDeviceList();
    Q_SLOT void testOmemoDeviceList();
    Q_SLOT void testIsOmemoDeviceBundle_data();
    Q_SLOT void testIsOmemoDeviceBundle();
    Q_SLOT void testOmemoDeviceBundle();
    Q_SLOT void testIsOmemoEnvelope_data();
    Q_SLOT void testIsOmemoEnvelope();
    Q_SLOT void testOmemoEnvelope_data();
    Q_SLOT void testOmemoEnvelope();
    Q_SLOT void testIsOmemoElement_data();
    Q_SLOT void testIsOmemoElement();
    Q_SLOT void testOmemoElement();
    Q_SLOT void testMessageOmemoElement();
    Q_SLOT void testOmemoIq();
};

void tst_QXmppOmemoData::testIsOmemoDeviceElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<device xmlns=\"urn:xmpp:omemo:2\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:2\"/>")
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
    QVERIFY(doc.setContent(xml, true));
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

void tst_QXmppOmemoData::testIsOmemoDeviceList_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<devices xmlns=\"urn:xmpp:omemo:2\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:2\"/>")
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
    QVERIFY(doc.setContent(xml, true));
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceList::isOmemoDeviceList(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceList()
{
    const QByteArray xml(QByteArrayLiteral(
        "<devices xmlns=\"urn:xmpp:omemo:2\">"
        "<device id=\"12345\"/>"
        "<device id=\"4223\" label=\"Gajim on Ubuntu Linux\"/>"
        "</devices>"));

    QXmppOmemoDeviceElement deviceElement1;
    deviceElement1.setId(12345);

    QXmppOmemoDeviceElement deviceElement2;
    deviceElement2.setId(4223);
    deviceElement2.setLabel(QStringLiteral("Gajim on Ubuntu Linux"));

    QXmppOmemoDeviceList deviceList1;
    parsePacket(deviceList1, xml);
    QCOMPARE(deviceList1.size(), 2);
    QVERIFY(deviceList1.contains(deviceElement1));
    QVERIFY(deviceList1.contains(deviceElement2));
    serializePacket(deviceList1, xml);

    QXmppOmemoDeviceList deviceList2;
    deviceList2.append(deviceElement1);
    deviceList2.append(deviceElement2);
    QCOMPARE(deviceList2.size(), 2);
    QVERIFY(deviceList2.contains(deviceElement1));
    QVERIFY(deviceList2.contains(deviceElement2));
    serializePacket(deviceList2, xml);
}

void tst_QXmppOmemoData::testIsOmemoDeviceBundle_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<bundle xmlns=\"urn:xmpp:omemo:2\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:2\"/>")
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
    QVERIFY(doc.setContent(xml, true));
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoDeviceBundle::isOmemoDeviceBundle(element), isValid);
}

void tst_QXmppOmemoData::testOmemoDeviceBundle()
{
    const QByteArray xml1(QByteArrayLiteral(
        "<bundle xmlns=\"urn:xmpp:omemo:2\">"
        "<ik>a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</ik>"
        "<spk id=\"1\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</spk>"
        "<spks>PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</spks>"
        "<prekeys>"
        "<pk id=\"1\">eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK</pk>"
        "<pk id=\"2\">aDRHdkcxNDNYUmJSNWVObnNWd0RCSzE1QlVKVGQ1RVEK</pk>"
        "</prekeys>"
        "</bundle>"));

    // The pre keys can be sorted in the reverse order since it is not fixed.
    // Thus, another representation is used for the comparision made by
    // serializePacket().
    const QByteArray xml2(QByteArrayLiteral(
        "<bundle xmlns=\"urn:xmpp:omemo:2\">"
        "<ik>a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</ik>"
        "<spk id=\"1\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</spk>"
        "<spks>PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</spks>"
        "<prekeys>"
        "<pk id=\"2\">aDRHdkcxNDNYUmJSNWVObnNWd0RCSzE1QlVKVGQ1RVEK</pk>"
        "<pk id=\"1\">eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK</pk>"
        "</prekeys>"
        "</bundle>"));

    const QByteArray xmlWithSinglePreKey(QByteArrayLiteral(
        "<bundle xmlns=\"urn:xmpp:omemo:2\">"
        "<ik>a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</ik>"
        "<spk id=\"1\">Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK</spk>"
        "<spks>PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</spks>"
        "<prekeys>"
        "<pk id=\"1\">eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK</pk>"
        "</prekeys>"
        "</bundle>"));

    const auto xmls = QVector({ xml1, xml2 });

    QHash<uint32_t, QByteArray> expectedPublicPreKeys = {
        { 1, QByteArray::fromBase64(QByteArrayLiteral("eDM2cnBiTmo4MmRGQ1RYTkZ0YnVwajJtNWdPdzkxZ0gK")) },
        { 2, QByteArray::fromBase64(QByteArrayLiteral("aDRHdkcxNDNYUmJSNWVObnNWd0RCSzE1QlVKVGQ1RVEK")) }
    };

    QXmppOmemoDeviceBundle deviceBundle1;
    parsePacket(deviceBundle1, xml1);
    QCOMPARE(deviceBundle1.publicIdentityKey().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    QCOMPARE(deviceBundle1.signedPublicPreKeyId(), uint32_t(1));
    QCOMPARE(deviceBundle1.signedPublicPreKey().toBase64(), QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    QCOMPARE(deviceBundle1.signedPublicPreKeySignature().toBase64(), QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));
    QCOMPARE(deviceBundle1.publicPreKeys(), expectedPublicPreKeys);
    serializePacket(deviceBundle1, xmls);

    QXmppOmemoDeviceBundle deviceBundle2;
    deviceBundle2.setPublicIdentityKey(QByteArray::fromBase64(QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK")));
    deviceBundle2.setSignedPublicPreKeyId(1);
    deviceBundle2.setSignedPublicPreKey(QByteArray::fromBase64(QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK")));
    deviceBundle2.setSignedPublicPreKeySignature(QByteArray::fromBase64(QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K")));
    deviceBundle2.addPublicPreKey(1, expectedPublicPreKeys.value(1));
    deviceBundle2.addPublicPreKey(2, expectedPublicPreKeys.value(2));
    QCOMPARE(deviceBundle2.publicIdentityKey().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    QCOMPARE(deviceBundle2.signedPublicPreKeyId(), uint32_t(1));
    QCOMPARE(deviceBundle2.signedPublicPreKey().toBase64(), QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    QCOMPARE(deviceBundle2.signedPublicPreKeySignature().toBase64(), QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));
    QCOMPARE(deviceBundle2.publicPreKeys(), expectedPublicPreKeys);
    serializePacket(deviceBundle2, xmls);

    deviceBundle2.removePublicPreKey(2);
    expectedPublicPreKeys.remove(2);
    QCOMPARE(deviceBundle2.publicPreKeys(), expectedPublicPreKeys);
    serializePacket(deviceBundle2, xmlWithSinglePreKey);
}

void tst_QXmppOmemoData::testIsOmemoEnvelope_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<key xmlns=\"urn:xmpp:omemo:2\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:2\"/>")
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
    QVERIFY(doc.setContent(xml, true));
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoEnvelope::isOmemoEnvelope(element), isValid);
}

void tst_QXmppOmemoData::testOmemoEnvelope_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<uint32_t>("recipientDeviceId");
    QTest::addColumn<bool>("isUsedForKeyExchange");
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("keyAndHmac")
        << QByteArrayLiteral("<key rid=\"1337\">PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K</key>")
        << uint32_t(1337)
        << false
        << QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K");
    QTest::newRow("keyExchange")
        << QByteArrayLiteral("<key rid=\"12321\" kex=\"true\">a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK</key>")
        << uint32_t(12321)
        << true
        << QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK");
}

void tst_QXmppOmemoData::testOmemoEnvelope()
{
    QFETCH(QByteArray, xml);
    QFETCH(uint32_t, recipientDeviceId);
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
    omemoEnvelope2.setData(QByteArray::fromBase64(data));
    QCOMPARE(omemoEnvelope2.recipientDeviceId(), recipientDeviceId);
    QCOMPARE(omemoEnvelope2.isUsedForKeyExchange(), isUsedForKeyExchange);
    QCOMPARE(omemoEnvelope2.data().toBase64(), data);
    serializePacket(omemoEnvelope2, xml);
}

void tst_QXmppOmemoData::testIsOmemoElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<encrypted xmlns=\"urn:xmpp:omemo:2\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:omemo:2\"/>")
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
    QVERIFY(doc.setContent(xml, true));
    const QDomElement element = doc.documentElement();
    QCOMPARE(QXmppOmemoElement::isOmemoElement(element), isValid);
}

void tst_QXmppOmemoData::testOmemoElement()
{
    const QByteArray xmlIn(QByteArrayLiteral(
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
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
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
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

    QCOMPARE(omemoElement1.senderDeviceId(), uint32_t(27183));

    const auto omemoEnvelope1 = omemoElement1.searchEnvelope(("juliet@capulet.lit"), 31415);
    QVERIFY(omemoEnvelope1);
    QCOMPARE(omemoEnvelope1->recipientDeviceId(), uint32_t(31415));
    QVERIFY(!omemoEnvelope1->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope1->data().toBase64(), QByteArrayLiteral("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));

    const auto omemoEnvelope2 = omemoElement1.searchEnvelope(QStringLiteral("romeo@montague.lit"), 12321);
    QVERIFY(omemoEnvelope2);
    QCOMPARE(omemoEnvelope2->recipientDeviceId(), uint32_t(12321));
    QVERIFY(omemoEnvelope2->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope2->data().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));

    const auto omemoEnvelope3 = omemoElement1.searchEnvelope(QStringLiteral("romeo@montague.lit"), 1337);
    QVERIFY(omemoEnvelope3);
    QCOMPARE(omemoEnvelope3->recipientDeviceId(), uint32_t(1337));
    QVERIFY(!omemoEnvelope3->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope3->data().toBase64(), QByteArrayLiteral("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));

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
            "RWw1P0AmIUAlQjk7ZFBWXW1RbWxoTFE+cUxMbk5UCg=="));

    serializePacket(omemoElement1, xmlOut);

    QXmppOmemoElement omemoElement2;
    omemoElement2.setSenderDeviceId(27183);
    omemoElement2.setPayload(
        QByteArray::fromBase64(QByteArrayLiteral(
            "Vk9NPi99bHFWKmErOUVTTkAwW1VcZjJvPlElZWUoOk90Kz03YUF7OHc/WjpaQz9ieFdsZjBsSH1w"
            "R1d2Zzt1bEFAMSZqP0dVJj9oaygmcWRPKGU3Kjc8aV4sJSlpSXBqaENCT2NUVFFmaFNXbCxQaHsj"
            "OnthQDJyUW9qNjwoZCtpLzpzLGpbKlJRY1NtMVVeRzdsOWRQciNnXV9tajEyWztnKiEhRHs5K2hX"
            "ZFloaEZtUENTQWIxM0tcVkxIVWY+aGYoeEk/SldZcyNlTzk2Q2NHW1NqWEhEPmhPXl1WZV5xNE9p"
            "WDZuck8zPGE2Rk4vKWJXd3F1YV0mSXA/NVNGNEQsK18mTlJNbl9WcGJXcVE5e1E0dlFAPVQ8THM+"
            "QjdcdjZSNDVJclo0QVo6cDBMQDtVcUFnNDpcd1ZXSkcsXz82QjhXLl9NSVBFdipeOmF4NC5YKnNx"
            "K2dxMGx1MDkrdnJhWTovUjk1ZCZUUSNTKHIvJUgmTyE4bjJbZlZAPl9IZi8ucSM7a2FAQWUzXUJO"
            "LmpALilFWGRqYlh1Siw2MzJqbipsWlZRMG91MGVQVlExLCFeayMuM3dfSn1ONiU8LixZWSx3YUlV"
            "bGtIcnVWP2Y0LGwvTzFIQy8qZVVBSVZLS1peSW0xNTRPcXRDIXBkXnhmWyNxQFxHQ19cYXVAO214"
            "RWw1P0AmIUAlQjk7ZFBWXW1RbWxoTFE+cUxMbk5UCg==")));

    QXmppOmemoEnvelope omemoEnvelope4;
    omemoEnvelope4.setRecipientDeviceId(31415);
    omemoEnvelope4.setData(QByteArray::fromBase64("Oy5TSG9vVVV4Wz9wUkUvI1lUXiVLIU5bbGIsUV0wRngK"));
    omemoElement2.addEnvelope(QStringLiteral("juliet@capulet.lit"), omemoEnvelope4);

    QXmppOmemoEnvelope omemoEnvelope5;
    omemoEnvelope5.setRecipientDeviceId(12321);
    omemoEnvelope5.setIsUsedForKeyExchange(true);
    omemoEnvelope5.setData(QByteArray::fromBase64("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));
    omemoElement2.addEnvelope(QStringLiteral("romeo@montague.lit"), omemoEnvelope5);

    QXmppOmemoEnvelope omemoEnvelope6;
    omemoEnvelope6.setRecipientDeviceId(1337);
    omemoEnvelope6.setData(QByteArray::fromBase64("PTEoSk91VnRZSXBzcFlPXy4jZ3NKcGVZZ2d3YVJbVj8K"));
    omemoElement2.addEnvelope(QStringLiteral("romeo@montague.lit"), omemoEnvelope6);

    QCOMPARE(omemoElement2.senderDeviceId(), uint32_t(27183));

    const auto omemoEnvelope7 = omemoElement2.searchEnvelope(QStringLiteral("romeo@montague.lit"), 12321);
    QVERIFY(omemoEnvelope7);
    QCOMPARE(omemoEnvelope7->recipientDeviceId(), uint32_t(12321));
    QVERIFY(omemoEnvelope7->isUsedForKeyExchange());
    QCOMPARE(omemoEnvelope7->data().toBase64(), QByteArrayLiteral("a012U0R9WixWKUYhYipucnZOWG06akFOR3Q1NGNOOmUK"));

    const auto omemoEnvelope8 = omemoElement2.searchEnvelope(QStringLiteral("juliet@capulet.lit"), 31415);
    QVERIFY(omemoEnvelope8);
    QVERIFY(!omemoEnvelope8->isUsedForKeyExchange());

    serializePacket(omemoElement2, xmlIn);
}

void tst_QXmppOmemoData::testMessageOmemoElement()
{
    const QByteArray xmlIn(QByteArrayLiteral(
        "<message id=\"send1\" to=\"juliet@capulet.lit\" from=\"romeo@montague.lit\" type=\"chat\">"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
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
        "</encrypted>"
        "</message>"));

    // An OMEMO element having its OMEMO envelopes sorted in reverse order is
    // needed since they are serialized in the reverse order in which they are
    // deserialized.
    const QByteArray xmlOut1(QByteArrayLiteral(
        "<message id=\"send1\" to=\"juliet@capulet.lit\" from=\"romeo@montague.lit\" type=\"chat\">"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
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
        "</encrypted>"
        "</message>"));

    const QByteArray xmlOut2(QByteArrayLiteral(
        "<message type=\"chat\">"
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
        "<header sid=\"0\"/>"
        "</encrypted>"
        "</message>"));

    QXmppMessage message1;
    QVERIFY(!message1.omemoElement());

    parsePacket(message1, xmlIn);
    QVERIFY(message1.omemoElement());
    serializePacket(message1, xmlOut1);

    QXmppMessage message2;
    message2.setOmemoElement(QXmppOmemoElement());
    QVERIFY(message2.omemoElement());
    serializePacket(message2, xmlOut2);
}

void tst_QXmppOmemoData::testOmemoIq()
{
    const QByteArray xmlOtherIq(
        "<iq id=\"qxmpp2\" type=\"get\">"
        "<encrypted xmlns=\"urn:xmpp:encryption:stub:sce:1\">"
        "<payload>"
        "V2FpdCwgd2hhdD8gQXJlIHlvdSBzZXJpb3VzPyBEaWQgeW91IHJlYWxseSBqdXN0IGdyYWIgeW91"
        "ciBmYXZvdXJpdGUgYmFzZTY0IGRlY29kZXIganVzdCB0byBjaGVjayB0aGlzIGRvY3VtZW50IGZv"
        "ciBoaWRkZW4gbWVzc2FnZXM/IFdoYXQgYXJlIHlvdSBzb21lIGtpbmQgb2YgbmVyZD8gU29tZSBn"
        "ZWVrIHdpdGggYSBiaW5hcnkgd3Jpc3Qgd2F0Y2g/"
        "</payload>"
        "</encrypted>"
        "</iq>");

    const QByteArray xmlOmemoIq(
        "<iq id=\"qxmpp2\" type=\"get\">"
        "<encrypted xmlns=\"urn:xmpp:omemo:2\">"
        "<header sid=\"27183\"/>"
        "<payload>"
        "V2FpdCwgd2hhdD8gQXJlIHlvdSBzZXJpb3VzPyBEaWQgeW91IHJlYWxseSBqdXN0IGdyYWIgeW91"
        "ciBmYXZvdXJpdGUgYmFzZTY0IGRlY29kZXIganVzdCB0byBjaGVjayB0aGlzIGRvY3VtZW50IGZv"
        "ciBoaWRkZW4gbWVzc2FnZXM/IFdoYXQgYXJlIHlvdSBzb21lIGtpbmQgb2YgbmVyZD8gU29tZSBn"
        "ZWVrIHdpdGggYSBiaW5hcnkgd3Jpc3Qgd2F0Y2g/"
        "</payload>"
        "</encrypted>"
        "</iq>");

    const QByteArray omemoPayload(
        "V2FpdCwgd2hhdD8gQXJlIHlvdSBzZXJpb3VzPyBEaWQgeW91IHJlYWxseSBqdXN0IGdyYWIgeW91"
        "ciBmYXZvdXJpdGUgYmFzZTY0IGRlY29kZXIganVzdCB0byBjaGVjayB0aGlzIGRvY3VtZW50IGZv"
        "ciBoaWRkZW4gbWVzc2FnZXM/IFdoYXQgYXJlIHlvdSBzb21lIGtpbmQgb2YgbmVyZD8gU29tZSBn"
        "ZWVrIHdpdGggYSBiaW5hcnkgd3Jpc3Qgd2F0Y2g/");

    QDomDocument doc;
    doc.setContent(xmlOtherIq, true);
    QDomElement element = doc.documentElement();
    QVERIFY(!QXmppOmemoIq::isOmemoIq(element));

    doc.setContent(xmlOmemoIq, true);
    element = doc.documentElement();
    QVERIFY(QXmppOmemoIq::isOmemoIq(element));

    QXmppOmemoIq omemoIq1;
    QVERIFY(omemoIq1.omemoElement().payload().isEmpty());

    parsePacket(omemoIq1, xmlOmemoIq);
    QCOMPARE(omemoIq1.omemoElement().payload(), QByteArray::fromBase64(omemoPayload));
    serializePacket(omemoIq1, xmlOmemoIq);

    QXmppOmemoElement omemoElement;
    omemoElement.setSenderDeviceId(27183);
    omemoElement.setPayload(QByteArray::fromBase64(omemoPayload));

    QXmppOmemoIq omemoIq2;
    omemoIq2.setOmemoElement(omemoElement);
    QCOMPARE(omemoIq2.omemoElement().payload(), QByteArray::fromBase64(omemoPayload));
    serializePacket(omemoIq2, xmlOmemoIq);
}

QTEST_MAIN(tst_QXmppOmemoData)
#include "tst_qxmppomemodata.moc"
