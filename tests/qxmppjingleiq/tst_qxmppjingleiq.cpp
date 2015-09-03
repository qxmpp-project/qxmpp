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
#include "QXmppJingleIq.h"
#include "util.h"

class tst_QXmppJingleIq : public QObject
{
    Q_OBJECT

private slots:
    void testCandidate();
    void testContent();
    void testContentFingerprint();
    void testContentSdp();
    void testContentSdpReflexive();
    void testContentSdpFingerprint();
    void testContentSdpParameters();
    void testSession();
    void testTerminate();
    void testAudioPayloadType();
    void testVideoPayloadType();
    void testRinging();
};

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
        "<payload-type id=\"96\" name=\"speex\" clockrate=\"16000\"/>"
        "<payload-type id=\"97\" name=\"speex\" clockrate=\"8000\"/>"
        "<payload-type id=\"18\" name=\"G729\"/>"
        "<payload-type id=\"0\" name=\"PCMU\"/>"
        "<payload-type id=\"103\" name=\"L16\" channels=\"2\" clockrate=\"16000\"/>"
        "<payload-type id=\"98\" name=\"x-ISAC\" clockrate=\"8000\"/>"
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
        "<candidate component=\"1\""
                   " foundation=\"2\""
                   " generation=\"0\""
                   " id=\"y3s2b30v3r\""
                   " ip=\"192.0.2.3\""
                   " network=\"1\""
                   " port=\"45664\""
                   " priority=\"1694498815\""
                   " protocol=\"udp\""
                   " type=\"srflx\"/>"
      "</transport>"
    "</content>");

    QXmppJingleIq::Content content;
    parsePacket(content, xml);

    QCOMPARE(content.creator(), QLatin1String("initiator"));
    QCOMPARE(content.name(), QLatin1String("voice"));
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

    serializePacket(content, xml);
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

void tst_QXmppJingleIq::testAudioPayloadType()
{
    const QByteArray xml("<payload-type id=\"103\" name=\"L16\" channels=\"2\" clockrate=\"16000\"/>");
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

void tst_QXmppJingleIq::testRinging()
{
    const QByteArray xml(
        "<iq"
        " id=\"tgr515bt\""
        " to=\"romeo@montague.lit/orchard\""
        " from=\"juliet@capulet.lit/balcony\""
        " type=\"set\">"
        "<jingle xmlns=\"urn:xmpp:jingle:1\""
        " action=\"session-info\""
        " initiator=\"romeo@montague.lit/orchard\""
        " sid=\"a73sjjvkla37jfea\">"
        "<ringing xmlns=\"urn:xmpp:jingle:apps:rtp:info:1\"/>"
        "</jingle>"
        "</iq>");

    QXmppJingleIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.ringing(), true);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppJingleIq)
#include "tst_qxmppjingleiq.moc"
