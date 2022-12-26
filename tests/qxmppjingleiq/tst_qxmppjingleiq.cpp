// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppJingleIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppJingleIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testIsSdpParameter_data();
    Q_SLOT void testIsSdpParameter();
    Q_SLOT void testSdpParameter();
    Q_SLOT void testSdpParameterWithoutValue();
    Q_SLOT void testIsRtpCryptoElement_data();
    Q_SLOT void testIsRtpCryptoElement();
    Q_SLOT void testRtpCryptoElement_data();
    Q_SLOT void testRtpCryptoElement();
    Q_SLOT void testIsRtpEncryption_data();
    Q_SLOT void testIsRtpEncryption();
    Q_SLOT void testRtpEncryption_data();
    Q_SLOT void testRtpEncryption();
    Q_SLOT void testIsRtpFeedbackProperty_data();
    Q_SLOT void testIsRtpFeedbackProperty();
    Q_SLOT void testRtpFeedbackProperty();
    Q_SLOT void testRtpFeedbackPropertyWithParameters();
    Q_SLOT void testIsRtpFeedbackInterval_data();
    Q_SLOT void testIsRtpFeedbackInterval();
    Q_SLOT void testRtpFeedbackInterval();
    Q_SLOT void testIsRtpHeaderExtensionProperty_data();
    Q_SLOT void testIsRtpHeaderExtensionProperty();
    Q_SLOT void testRtpHeaderExtensionProperty();
    Q_SLOT void testRtpHeaderExtensionPropertyWithSenders();
    Q_SLOT void testRtpHeaderExtensionPropertyWithParameters();
    Q_SLOT void testCandidate();
    Q_SLOT void testContent();
    Q_SLOT void testContentFingerprint();
    Q_SLOT void testContentSdp();
    Q_SLOT void testContentSdpReflexive();
    Q_SLOT void testContentSdpFingerprint();
    Q_SLOT void testContentSdpParameters();
    Q_SLOT void testContentRtpFeedbackNegotiation();
    Q_SLOT void testContentRtpHeaderExtensionsNegotiation();
    Q_SLOT void testSession();
    Q_SLOT void testTerminate();
    Q_SLOT void testRtpSessionState_data();
    Q_SLOT void testRtpSessionState();
    Q_SLOT void testAudioPayloadType();
    Q_SLOT void testVideoPayloadType();
    Q_SLOT void testPayloadTypeRtpFeedbackNegotiation();
    Q_SLOT void testRtpErrorCondition_data();
    Q_SLOT void testRtpErrorCondition();
};

void tst_QXmppJingleIq::testIsSdpParameter_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<parameter name=\"test-name\" value=\"test-value\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid name=\"test-name\" value=\"test-value\"/>")
        << false;
}

void tst_QXmppJingleIq::testIsSdpParameter()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppSdpParameter::isSdpParameter(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testSdpParameter()
{
    const QByteArray xml("<parameter name=\"test-name\" value=\"test-value\"/>");

    QXmppSdpParameter parameter1;
    QVERIFY(parameter1.name().isEmpty());
    QVERIFY(parameter1.value().isEmpty());

    parsePacket(parameter1, xml);
    QCOMPARE(parameter1.name(), QStringLiteral("test-name"));
    QCOMPARE(parameter1.value(), QStringLiteral("test-value"));

    serializePacket(parameter1, xml);

    QXmppSdpParameter parameter2;
    parameter2.setName(QStringLiteral("test-name"));
    parameter2.setValue(QStringLiteral("test-value"));

    serializePacket(parameter2, xml);
}

void tst_QXmppJingleIq::testSdpParameterWithoutValue()
{
    const QByteArray xml("<parameter name=\"test-name\"/>");

    QXmppSdpParameter parameter1;

    parsePacket(parameter1, xml);
    QCOMPARE(parameter1.name(), QStringLiteral("test-name"));
    QVERIFY(parameter1.value().isEmpty());

    serializePacket(parameter1, xml);

    QXmppSdpParameter parameter2;
    parameter2.setName(QStringLiteral("test-name"));

    serializePacket(parameter2, xml);
}

void tst_QXmppJingleIq::testIsRtpCryptoElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<crypto/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid/>")
        << false;
}

void tst_QXmppJingleIq::testIsRtpCryptoElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppJingleRtpCryptoElement::isJingleRtpCryptoElement(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testRtpCryptoElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("hasSessionParams");

    QTest::newRow("withoutSessionParams")
        << QByteArrayLiteral("<crypto"
                             " tag=\"1\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>")
        << false;
    QTest::newRow("withSessionParams")
        << QByteArrayLiteral("<crypto"
                             " tag=\"1\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\""
                             " session-params=\"KDR=1 UNENCRYPTED_SRTCP\"/>")
        << true;
}

void tst_QXmppJingleIq::testRtpCryptoElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, hasSessionParams);

    QXmppJingleRtpCryptoElement rtpCryptoElement1;
    QCOMPARE(rtpCryptoElement1.tag(), uint32_t(0));
    QVERIFY(rtpCryptoElement1.cryptoSuite().isEmpty());
    QVERIFY(rtpCryptoElement1.keyParams().isEmpty());
    QVERIFY(rtpCryptoElement1.sessionParams().isEmpty());
    parsePacket(rtpCryptoElement1, xml);

    QCOMPARE(rtpCryptoElement1.tag(), uint32_t(1));
    QCOMPARE(rtpCryptoElement1.cryptoSuite(), QStringLiteral("AES_CM_128_HMAC_SHA1_80"));
    QCOMPARE(rtpCryptoElement1.keyParams(), QStringLiteral("inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32"));
    if (hasSessionParams) {
        QCOMPARE(rtpCryptoElement1.sessionParams(), QStringLiteral("KDR=1 UNENCRYPTED_SRTCP"));
    } else {
        QVERIFY(rtpCryptoElement1.sessionParams().isEmpty());
    }

    serializePacket(rtpCryptoElement1, xml);

    QXmppJingleRtpCryptoElement rtpCryptoElement2;
    rtpCryptoElement2.setTag(1);
    rtpCryptoElement2.setCryptoSuite(QStringLiteral("AES_CM_128_HMAC_SHA1_80"));
    rtpCryptoElement2.setKeyParams(QStringLiteral("inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32"));

    if (hasSessionParams) {
        rtpCryptoElement2.setSessionParams(QStringLiteral("KDR=1 UNENCRYPTED_SRTCP"));
    }

    QCOMPARE(rtpCryptoElement2.tag(), uint32_t(1));
    QCOMPARE(rtpCryptoElement2.cryptoSuite(), QStringLiteral("AES_CM_128_HMAC_SHA1_80"));
    QCOMPARE(rtpCryptoElement2.keyParams(), QStringLiteral("inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32"));
    if (hasSessionParams) {
        QCOMPARE(rtpCryptoElement2.sessionParams(), QStringLiteral("KDR=1 UNENCRYPTED_SRTCP"));
    } else {
        QVERIFY(rtpCryptoElement2.sessionParams().isEmpty());
    }

    serializePacket(rtpCryptoElement2, xml);
}

void tst_QXmppJingleIq::testIsRtpEncryption_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<encryption xmlns=\"urn:xmpp:jingle:apps:rtp:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:jingle:apps:rtp:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<encryption xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppJingleIq::testIsRtpEncryption()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppJingleRtpEncryption::isJingleRtpEncryption(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testRtpEncryption_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isRequired");
    QTest::addColumn<int>("cryptoElementCount");

    QTest::newRow("required")
        << QByteArrayLiteral("<encryption xmlns=\"urn:xmpp:jingle:apps:rtp:1\" required=\"1\">"
                             "<crypto"
                             " tag=\"1\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>"
                             "</encryption>")
        << true
        << 1;
    QTest::newRow("optional")
        << QByteArrayLiteral("<encryption xmlns=\"urn:xmpp:jingle:apps:rtp:1\">"
                             "<crypto"
                             " tag=\"1\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>"
                             "</encryption>")
        << false
        << 1;
    QTest::newRow("optionalWithMultipleCryptoElements")
        << QByteArrayLiteral("<encryption xmlns=\"urn:xmpp:jingle:apps:rtp:1\">"
                             "<crypto"
                             " tag=\"1\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>"
                             "<crypto"
                             " tag=\"2\""
                             " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
                             " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>"
                             "</encryption>")
        << false
        << 2;
}

void tst_QXmppJingleIq::testRtpEncryption()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isRequired);
    QFETCH(int, cryptoElementCount);

    QXmppJingleRtpEncryption rtpEncryption1;
    QVERIFY(!rtpEncryption1.isRequired());
    QVERIFY(rtpEncryption1.cryptoElements().isEmpty());

    parsePacket(rtpEncryption1, xml);

    QCOMPARE(rtpEncryption1.isRequired(), isRequired);
    QCOMPARE(rtpEncryption1.cryptoElements().size(), cryptoElementCount);

    serializePacket(rtpEncryption1, xml);

    QXmppJingleRtpCryptoElement rtpCryptoElement2;
    rtpCryptoElement2.setTag(1);
    rtpCryptoElement2.setCryptoSuite(QStringLiteral("AES_CM_128_HMAC_SHA1_80"));
    rtpCryptoElement2.setKeyParams(QStringLiteral("inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32"));

    QXmppJingleRtpEncryption rtpEncryption2;
    rtpEncryption2.setRequired(isRequired);

    if (cryptoElementCount == 2) {
        auto rtpCryptoElement3 = rtpCryptoElement2;
        rtpCryptoElement3.setTag(2);

        rtpEncryption2.setCryptoElements({ rtpCryptoElement2, rtpCryptoElement3 });
    } else {
        rtpEncryption2.setCryptoElements({ rtpCryptoElement2 });
    }

    QCOMPARE(rtpEncryption2.isRequired(), isRequired);
    QCOMPARE(rtpEncryption2.cryptoElements().size(), cryptoElementCount);
    QCOMPARE(rtpEncryption2.cryptoElements().at(0).tag(), uint32_t(1));

    if (cryptoElementCount == 2) {
        QCOMPARE(rtpEncryption2.cryptoElements().at(1).tag(), uint32_t(2));
    }

    serializePacket(rtpEncryption2, xml);
}

void tst_QXmppJingleIq::testIsRtpFeedbackProperty_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<rtcp-fb xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppJingleIq::testIsRtpFeedbackProperty()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppJingleRtpFeedbackProperty::isJingleRtpFeedbackProperty(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testRtpFeedbackProperty()
{
    const QByteArray xml("<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"nack\" subtype=\"sli\"/>");

    QXmppJingleRtpFeedbackProperty property1;
    QVERIFY(property1.type().isEmpty());
    QVERIFY(property1.subtype().isEmpty());

    parsePacket(property1, xml);
    QCOMPARE(property1.type(), QStringLiteral("nack"));
    QCOMPARE(property1.subtype(), QStringLiteral("sli"));

    serializePacket(property1, xml);

    QXmppJingleRtpFeedbackProperty property2;
    property2.setType(QStringLiteral("nack"));
    property2.setSubtype(QStringLiteral("sli"));

    QCOMPARE(property1.type(), QStringLiteral("nack"));
    QCOMPARE(property1.subtype(), QStringLiteral("sli"));

    serializePacket(property2, xml);
}

void tst_QXmppJingleIq::testRtpFeedbackPropertyWithParameters()
{
    const QByteArray xml(
        "<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"test-type\">"
        "<parameter name=\"test-name-1\"/>"
        "<parameter name=\"test-name-2\"/>"
        "</rtcp-fb>");

    QXmppJingleRtpFeedbackProperty property1;

    parsePacket(property1, xml);
    QCOMPARE(property1.type(), QStringLiteral("test-type"));
    QVERIFY(property1.subtype().isEmpty());
    QCOMPARE(property1.parameters().size(), 2);
    QCOMPARE(property1.parameters().at(0).name(), QStringLiteral("test-name-1"));
    QCOMPARE(property1.parameters().at(1).name(), QStringLiteral("test-name-2"));

    serializePacket(property1, xml);

    QXmppJingleRtpFeedbackProperty property2;
    property2.setType(QStringLiteral("test-type"));

    QXmppSdpParameter parameter1;
    parameter1.setName(QStringLiteral("test-name-1"));

    QXmppSdpParameter parameter2;
    parameter2.setName(QStringLiteral("test-name-2"));

    property2.setParameters({ parameter1, parameter2 });

    QCOMPARE(property2.parameters().size(), 2);
    QCOMPARE(property2.parameters().at(0).name(), QStringLiteral("test-name-1"));
    QCOMPARE(property2.parameters().at(1).name(), QStringLiteral("test-name-2"));

    serializePacket(property2, xml);
}

void tst_QXmppJingleIq::testIsRtpFeedbackInterval_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<rtcp-fb-trr-int xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<rtcp-fb-trr-int xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppJingleIq::testIsRtpFeedbackInterval()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppJingleRtpFeedbackInterval::isJingleRtpFeedbackInterval(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testRtpFeedbackInterval()
{
    const QByteArray xml("<rtcp-fb-trr-int xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" value=\"100\"/>");

    QXmppJingleRtpFeedbackInterval interval1;

    parsePacket(interval1, xml);
    QCOMPARE(interval1.value(), uint64_t(100));

    serializePacket(interval1, xml);

    QXmppJingleRtpFeedbackInterval interval2;
    interval2.setValue(100);

    QCOMPARE(interval1.value(), uint64_t(100));

    serializePacket(interval2, xml);
}

void tst_QXmppJingleIq::testIsRtpHeaderExtensionProperty_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<rtp-hdrext xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppJingleIq::testIsRtpHeaderExtensionProperty()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppJingleRtpHeaderExtensionProperty::isJingleRtpHeaderExtensionProperty(xmlToDom(xml)), isValid);
}

void tst_QXmppJingleIq::testRtpHeaderExtensionProperty()
{
    const QByteArray xml("<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\" id=\"1\" uri=\"urn:ietf:params:rtp-hdrext:toffset\"/>");

    QXmppJingleRtpHeaderExtensionProperty property1;
    QCOMPARE(property1.id(), uint32_t(0));
    QVERIFY(property1.uri().isEmpty());
    QCOMPARE(property1.senders(), QXmppJingleRtpHeaderExtensionProperty::Both);

    parsePacket(property1, xml);
    QCOMPARE(property1.id(), uint32_t(1));
    QCOMPARE(property1.uri(), QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));
    QCOMPARE(property1.senders(), QXmppJingleRtpHeaderExtensionProperty::Both);

    serializePacket(property1, xml);

    QXmppJingleRtpHeaderExtensionProperty property2;
    property2.setId(1);
    property2.setUri(QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));
    property2.setSenders(QXmppJingleRtpHeaderExtensionProperty::Both);

    QCOMPARE(property1.id(), uint32_t(1));
    QCOMPARE(property1.uri(), QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));
    QCOMPARE(property1.senders(), QXmppJingleRtpHeaderExtensionProperty::Both);

    serializePacket(property2, xml);
}

void tst_QXmppJingleIq::testRtpHeaderExtensionPropertyWithSenders()
{
    const QByteArray xml("<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\" id=\"1\" uri=\"urn:ietf:params:rtp-hdrext:toffset\" senders=\"initiator\"/>");

    QXmppJingleRtpHeaderExtensionProperty property1;

    parsePacket(property1, xml);
    QCOMPARE(property1.senders(), QXmppJingleRtpHeaderExtensionProperty::Initiator);

    serializePacket(property1, xml);

    QXmppJingleRtpHeaderExtensionProperty property2;
    property2.setId(1);
    property2.setUri(QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));
    property2.setSenders(QXmppJingleRtpHeaderExtensionProperty::Initiator);

    QCOMPARE(property1.senders(), QXmppJingleRtpHeaderExtensionProperty::Initiator);

    serializePacket(property2, xml);
}

void tst_QXmppJingleIq::testRtpHeaderExtensionPropertyWithParameters()
{
    const QByteArray xml(
        "<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\" id=\"1\" uri=\"urn:ietf:params:rtp-hdrext:toffset\">"
        "<parameter name=\"test-name-1\"/>"
        "<parameter name=\"test-name-2\"/>"
        "</rtp-hdrext>");

    QXmppJingleRtpHeaderExtensionProperty property1;

    parsePacket(property1, xml);
    QCOMPARE(property1.parameters().size(), 2);
    QCOMPARE(property1.parameters().at(0).name(), QStringLiteral("test-name-1"));
    QCOMPARE(property1.parameters().at(1).name(), QStringLiteral("test-name-2"));

    serializePacket(property1, xml);

    QXmppJingleRtpHeaderExtensionProperty property2;
    property2.setId(1);
    property2.setUri(QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));

    QXmppSdpParameter parameter1;
    parameter1.setName(QStringLiteral("test-name-1"));

    QXmppSdpParameter parameter2;
    parameter2.setName(QStringLiteral("test-name-2"));

    property2.setParameters({ parameter1, parameter2 });

    QCOMPARE(property2.parameters().size(), 2);
    QCOMPARE(property2.parameters().at(0).name(), QStringLiteral("test-name-1"));
    QCOMPARE(property2.parameters().at(1).name(), QStringLiteral("test-name-2"));

    serializePacket(property2, xml);
}

void tst_QXmppJingleIq::testCandidate()
{
    const QByteArray xml(
        "<candidate component=\"1\""
        " foundation=\"1\""
        " generation=\"0\""
        " id=\"el0747fg11\""
        " ip=\"10.0.1.1\""
        " network=\"1\""
        " port=\"8998\""
        " priority=\"2130706431\""
        " protocol=\"udp\""
        " type=\"host\"/>");

    QXmppJingleCandidate candidate;
    parsePacket(candidate, xml);
    QCOMPARE(candidate.foundation(), QLatin1String("1"));
    QCOMPARE(candidate.generation(), 0);
    QCOMPARE(candidate.id(), QLatin1String("el0747fg11"));
    QCOMPARE(candidate.host(), QHostAddress("10.0.1.1"));
    QCOMPARE(candidate.network(), 1);
    QCOMPARE(candidate.port(), quint16(8998));
    QCOMPARE(candidate.priority(), 2130706431);
    QCOMPARE(candidate.protocol(), QLatin1String("udp"));
    QCOMPARE(candidate.type(), QXmppJingleCandidate::HostType);
    serializePacket(candidate, xml);
};

void tst_QXmppJingleIq::testContent()
{
    const QByteArray xml(
        "<content creator=\"initiator\" name=\"voice\">"
        "<description xmlns=\"urn:xmpp:jingle:apps:rtp:1\" media=\"audio\">"
        "<rtcp-mux/>"
        "<encryption xmlns=\"urn:xmpp:jingle:apps:rtp:1\">"
        "<crypto"
        " tag=\"1\""
        " crypto-suite=\"AES_CM_128_HMAC_SHA1_80\""
        " key-params=\"inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32\"/>"
        "</encryption>"
        "<payload-type id=\"96\"/>"
        "<payload-type id=\"97\"/>"
        "</description>"
        "<transport xmlns=\"urn:xmpp:jingle:transports:ice-udp:1\""
        " ufrag=\"8hhy\""
        " pwd=\"asd88fgpdd777uzjYhagZg\">"
        "<candidate component=\"0\""
        " generation=\"0\""
        " id=\"el0747fg11\""
        " network=\"0\""
        " port=\"0\""
        " priority=\"0\""
        " type=\"host\"/>"
        "<candidate component=\"0\""
        " generation=\"0\""
        " id=\"y3s2b30v3r\""
        " network=\"0\""
        " port=\"0\""
        " priority=\"0\""
        " type=\"host\"/>"
        "</transport>"
        "</content>");

    QXmppJingleIq::Content content1;
    QVERIFY(content1.creator().isEmpty());
    QVERIFY(content1.name().isEmpty());
    QVERIFY(content1.descriptionMedia().isEmpty());
    QCOMPARE(content1.descriptionSsrc(), quint32(0));
    QVERIFY(!content1.isRtpMultiplexingSupported());
    QVERIFY(!content1.rtpEncryption());
    QCOMPARE(content1.payloadTypes().size(), 0);
    QVERIFY(content1.transportUser().isEmpty());
    QVERIFY(content1.transportPassword().isEmpty());
    QCOMPARE(content1.transportCandidates().size(), 0);
    parsePacket(content1, xml);

    QCOMPARE(content1.creator(), QStringLiteral("initiator"));
    QCOMPARE(content1.name(), QStringLiteral("voice"));
    QCOMPARE(content1.descriptionMedia(), QStringLiteral("audio"));
    QCOMPARE(content1.descriptionSsrc(), quint32(0));
    QVERIFY(content1.isRtpMultiplexingSupported());
    QVERIFY(content1.rtpEncryption());
    QCOMPARE(content1.payloadTypes().size(), 2);
    QCOMPARE(content1.payloadTypes().at(0).id(), quint8(96));
    QCOMPARE(content1.payloadTypes().at(1).id(), quint8(97));
    QCOMPARE(content1.transportUser(), QStringLiteral("8hhy"));
    QCOMPARE(content1.transportPassword(), QStringLiteral("asd88fgpdd777uzjYhagZg"));
    QCOMPARE(content1.transportCandidates().size(), 2);
    QCOMPARE(content1.transportCandidates().at(0).id(), QStringLiteral("el0747fg11"));
    QCOMPARE(content1.transportCandidates().at(1).id(), QStringLiteral("y3s2b30v3r"));
    serializePacket(content1, xml);

    QXmppJingleIq::Content content2;
    content2.setCreator(QStringLiteral("initiator"));
    content2.setName(QStringLiteral("voice"));
    content2.setDescriptionMedia(QStringLiteral("audio"));
    content2.setDescriptionSsrc(quint32(0));
    content2.setRtpMultiplexingSupported(true);
    QXmppJingleRtpCryptoElement rtpCryptoElement;
    rtpCryptoElement.setTag(1);
    rtpCryptoElement.setCryptoSuite(QStringLiteral("AES_CM_128_HMAC_SHA1_80"));
    rtpCryptoElement.setKeyParams(QStringLiteral("inline:WVNfX19zZW1jdGwgKCkgewkyMjA7fQp9CnVubGVz|2^20|1:32"));
    QXmppJingleRtpEncryption rtpEncryption;
    rtpEncryption.setCryptoElements({ rtpCryptoElement });
    content2.setRtpEncryption(rtpEncryption);
    QXmppJinglePayloadType payloadType1;
    payloadType1.setId(quint8(96));
    content2.setPayloadTypes({ payloadType1 });
    QXmppJinglePayloadType payloadType2;
    payloadType2.setId(quint8(97));
    content2.addPayloadType(payloadType2);
    content2.setTransportUser(QStringLiteral("8hhy"));
    content2.setTransportPassword(QStringLiteral("asd88fgpdd777uzjYhagZg"));
    QXmppJingleCandidate transportCandidate1;
    transportCandidate1.setId(QStringLiteral("el0747fg11"));
    content2.setTransportCandidates({ transportCandidate1 });
    QXmppJingleCandidate transportCandidate2;
    transportCandidate2.setId(QStringLiteral("y3s2b30v3r"));
    content2.addTransportCandidate(transportCandidate2);

    QCOMPARE(content2.creator(), QStringLiteral("initiator"));
    QCOMPARE(content2.name(), QStringLiteral("voice"));
    QCOMPARE(content2.descriptionMedia(), QStringLiteral("audio"));
    QCOMPARE(content2.descriptionSsrc(), quint32(0));
    QVERIFY(content2.isRtpMultiplexingSupported());
    QVERIFY(content2.rtpEncryption());
    QCOMPARE(content2.payloadTypes().size(), 2);
    QCOMPARE(content2.payloadTypes().at(0).id(), quint8(96));
    QCOMPARE(content2.payloadTypes().at(1).id(), quint8(97));
    QCOMPARE(content2.transportUser(), QStringLiteral("8hhy"));
    QCOMPARE(content2.transportPassword(), QStringLiteral("asd88fgpdd777uzjYhagZg"));
    QCOMPARE(content2.transportCandidates().size(), 2);
    QCOMPARE(content2.transportCandidates().at(0).id(), QStringLiteral("el0747fg11"));
    QCOMPARE(content2.transportCandidates().at(1).id(), QStringLiteral("y3s2b30v3r"));
    serializePacket(content2, xml);
}

void tst_QXmppJingleIq::testContentFingerprint()
{
    const QByteArray xml(
        "<content creator=\"initiator\" name=\"voice\">"
        "<description xmlns=\"urn:xmpp:jingle:apps:rtp:1\" media=\"audio\">"
        "<payload-type id=\"0\" name=\"PCMU\"/>"
        "</description>"
        "<transport xmlns=\"urn:xmpp:jingle:transports:ice-udp:1\""
        " ufrag=\"8hhy\""
        " pwd=\"asd88fgpdd777uzjYhagZg\">"
        "<candidate component=\"1\""
        " foundation=\"1\""
        " generation=\"0\""
        " id=\"el0747fg11\""
        " ip=\"10.0.1.1\""
        " network=\"1\""
        " port=\"8998\""
        " priority=\"2130706431\""
        " protocol=\"udp\""
        " type=\"host\"/>"
        "<fingerprint xmlns=\"urn:xmpp:jingle:apps:dtls:0\" hash=\"sha-256\" setup=\"actpass\">"
        "02:1A:CC:54:27:AB:EB:9C:53:3F:3E:4B:65:2E:7D:46:3F:54:42:CD:54:F1:7A:03:A2:7D:F9:B0:7F:46:19:B2"
        "</fingerprint>"
        "</transport>"
        "</content>");

    QXmppJingleIq::Content content;
    parsePacket(content, xml);

    QCOMPARE(content.creator(), QLatin1String("initiator"));
    QCOMPARE(content.name(), QLatin1String("voice"));
    QCOMPARE(content.descriptionMedia(), QLatin1String("audio"));
    QCOMPARE(content.descriptionSsrc(), quint32(0));
    QCOMPARE(content.payloadTypes().size(), 1);
    QCOMPARE(content.payloadTypes()[0].id(), quint8(0));
    QCOMPARE(content.transportCandidates().size(), 1);
    QCOMPARE(content.transportCandidates()[0].component(), 1);
    QCOMPARE(content.transportCandidates()[0].foundation(), QLatin1String("1"));
    QCOMPARE(content.transportCandidates()[0].host(), QHostAddress("10.0.1.1"));
    QCOMPARE(content.transportCandidates()[0].port(), quint16(8998));
    QCOMPARE(content.transportCandidates()[0].priority(), 2130706431);
    QCOMPARE(content.transportCandidates()[0].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[0].type(), QXmppJingleCandidate::HostType);
    QCOMPARE(content.transportUser(), QLatin1String("8hhy"));
    QCOMPARE(content.transportPassword(), QLatin1String("asd88fgpdd777uzjYhagZg"));
    QCOMPARE(content.transportFingerprint(), QByteArray::fromHex("021acc5427abeb9c533f3e4b652e7d463f5442cd54f17a03a27df9b07f4619b2"));
    QCOMPARE(content.transportFingerprintHash(), QLatin1String("sha-256"));
    QCOMPARE(content.transportFingerprintSetup(), QLatin1String("actpass"));

    serializePacket(content, xml);
}

void tst_QXmppJingleIq::testContentSdp()
{
    const QString sdp(
        "m=audio 8998 RTP/AVP 96 97 18 0 103 98\r\n"
        "c=IN IP4 10.0.1.1\r\n"
        "a=rtpmap:96 speex/16000\r\n"
        "a=rtpmap:97 speex/8000\r\n"
        "a=rtpmap:18 G729/0\r\n"
        "a=rtpmap:0 PCMU/0\r\n"
        "a=rtpmap:103 L16/16000/2\r\n"
        "a=rtpmap:98 x-ISAC/8000\r\n"
        "a=candidate:1 1 udp 2130706431 10.0.1.1 8998 typ host generation 0\r\n"
        "a=candidate:2 1 udp 1694498815 192.0.2.3 45664 typ host generation 0\r\n"
        "a=ice-ufrag:8hhy\r\n"
        "a=ice-pwd:asd88fgpdd777uzjYhagZg\r\n");

    QXmppJingleIq::Content content;
    QVERIFY(content.parseSdp(sdp));

    QCOMPARE(content.descriptionMedia(), QLatin1String("audio"));
    QCOMPARE(content.descriptionSsrc(), quint32(0));
    QCOMPARE(content.payloadTypes().size(), 6);
    QCOMPARE(content.payloadTypes()[0].id(), quint8(96));
    QCOMPARE(content.payloadTypes()[1].id(), quint8(97));
    QCOMPARE(content.payloadTypes()[2].id(), quint8(18));
    QCOMPARE(content.payloadTypes()[3].id(), quint8(0));
    QCOMPARE(content.payloadTypes()[4].id(), quint8(103));
    QCOMPARE(content.payloadTypes()[5].id(), quint8(98));
    QCOMPARE(content.transportCandidates().size(), 2);
    QCOMPARE(content.transportCandidates()[0].component(), 1);
    QCOMPARE(content.transportCandidates()[0].foundation(), QLatin1String("1"));
    QCOMPARE(content.transportCandidates()[0].host(), QHostAddress("10.0.1.1"));
    QCOMPARE(content.transportCandidates()[0].port(), quint16(8998));
    QCOMPARE(content.transportCandidates()[0].priority(), 2130706431);
    QCOMPARE(content.transportCandidates()[0].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[0].type(), QXmppJingleCandidate::HostType);
    QCOMPARE(content.transportCandidates()[1].component(), 1);
    QCOMPARE(content.transportCandidates()[1].foundation(), QLatin1String("2"));
    QCOMPARE(content.transportCandidates()[1].host(), QHostAddress("192.0.2.3"));
    QCOMPARE(content.transportCandidates()[1].port(), quint16(45664));
    QCOMPARE(content.transportCandidates()[1].priority(), 1694498815);
    QCOMPARE(content.transportCandidates()[1].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[1].type(), QXmppJingleCandidate::HostType);
    QCOMPARE(content.transportUser(), QLatin1String("8hhy"));
    QCOMPARE(content.transportPassword(), QLatin1String("asd88fgpdd777uzjYhagZg"));

    QCOMPARE(content.toSdp(), sdp);
}

void tst_QXmppJingleIq::testContentSdpReflexive()
{
    const QString sdp(
        "m=audio 45664 RTP/AVP 96 97 18 0 103 98\r\n"
        "c=IN IP4 192.0.2.3\r\n"
        "a=rtpmap:96 speex/16000\r\n"
        "a=rtpmap:97 speex/8000\r\n"
        "a=rtpmap:18 G729/0\r\n"
        "a=rtpmap:0 PCMU/0\r\n"
        "a=rtpmap:103 L16/16000/2\r\n"
        "a=rtpmap:98 x-ISAC/8000\r\n"
        "a=candidate:1 1 udp 2130706431 10.0.1.1 8998 typ host generation 0\r\n"
        "a=candidate:2 1 udp 1694498815 192.0.2.3 45664 typ srflx generation 0\r\n"
        "a=ice-ufrag:8hhy\r\n"
        "a=ice-pwd:asd88fgpdd777uzjYhagZg\r\n");

    QXmppJingleIq::Content content;
    QVERIFY(content.parseSdp(sdp));

    QCOMPARE(content.descriptionMedia(), QLatin1String("audio"));
    QCOMPARE(content.descriptionSsrc(), quint32(0));
    QCOMPARE(content.payloadTypes().size(), 6);
    QCOMPARE(content.payloadTypes()[0].id(), quint8(96));
    QCOMPARE(content.payloadTypes()[1].id(), quint8(97));
    QCOMPARE(content.payloadTypes()[2].id(), quint8(18));
    QCOMPARE(content.payloadTypes()[3].id(), quint8(0));
    QCOMPARE(content.payloadTypes()[4].id(), quint8(103));
    QCOMPARE(content.payloadTypes()[5].id(), quint8(98));
    QCOMPARE(content.transportCandidates().size(), 2);
    QCOMPARE(content.transportCandidates()[0].component(), 1);
    QCOMPARE(content.transportCandidates()[0].foundation(), QLatin1String("1"));
    QCOMPARE(content.transportCandidates()[0].host(), QHostAddress("10.0.1.1"));
    QCOMPARE(content.transportCandidates()[0].port(), quint16(8998));
    QCOMPARE(content.transportCandidates()[0].priority(), 2130706431);
    QCOMPARE(content.transportCandidates()[0].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[0].type(), QXmppJingleCandidate::HostType);
    QCOMPARE(content.transportCandidates()[1].component(), 1);
    QCOMPARE(content.transportCandidates()[1].foundation(), QLatin1String("2"));
    QCOMPARE(content.transportCandidates()[1].host(), QHostAddress("192.0.2.3"));
    QCOMPARE(content.transportCandidates()[1].port(), quint16(45664));
    QCOMPARE(content.transportCandidates()[1].priority(), 1694498815);
    QCOMPARE(content.transportCandidates()[1].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[1].type(), QXmppJingleCandidate::ServerReflexiveType);
    QCOMPARE(content.transportUser(), QLatin1String("8hhy"));
    QCOMPARE(content.transportPassword(), QLatin1String("asd88fgpdd777uzjYhagZg"));

    QCOMPARE(content.toSdp(), sdp);
}

void tst_QXmppJingleIq::testContentSdpFingerprint()
{
    const QString sdp(
        "m=audio 8998 RTP/AVP 96 100\r\n"
        "c=IN IP4 10.0.1.1\r\n"
        "a=rtpmap:96 speex/16000\r\n"
        "a=fmtp:96 cng=on; vbr=on\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=fmtp:100 0-15,66,70\r\n"
        "a=candidate:1 1 udp 2130706431 10.0.1.1 8998 typ host generation 0\r\n"
        "a=fingerprint:sha-256 02:1A:CC:54:27:AB:EB:9C:53:3F:3E:4B:65:2E:7D:46:3F:54:42:CD:54:F1:7A:03:A2:7D:F9:B0:7F:46:19:B2\r\n"
        "a=setup:actpass\r\n");

    QXmppJingleIq::Content content;
    QVERIFY(content.parseSdp(sdp));

    QCOMPARE(content.descriptionMedia(), QLatin1String("audio"));
    QCOMPARE(content.descriptionSsrc(), quint32(0));
    QCOMPARE(content.payloadTypes().size(), 2);
    QCOMPARE(content.payloadTypes()[0].id(), quint8(96));
    QCOMPARE(content.payloadTypes()[0].parameters().value("vbr"), QLatin1String("on"));
    QCOMPARE(content.payloadTypes()[0].parameters().value("cng"), QLatin1String("on"));
    QCOMPARE(content.payloadTypes()[1].id(), quint8(100));
    QCOMPARE(content.payloadTypes()[1].parameters().value("events"), QLatin1String("0-15,66,70"));
    QCOMPARE(content.transportCandidates().size(), 1);
    QCOMPARE(content.transportCandidates()[0].component(), 1);
    QCOMPARE(content.transportCandidates()[0].foundation(), QLatin1String("1"));
    QCOMPARE(content.transportCandidates()[0].host(), QHostAddress("10.0.1.1"));
    QCOMPARE(content.transportCandidates()[0].port(), quint16(8998));
    QCOMPARE(content.transportCandidates()[0].priority(), 2130706431);
    QCOMPARE(content.transportCandidates()[0].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[0].type(), QXmppJingleCandidate::HostType);
    QCOMPARE(content.transportFingerprint(), QByteArray::fromHex("021acc5427abeb9c533f3e4b652e7d463f5442cd54f17a03a27df9b07f4619b2"));
    QCOMPARE(content.transportFingerprintHash(), QLatin1String("sha-256"));
    QCOMPARE(content.transportFingerprintSetup(), QLatin1String("actpass"));

    QCOMPARE(content.toSdp(), sdp);
}

void tst_QXmppJingleIq::testContentSdpParameters()
{
    const QString sdp(
        "m=audio 8998 RTP/AVP 96 100\r\n"
        "c=IN IP4 10.0.1.1\r\n"
        "a=rtpmap:96 speex/16000\r\n"
        "a=fmtp:96 cng=on; vbr=on\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=fmtp:100 0-15,66,70\r\n"
        "a=candidate:1 1 udp 2130706431 10.0.1.1 8998 typ host generation 0\r\n");

    QXmppJingleIq::Content content;
    QVERIFY(content.parseSdp(sdp));

    QCOMPARE(content.descriptionMedia(), QLatin1String("audio"));
    QCOMPARE(content.descriptionSsrc(), quint32(0));
    QCOMPARE(content.payloadTypes().size(), 2);
    QCOMPARE(content.payloadTypes()[0].id(), quint8(96));
    QCOMPARE(content.payloadTypes()[0].parameters().value("vbr"), QLatin1String("on"));
    QCOMPARE(content.payloadTypes()[0].parameters().value("cng"), QLatin1String("on"));
    QCOMPARE(content.payloadTypes()[1].id(), quint8(100));
    QCOMPARE(content.payloadTypes()[1].parameters().value("events"), QLatin1String("0-15,66,70"));
    QCOMPARE(content.transportCandidates().size(), 1);
    QCOMPARE(content.transportCandidates()[0].component(), 1);
    QCOMPARE(content.transportCandidates()[0].foundation(), QLatin1String("1"));
    QCOMPARE(content.transportCandidates()[0].host(), QHostAddress("10.0.1.1"));
    QCOMPARE(content.transportCandidates()[0].port(), quint16(8998));
    QCOMPARE(content.transportCandidates()[0].priority(), 2130706431);
    QCOMPARE(content.transportCandidates()[0].protocol(), QLatin1String("udp"));
    QCOMPARE(content.transportCandidates()[0].type(), QXmppJingleCandidate::HostType);

    QCOMPARE(content.toSdp(), sdp);
}

void tst_QXmppJingleIq::testContentRtpFeedbackNegotiation()
{
    const QByteArray xml(
        "<content creator=\"initiator\" name=\"voice\">"
        "<description xmlns=\"urn:xmpp:jingle:apps:rtp:1\">"
        "<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"nack\" subtype=\"pli\"/>"
        "<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"nack\" subtype=\"sli\"/>"
        "<rtcp-fb-trr-int xmlns='urn:xmpp:jingle:apps:rtp:rtcp-fb:0' value='60'/>"
        "<rtcp-fb-trr-int xmlns='urn:xmpp:jingle:apps:rtp:rtcp-fb:0' value='80'/>"
        "<payload-type id=\"96\" name=\"speex\"/>"
        "</description>"
        "</content>");

    QXmppJingleIq::Content content1;
    QVERIFY(content1.rtpFeedbackProperties().isEmpty());
    QVERIFY(content1.rtpFeedbackIntervals().isEmpty());
    parsePacket(content1, xml);

    const auto rtpFeedbackProperties1 = content1.rtpFeedbackProperties();
    QCOMPARE(rtpFeedbackProperties1.size(), 2);
    QCOMPARE(rtpFeedbackProperties1[0].subtype(), QStringLiteral("pli"));
    QCOMPARE(rtpFeedbackProperties1[1].subtype(), QStringLiteral("sli"));

    const auto rtpFeedbackIntervals1 = content1.rtpFeedbackIntervals();
    QCOMPARE(rtpFeedbackIntervals1.size(), 2);
    QCOMPARE(rtpFeedbackIntervals1[0].value(), uint64_t(60));
    QCOMPARE(rtpFeedbackIntervals1[1].value(), uint64_t(80));

    serializePacket(content1, xml);

    QXmppJingleRtpFeedbackProperty rtpFeedbackProperty1;
    rtpFeedbackProperty1.setType(QStringLiteral("nack"));
    rtpFeedbackProperty1.setSubtype(QStringLiteral("pli"));

    QXmppJingleRtpFeedbackProperty rtpFeedbackProperty2;
    rtpFeedbackProperty2.setType(QStringLiteral("nack"));
    rtpFeedbackProperty2.setSubtype(QStringLiteral("sli"));

    QXmppJingleRtpFeedbackInterval rtpFeedbackInterval1;
    rtpFeedbackInterval1.setValue(60);

    QXmppJingleRtpFeedbackInterval rtpFeedbackInterval2;
    rtpFeedbackInterval2.setValue(80);

    QXmppJinglePayloadType payloadType;
    payloadType.setId(96);
    payloadType.setName(QStringLiteral("speex"));

    QXmppJingleIq::Content content2;
    content2.setCreator(QStringLiteral("initiator"));
    content2.setName(QStringLiteral("voice"));
    content2.addPayloadType(payloadType);
    content2.setRtpFeedbackProperties({ rtpFeedbackProperty1, rtpFeedbackProperty2 });
    content2.setRtpFeedbackIntervals({ rtpFeedbackInterval1, rtpFeedbackInterval2 });

    const auto rtpFeedbackProperties2 = content2.rtpFeedbackProperties();
    QCOMPARE(rtpFeedbackProperties2.size(), 2);
    QCOMPARE(rtpFeedbackProperties2[0].subtype(), QStringLiteral("pli"));
    QCOMPARE(rtpFeedbackProperties2[1].subtype(), QStringLiteral("sli"));

    const auto rtpFeedbackIntervals2 = content2.rtpFeedbackIntervals();
    QCOMPARE(rtpFeedbackIntervals2.size(), 2);
    QCOMPARE(rtpFeedbackIntervals2[0].value(), uint64_t(60));
    QCOMPARE(rtpFeedbackIntervals2[1].value(), uint64_t(80));

    serializePacket(content2, xml);
}

void tst_QXmppJingleIq::testContentRtpHeaderExtensionsNegotiation()
{
    const QByteArray xml(
        "<content creator=\"initiator\" name=\"voice\">"
        "<description xmlns=\"urn:xmpp:jingle:apps:rtp:1\">"
        "<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\" id=\"1\" uri=\"urn:ietf:params:rtp-hdrext:toffset\"/>"
        "<rtp-hdrext xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\" id=\"2\" uri=\"urn:ietf:params:rtp-hdrext:ntp-64\"/>"
        "<extmap-allow-mixed xmlns=\"urn:xmpp:jingle:apps:rtp:rtp-hdrext:0\"/>"
        "<payload-type id=\"96\" name=\"speex\"/>"
        "</description>"
        "</content>");

    QXmppJingleIq::Content content1;
    QVERIFY(content1.rtpHeaderExtensionProperties().isEmpty());
    QVERIFY(!content1.isRtpHeaderExtensionMixingAllowed());
    parsePacket(content1, xml);

    const auto rtpHeaderExtensionProperties1 = content1.rtpHeaderExtensionProperties();
    QCOMPARE(rtpHeaderExtensionProperties1.size(), 2);
    QCOMPARE(rtpHeaderExtensionProperties1[0].id(), uint32_t(1));
    QCOMPARE(rtpHeaderExtensionProperties1[1].id(), uint32_t(2));

    QVERIFY(content1.isRtpHeaderExtensionMixingAllowed());

    serializePacket(content1, xml);

    QXmppJingleRtpHeaderExtensionProperty rtpHeaderExtensionProperty1;
    rtpHeaderExtensionProperty1.setId(uint32_t(1));
    rtpHeaderExtensionProperty1.setUri(QStringLiteral("urn:ietf:params:rtp-hdrext:toffset"));

    QXmppJingleRtpHeaderExtensionProperty rtpHeaderExtensionProperty2;
    rtpHeaderExtensionProperty2.setId(uint32_t(2));
    rtpHeaderExtensionProperty2.setUri(QStringLiteral("urn:ietf:params:rtp-hdrext:ntp-64"));

    QXmppJinglePayloadType payloadType;
    payloadType.setId(96);
    payloadType.setName(QStringLiteral("speex"));

    QXmppJingleIq::Content content2;
    content2.setCreator(QStringLiteral("initiator"));
    content2.setName(QStringLiteral("voice"));
    content2.addPayloadType(payloadType);
    content2.setRtpHeaderExtensionProperties({ rtpHeaderExtensionProperty1, rtpHeaderExtensionProperty2 });
    content2.setRtpHeaderExtensionMixingAllowed(true);

    const auto rtpHeaderExtensionProperties2 = content2.rtpHeaderExtensionProperties();
    QCOMPARE(rtpHeaderExtensionProperties2.size(), 2);
    QCOMPARE(rtpHeaderExtensionProperties2[0].id(), uint32_t(1));
    QCOMPARE(rtpHeaderExtensionProperties2[1].id(), uint32_t(2));

    QVERIFY(content2.isRtpHeaderExtensionMixingAllowed());

    serializePacket(content2, xml);
}

void tst_QXmppJingleIq::testSession()
{
    const QByteArray xml(
        "<iq"
        " id=\"zid615d9\""
        " to=\"juliet@capulet.lit/balcony\""
        " from=\"romeo@montague.lit/orchard\""
        " type=\"set\">"
        "<jingle xmlns=\"urn:xmpp:jingle:1\""
        " action=\"session-initiate\""
        " initiator=\"romeo@montague.lit/orchard\""
        " sid=\"a73sjjvkla37jfea\">"
        "<muji xmlns=\"urn:xmpp:jingle:muji:0\" room=\"darkcave@chat.shakespeare.lit\"/>"
        "<content creator=\"initiator\" name=\"this-is-a-stub\">"
        "<description xmlns=\"urn:xmpp:jingle:apps:stub:0\"/>"
        "<transport xmlns=\"urn:xmpp:jingle:transports:stub:0\"/>"
        "</content>"
        "</jingle>"
        "</iq>");

    QXmppJingleIq session;
    parsePacket(session, xml);
    QCOMPARE(session.action(), QXmppJingleIq::SessionInitiate);
    QCOMPARE(session.initiator(), QLatin1String("romeo@montague.lit/orchard"));
    QCOMPARE(session.sid(), QLatin1String("a73sjjvkla37jfea"));
    QCOMPARE(session.mujiGroupChatJid(), QStringLiteral("darkcave@chat.shakespeare.lit"));
    QCOMPARE(session.contents().size(), 1);
    QCOMPARE(session.contents()[0].creator(), QLatin1String("initiator"));
    QCOMPARE(session.contents()[0].name(), QLatin1String("this-is-a-stub"));
    QCOMPARE(session.reason().text(), QString());
    QCOMPARE(session.reason().type(), QXmppJingleIq::Reason::None);
    serializePacket(session, xml);
}

void tst_QXmppJingleIq::testTerminate()
{
    const QByteArray xml(
        "<iq"
        " id=\"le71fa63\""
        " to=\"romeo@montague.lit/orchard\""
        " from=\"juliet@capulet.lit/balcony\""
        " type=\"set\">"
        "<jingle xmlns=\"urn:xmpp:jingle:1\""
        " action=\"session-terminate\""
        " sid=\"a73sjjvkla37jfea\">"
        "<reason>"
        "<success/>"
        "</reason>"
        "</jingle>"
        "</iq>");

    QXmppJingleIq session;
    parsePacket(session, xml);
    QCOMPARE(session.action(), QXmppJingleIq::SessionTerminate);
    QCOMPARE(session.initiator(), QString());
    QCOMPARE(session.sid(), QLatin1String("a73sjjvkla37jfea"));
    QCOMPARE(session.reason().text(), QString());
    QCOMPARE(session.reason().type(), QXmppJingleIq::Reason::Success);
    serializePacket(session, xml);
}

void tst_QXmppJingleIq::testRtpSessionState_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("state");

    QTest::newRow("active")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<active xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("active");
    QTest::newRow("hold")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<hold xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("hold");
    QTest::newRow("unhold")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<unhold xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("unhold");
    QTest::newRow("mute")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<mute xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\" creator=\"initiator\" name=\"voice\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("mute");
    QTest::newRow("unmute")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<unmute xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\" creator=\"responder\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("unmute");
    QTest::newRow("ringing")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-info\">"
                             "<ringing xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\"/>"
                             "</jingle>"
                             "</iq>")
        << QStringLiteral("ringing");
}

void tst_QXmppJingleIq::testRtpSessionState()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, state);

    QXmppJingleIq iq1;
    QVERIFY(!iq1.rtpSessionState());
    parsePacket(iq1, xml);

    const auto rtpSessionState1 = *iq1.rtpSessionState();
    if (state == QStringLiteral("active")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateActive>(rtpSessionState1));
    } else if (state == QStringLiteral("hold")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateHold>(rtpSessionState1));
    } else if (state == QStringLiteral("unhold")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateUnhold>(rtpSessionState1));
    } else if (const auto isMute = state == QStringLiteral("mute"); isMute || state == QStringLiteral("unmute")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateMuting>(rtpSessionState1));

        const auto stateMuting = std::get<QXmppJingleIq::RtpSessionStateMuting>(rtpSessionState1);
        QCOMPARE(stateMuting.isMute, isMute);

        if (isMute) {
            QCOMPARE(stateMuting.creator, QXmppJingleIq::Initiator);
            QCOMPARE(stateMuting.name, QStringLiteral("voice"));
        } else {
            QCOMPARE(stateMuting.creator, QXmppJingleIq::Responder);
            QVERIFY(stateMuting.name.isEmpty());
        }
    } else if (state == QStringLiteral("ringing")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateRinging>(rtpSessionState1));
    }

    serializePacket(iq1, xml);

    QXmppJingleIq iq2;
    iq2.setType(QXmppIq::Set);
    iq2.setId({});

    if (state == QStringLiteral("active")) {
        iq2.setRtpSessionState(QXmppJingleIq::RtpSessionStateActive());
    } else if (state == QStringLiteral("hold")) {
        iq2.setRtpSessionState(QXmppJingleIq::RtpSessionStateHold());
    } else if (state == QStringLiteral("unhold")) {
        iq2.setRtpSessionState(QXmppJingleIq::RtpSessionStateUnhold());
    } else if (const auto isMute = state == QStringLiteral("mute"); isMute || state == QStringLiteral("unmute")) {
        QXmppJingleIq::RtpSessionStateMuting stateMuting;
        stateMuting.isMute = isMute;

        if (isMute) {
            stateMuting.creator = QXmppJingleIq::Initiator;
            stateMuting.name = QStringLiteral("voice");
        } else {
            stateMuting.creator = QXmppJingleIq::Responder;
        }

        iq2.setRtpSessionState(stateMuting);
    } else if (state == QStringLiteral("ringing")) {
        iq2.setRtpSessionState(QXmppJingleIq::RtpSessionStateRinging());
    }

    const auto rtpSessionState2 = *iq2.rtpSessionState();
    if (state == QStringLiteral("active")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateActive>(rtpSessionState2));
    } else if (state == QStringLiteral("hold")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateHold>(rtpSessionState2));
    } else if (state == QStringLiteral("unhold")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateUnhold>(rtpSessionState2));
    } else if (const auto isMute = state == QStringLiteral("mute"); isMute || state == QStringLiteral("unmute")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateMuting>(rtpSessionState2));

        const auto stateMuting = std::get<QXmppJingleIq::RtpSessionStateMuting>(rtpSessionState2);
        QCOMPARE(stateMuting.isMute, isMute);

        if (isMute) {
            QCOMPARE(stateMuting.creator, QXmppJingleIq::Initiator);
            QCOMPARE(stateMuting.name, QStringLiteral("voice"));
        } else {
            QCOMPARE(stateMuting.creator, QXmppJingleIq::Responder);
            QVERIFY(stateMuting.name.isEmpty());
        }
    } else if (state == QStringLiteral("ringing")) {
        QVERIFY(std::holds_alternative<QXmppJingleIq::RtpSessionStateRinging>(rtpSessionState2));
    }

    serializePacket(iq2, xml);
}

void tst_QXmppJingleIq::testAudioPayloadType()
{
    const QByteArray xml(R"(<payload-type id="103" name="L16" channels="2" clockrate="16000"/>)");
    QXmppJinglePayloadType payload;
    parsePacket(payload, xml);
    QCOMPARE(payload.id(), static_cast<unsigned char>(103));
    QCOMPARE(payload.name(), QLatin1String("L16"));
    QCOMPARE(payload.channels(), static_cast<unsigned char>(2));
    QCOMPARE(payload.clockrate(), 16000u);
    serializePacket(payload, xml);
}

void tst_QXmppJingleIq::testVideoPayloadType()
{
    const QByteArray xml(
        "<payload-type id=\"98\" name=\"theora\" clockrate=\"90000\">"
        "<parameter name=\"height\" value=\"768\"/>"
        "<parameter name=\"width\" value=\"1024\"/>"
        "</payload-type>");
    QXmppJinglePayloadType payload;
    parsePacket(payload, xml);
    QCOMPARE(payload.id(), static_cast<unsigned char>(98));
    QCOMPARE(payload.name(), QLatin1String("theora"));
    QCOMPARE(payload.clockrate(), 90000u);
    QCOMPARE(payload.parameters().size(), 2);
    QCOMPARE(payload.parameters().value("height"), QLatin1String("768"));
    QCOMPARE(payload.parameters().value("width"), QLatin1String("1024"));
    serializePacket(payload, xml);
}

void tst_QXmppJingleIq::testPayloadTypeRtpFeedbackNegotiation()
{
    const QByteArray xml(
        "<payload-type id=\"96\">"
        "<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"nack\" subtype=\"pli\"/>"
        "<rtcp-fb xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" type=\"nack\" subtype=\"sli\"/>"
        "<rtcp-fb-trr-int xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" value=\"60\"/>"
        "<rtcp-fb-trr-int xmlns=\"urn:xmpp:jingle:apps:rtp:rtcp-fb:0\" value=\"80\"/>"
        "</payload-type>");

    QXmppJinglePayloadType payload1;
    QVERIFY(payload1.rtpFeedbackProperties().isEmpty());
    QVERIFY(payload1.rtpFeedbackIntervals().isEmpty());
    parsePacket(payload1, xml);

    const auto rtpFeedbackProperties1 = payload1.rtpFeedbackProperties();
    QCOMPARE(rtpFeedbackProperties1.size(), 2);
    QCOMPARE(rtpFeedbackProperties1[0].subtype(), QStringLiteral("pli"));
    QCOMPARE(rtpFeedbackProperties1[1].subtype(), QStringLiteral("sli"));

    const auto rtpFeedbackIntervals1 = payload1.rtpFeedbackIntervals();
    QCOMPARE(rtpFeedbackIntervals1.size(), 2);
    QCOMPARE(rtpFeedbackIntervals1[0].value(), uint64_t(60));
    QCOMPARE(rtpFeedbackIntervals1[1].value(), uint64_t(80));

    serializePacket(payload1, xml);

    QXmppJingleRtpFeedbackProperty rtpFeedbackProperty1;
    rtpFeedbackProperty1.setType(QStringLiteral("nack"));
    rtpFeedbackProperty1.setSubtype(QStringLiteral("pli"));

    QXmppJingleRtpFeedbackProperty rtpFeedbackProperty2;
    rtpFeedbackProperty2.setType(QStringLiteral("nack"));
    rtpFeedbackProperty2.setSubtype(QStringLiteral("sli"));

    QXmppJingleRtpFeedbackInterval rtpFeedbackInterval1;
    rtpFeedbackInterval1.setValue(60);

    QXmppJingleRtpFeedbackInterval rtpFeedbackInterval2;
    rtpFeedbackInterval2.setValue(80);

    QXmppJinglePayloadType payload2;
    payload2.setId(96);
    payload2.setRtpFeedbackProperties({ rtpFeedbackProperty1, rtpFeedbackProperty2 });
    payload2.setRtpFeedbackIntervals({ rtpFeedbackInterval1, rtpFeedbackInterval2 });

    const auto rtpFeedbackProperties2 = payload2.rtpFeedbackProperties();
    QCOMPARE(rtpFeedbackProperties2.size(), 2);
    QCOMPARE(rtpFeedbackProperties2[0].subtype(), QStringLiteral("pli"));
    QCOMPARE(rtpFeedbackProperties2[1].subtype(), QStringLiteral("sli"));

    const auto rtpFeedbackIntervals2 = payload2.rtpFeedbackIntervals();
    QCOMPARE(rtpFeedbackIntervals2.size(), 2);
    QCOMPARE(rtpFeedbackIntervals2[0].value(), uint64_t(60));
    QCOMPARE(rtpFeedbackIntervals2[1].value(), uint64_t(80));

    serializePacket(payload2, xml);
}

void tst_QXmppJingleIq::testRtpErrorCondition_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QXmppJingleIq::Reason::RtpErrorCondition>("condition");

    QTest::newRow("NoErrorCondition")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-terminate\">"
                             "<reason>"
                             "<security-error/>"
                             "</reason>"
                             "</jingle>"
                             "</iq>")
        << QXmppJingleIq::Reason::NoErrorCondition;
    QTest::newRow("InvalidCrypto")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-terminate\">"
                             "<reason>"
                             "<security-error/>"
                             "<invalid-crypto xmlns=\"urn:xmpp:jingle:apps:rtp:errors:1\"/>"
                             "</reason>"
                             "</jingle>"
                             "</iq>")
        << QXmppJingleIq::Reason::InvalidCrypto;
    QTest::newRow("CryptoRequired")
        << QByteArrayLiteral("<iq type=\"set\">"
                             "<jingle xmlns=\"urn:xmpp:jingle:1\" action=\"session-terminate\">"
                             "<reason>"
                             "<security-error/>"
                             "<crypto-required xmlns=\"urn:xmpp:jingle:apps:rtp:errors:1\"/>"
                             "</reason>"
                             "</jingle>"
                             "</iq>")
        << QXmppJingleIq::Reason::CryptoRequired;
}

void tst_QXmppJingleIq::testRtpErrorCondition()
{
    QFETCH(QByteArray, xml);
    QFETCH(QXmppJingleIq::Reason::RtpErrorCondition, condition);

    QXmppJingleIq iq1;
    QCOMPARE(iq1.reason().rtpErrorCondition(), QXmppJingleIq::Reason::NoErrorCondition);
    parsePacket(iq1, xml);

    const auto rtpErrorCondition1 = iq1.reason().rtpErrorCondition();
    switch (condition) {
    case QXmppJingleIq::Reason::NoErrorCondition:
        QVERIFY(rtpErrorCondition1 == QXmppJingleIq::Reason::NoErrorCondition);
        break;
    case QXmppJingleIq::Reason::InvalidCrypto:
        QVERIFY(rtpErrorCondition1 == QXmppJingleIq::Reason::InvalidCrypto);
        break;
    case QXmppJingleIq::Reason::CryptoRequired:
        QVERIFY(rtpErrorCondition1 == QXmppJingleIq::Reason::CryptoRequired);
        break;
    }

    serializePacket(iq1, xml);

    QXmppJingleIq iq2;
    iq2.setType(QXmppIq::Set);
    iq2.setId({});
    iq2.setAction(QXmppJingleIq::SessionTerminate);

    switch (condition) {
    case QXmppJingleIq::Reason::NoErrorCondition:
        iq2.reason().setRtpErrorCondition(QXmppJingleIq::Reason::NoErrorCondition);
        break;
    case QXmppJingleIq::Reason::InvalidCrypto:
        iq2.reason().setRtpErrorCondition(QXmppJingleIq::Reason::InvalidCrypto);
        break;
    case QXmppJingleIq::Reason::CryptoRequired:
        iq2.reason().setRtpErrorCondition(QXmppJingleIq::Reason::CryptoRequired);
        break;
    }

    iq2.reason().setType(QXmppJingleIq::Reason::SecurityError);

    const auto rtpErrorCondition2 = iq2.reason().rtpErrorCondition();
    switch (condition) {
    case QXmppJingleIq::Reason::NoErrorCondition:
        QVERIFY(rtpErrorCondition2 == QXmppJingleIq::Reason::NoErrorCondition);
        break;
    case QXmppJingleIq::Reason::InvalidCrypto:
        QVERIFY(rtpErrorCondition2 == QXmppJingleIq::Reason::InvalidCrypto);
        break;
    case QXmppJingleIq::Reason::CryptoRequired:
        QVERIFY(rtpErrorCondition2 == QXmppJingleIq::Reason::CryptoRequired);
        break;
    }

    serializePacket(iq2, xml);
}

QTEST_MAIN(tst_QXmppJingleIq)
#include "tst_qxmppjingleiq.moc"
