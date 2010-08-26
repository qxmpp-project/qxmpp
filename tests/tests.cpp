/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#include <cstdlib>

#include <QCoreApplication>
#include <QDomDocument>
#include <QVariant>
#include <QtTest/QtTest>

#include "QXmppArchiveIq.h"
#include "QXmppBindIq.h"
#include "QXmppJingleIq.h"
#include "QXmppMessage.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppPresence.h"
#include "QXmppRpcIq.h"
#include "QXmppSessionIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "tests.h"

void TestUtils::testCrc32()
{
    quint32 crc = generateCrc32(QByteArray());
    QCOMPARE(crc, 0u);

    crc = generateCrc32(QByteArray("Hi There"));
    QCOMPARE(crc, 0xDB143BBEu);
}

void TestUtils::testDigestMd5()
{
    // empty
    QMap<QByteArray, QByteArray> empty = parseDigestMd5(QByteArray());
    QCOMPARE(empty.size(), 0);
    QCOMPARE(serializeDigestMd5(empty), QByteArray());

    // non-empty
    const QByteArray bytes("number=12345,quoted_plain=\"quoted string\",quoted_quote=\"quoted\\\\slash\\\"quote\",string=string");

    QMap<QByteArray, QByteArray> map = parseDigestMd5(bytes);
    QCOMPARE(map.size(), 4);
    QCOMPARE(map["number"], QByteArray("12345"));
    QCOMPARE(map["quoted_plain"], QByteArray("quoted string"));
    QCOMPARE(map["quoted_quote"], QByteArray("quoted\\slash\"quote"));
    QCOMPARE(map["string"], QByteArray("string"));
    QCOMPARE(serializeDigestMd5(map), bytes);
}

void TestUtils::testHmac()
{
    QByteArray hmac = generateHmacMd5(QByteArray(16, 0x0b), QByteArray("Hi There"));
    QCOMPARE(hmac, QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d"));

    hmac = generateHmacMd5(QByteArray("Jefe"), QByteArray("what do ya want for nothing?"));
    QCOMPARE(hmac, QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738"));

    hmac = generateHmacMd5(QByteArray(16, 0xaa), QByteArray(50, 0xdd));
    QCOMPARE(hmac, QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6"));
}

void TestUtils::testJid()
{
    QCOMPARE(jidToBareJid("foo@example.com/resource"), QLatin1String("foo@example.com"));
    QCOMPARE(jidToBareJid("foo@example.com"), QLatin1String("foo@example.com"));
    QCOMPARE(jidToBareJid("example.com"), QLatin1String("example.com"));
    QCOMPARE(jidToBareJid(QString()), QString());

    QCOMPARE(jidToDomain("foo@example.com/resource"), QLatin1String("example.com"));
    QCOMPARE(jidToDomain("foo@example.com"), QLatin1String("example.com"));
    QCOMPARE(jidToDomain("example.com"), QLatin1String("example.com"));
    QCOMPARE(jidToDomain(QString()), QString());

    QCOMPARE(jidToResource("foo@example.com/resource"), QLatin1String("resource"));
    QCOMPARE(jidToResource("foo@example.com"), QString());
    QCOMPARE(jidToResource("example.com"), QString());
    QCOMPARE(jidToResource(QString()), QString());

    QCOMPARE(jidToUser("foo@example.com/resource"), QLatin1String("foo"));
    QCOMPARE(jidToUser("foo@example.com"), QLatin1String("foo"));
    QCOMPARE(jidToUser("example.com"), QString());
    QCOMPARE(jidToUser(QString()), QString());
}

template <class T>
static void parsePacket(T &packet, const QByteArray &xml)
{
    //qDebug() << "parsing" << xml;
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    packet.parse(element);
}

template <class T>
static void serializePacket(T &packet, const QByteArray &xml)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

void TestPackets::testArchiveList()
{
    const QByteArray xml(
        "<iq id=\"list_1\" type=\"get\">"
        "<list xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</list>"
        "</iq>");

    QXmppArchiveListIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("list_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.end(), QDateTime(QDate(1479, 7, 21), QTime(4, 0, 0), Qt::UTC));
    QCOMPARE(iq.max(), 30);
    serializePacket(iq, xml);
}

void TestPackets::testArchiveChat()
{
    const QByteArray xml(
        "<iq id=\"chat_1\" type=\"result\">"
        "<chat xmlns=\"urn:xmpp:archive\""
        " with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:56:15Z\""
        " subject=\"She speaks!\""
        " version=\"4\""
        ">"
        "<from secs=\"0\"><body>Art thou not Romeo, and a Montague?</body></from>"
        "<to secs=\"11\"><body>Neither, fair saint, if either thee dislike.</body></to>"
        "</chat>"
        "</iq>");

    QXmppArchiveChatIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.id(), QLatin1String("chat_1"));
    QCOMPARE(iq.chat().with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.chat().messages().size(), 2);
    QCOMPARE(iq.chat().messages()[0].isReceived(), true);
    QCOMPARE(iq.chat().messages()[0].body(), QLatin1String("Art thou not Romeo, and a Montague?"));
    QCOMPARE(iq.chat().messages()[0].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 15), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].isReceived(), false);
    QCOMPARE(iq.chat().messages()[1].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 26), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].body(), QLatin1String("Neither, fair saint, if either thee dislike."));
    serializePacket(iq, xml);
}

void TestPackets::testArchiveRetrieve()
{
    const QByteArray xml(
        "<iq id=\"retrieve_1\" type=\"get\">"
        "<retrieve xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</retrieve>"
        "</iq>");

    QXmppArchiveRetrieveIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("retrieve_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.max(), 30);
    serializePacket(iq, xml);
}

void TestPackets::testBindNoResource()
{
    const QByteArray xml(
        "<iq id=\"bind_1\" type=\"set\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Set);
    QCOMPARE(bind.id(), QString("bind_1"));
    QCOMPARE(bind.jid(), QString());
    QCOMPARE(bind.resource(), QString());
    serializePacket(bind, xml);
}

void TestPackets::testBindResource()
{
    const QByteArray xml(
        "<iq id=\"bind_2\" type=\"set\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\">"
        "<resource>someresource</resource>"
        "</bind>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Set);
    QCOMPARE(bind.id(), QString("bind_2"));
    QCOMPARE(bind.jid(), QString());
    QCOMPARE(bind.resource(), QString("someresource"));
    serializePacket(bind, xml);
}

void TestPackets::testBindResult()
{
    const QByteArray xml(
        "<iq id=\"bind_2\" type=\"result\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\">"
        "<jid>somenode@example.com/someresource</jid>"
        "</bind>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Result);
    QCOMPARE(bind.id(), QString("bind_2"));
    QCOMPARE(bind.jid(), QString("somenode@example.com/someresource"));
    QCOMPARE(bind.resource(), QString());
    serializePacket(bind, xml);
}

void TestPackets::testMessage()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\"/>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    serializePacket(message, xml);
}

void TestPackets::testMessageFull()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<subject>test subject</subject>"
        "<body>test body</body>"
        "<thread>test thread</thread>"
        "<composing xmlns=\"http://jabber.org/protocol/chatstates\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString("test body"));
    QCOMPARE(message.subject(), QString("test subject"));
    QCOMPARE(message.thread(), QString("test thread"));
    QCOMPARE(message.state(), QXmppMessage::Composing);
    serializePacket(message, xml);
}

void TestPackets::testMessageDelay()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-06-29T08:23:06Z\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.stamp(), QDateTime(QDate(2010, 06, 29), QTime(8, 23, 6), Qt::UTC));
    serializePacket(message, xml);
}

void TestPackets::testMessageLegacyDelay()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<x xmlns=\"jabber:x:delay\" stamp=\"20100629T08:23:06\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.stamp(), QDateTime(QDate(2010, 06, 29), QTime(8, 23, 6), Qt::UTC));
    serializePacket(message, xml);
}

void TestPackets::testNonSaslAuth()
{
    // Client Requests Authentication Fields from Server
    const QByteArray xml1(
        "<iq id=\"auth1\" to=\"shakespeare.lit\" type=\"get\">"
        "<query xmlns=\"jabber:iq:auth\"/>"
        "</iq>");

    QXmppNonSASLAuthIq iq1;
    parsePacket(iq1, xml1);
    serializePacket(iq1, xml1);

    // Client Provides Required Information (Plaintext)
    const QByteArray xml3(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<password>Calli0pe</password>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq3;
    parsePacket(iq3, xml3);
    QCOMPARE(iq3.username(), QLatin1String("bill"));
    QCOMPARE(iq3.digest(), QByteArray());
    QCOMPARE(iq3.password(), QLatin1String("Calli0pe"));
    QCOMPARE(iq3.resource(), QLatin1String("globe"));
    serializePacket(iq3, xml3);

    // Client Provides Required Information (Plaintext)
    const QByteArray xml4(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<digest>48fc78be9ec8f86d8ce1c39c320c97c21d62334d</digest>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq4;
    parsePacket(iq4, xml4);
    QCOMPARE(iq4.username(), QLatin1String("bill"));
    QCOMPARE(iq4.digest(), QByteArray("\x48\xfc\x78\xbe\x9e\xc8\xf8\x6d\x8c\xe1\xc3\x9c\x32\x0c\x97\xc2\x1d\x62\x33\x4d"));
    QCOMPARE(iq4.password(), QString());
    QCOMPARE(iq4.resource(), QLatin1String("globe"));
    serializePacket(iq4, xml4);
}

void TestPackets::testPresence()
{
    const QByteArray xml(
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\"/>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(presence.from(), QString("bar@example.com/QXmpp"));
    serializePacket(presence, xml);
}

void TestPackets::testPresenceFull()
{
    const QByteArray xml(
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\">"
        "<show>away</show>"
        "<status>In a meeting</status>"
        "<priority>5</priority>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(presence.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(presence.status().type(), QXmppPresence::Status::Away);
    QCOMPARE(presence.status().statusText(), QString("In a meeting"));
    QCOMPARE(presence.status().priority(), 5);
    serializePacket(presence, xml);
}

void TestPackets::testSession()
{
    const QByteArray xml(
        "<iq id=\"session_1\" to=\"example.com\" type=\"set\">"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "</iq>");

    QXmppSessionIq session;
    parsePacket(session, xml);
    QCOMPARE(session.id(), QString("session_1"));
    QCOMPARE(session.to(), QString("example.com"));
    QCOMPARE(session.type(), QXmppIq::Set);
    serializePacket(session, xml);
}

void TestPackets::testStreamFeatures()
{
    const QByteArray xml("<stream:features/>");
    QXmppStreamFeatures features;
    parsePacket(features, xml);
    QCOMPARE(features.isBindAvailable(), false);
    QCOMPARE(features.isSessionAvailable(), false);
    QCOMPARE(features.isNonSaslAuthAvailable(), false);
    QCOMPARE(features.authMechanisms(), QList<QXmppConfiguration::SASLAuthMechanism>());
    QCOMPARE(features.securityMode(), QXmppConfiguration::TLSDisabled);
    serializePacket(features, xml);

    const QByteArray xml2("<stream:features>"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><mechanism>PLAIN</mechanism></mechanisms>"
        "<auth xmlns=\"http://jabber.org/features/iq-auth\"/>"
        "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>"
        "</stream:features>");
    QXmppStreamFeatures features2;
    parsePacket(features2, xml2);
    QCOMPARE(features2.isBindAvailable(), true);
    QCOMPARE(features2.isSessionAvailable(), true);
    QCOMPARE(features2.isNonSaslAuthAvailable(), true);
    QCOMPARE(features2.authMechanisms(), QList<QXmppConfiguration::SASLAuthMechanism>() << QXmppConfiguration::SASLPlain);
    QCOMPARE(features2.securityMode(), QXmppConfiguration::TLSEnabled);
    serializePacket(features2, xml2);


}


void TestJingle::testSession()
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
    QCOMPARE(session.content().creator(), QLatin1String("initiator"));
    QCOMPARE(session.content().name(), QLatin1String("this-is-a-stub"));
    QCOMPARE(session.reason().text(), QString());
    QCOMPARE(session.reason().type(), QXmppJingleIq::Reason::None);
    serializePacket(session, xml);
}

void TestJingle::testTerminate()
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

void TestJingle::testPayloadType()
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

void TestJingle::testRinging()
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

static void checkVariant(const QVariant &value, const QByteArray &xml)
{
    // serialise
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    XMLRPC::marshall(&writer, value);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);

    // parse
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    QStringList errors;
    QVariant test = XMLRPC::demarshall(element, errors);
    if (!errors.isEmpty())
        qDebug() << errors;
    QCOMPARE(errors, QStringList());
    QCOMPARE(test, value);
}

void TestXmlRpc::testBase64()
{
    checkVariant(QByteArray("\0\1\2\3", 4),
                 QByteArray("<value><base64>AAECAw==</base64></value>"));
}

void TestXmlRpc::testBool()
{
    checkVariant(false,
                 QByteArray("<value><boolean>0</boolean></value>"));
    checkVariant(true,
                 QByteArray("<value><boolean>1</boolean></value>"));
}

void TestXmlRpc::testDateTime()
{
    checkVariant(QDateTime(QDate(1998, 7, 17), QTime(14, 8, 55)),
                 QByteArray("<value><dateTime.iso8601>1998-07-17T14:08:55</dateTime.iso8601></value>"));
}

void TestXmlRpc::testDouble()
{
    checkVariant(double(-12.214),
                 QByteArray("<value><double>-12.214</double></value>"));
}

void TestXmlRpc::testInt()
{
    checkVariant(int(-12),
                 QByteArray("<value><i4>-12</i4></value>"));
}

void TestXmlRpc::testNil()
{
    checkVariant(QVariant(),
                 QByteArray("<value><nil/></value>"));
}

void TestXmlRpc::testString()
{
    checkVariant(QString("hello world"),
                 QByteArray("<value><string>hello world</string></value>"));
}

void TestXmlRpc::testArray()
{
    checkVariant(QVariantList() << QString("hello world") << double(-12.214),
                 QByteArray("<value><array><data>"
                            "<value><string>hello world</string></value>"
                            "<value><double>-12.214</double></value>"
                            "</data></array></value>"));
}

void TestXmlRpc::testStruct()
{
    QMap<QString, QVariant> map;
    map["bar"] = QString("hello world");
    map["foo"] = double(-12.214);
    checkVariant(map,
                 QByteArray("<value><struct>"
                            "<member>"
                                "<name>bar</name>"
                                "<value><string>hello world</string></value>"
                            "</member>"
                            "<member>"
                                "<name>foo</name>"
                                "<value><double>-12.214</double></value>"
                            "</member>"
                            "</struct></value>"));
}

void TestXmlRpc::testInvoke()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"responder@company-a.com/jrpc-server\""
        " from=\"requester@company-b.com/jrpc-client\""
        " type=\"set\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodCall>"
        "<methodName>examples.getStateName</methodName>"
        "<params>"
        "<param>"
        "<value><i4>6</i4></value>"
        "</param>"
        "</params>"
        "</methodCall>"
        "</query>"
        "</iq>");

    QXmppRpcInvokeIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.method(), QLatin1String("examples.getStateName"));
    QCOMPARE(iq.arguments(), QVariantList() << int(6));
    serializePacket(iq, xml);
}

void TestXmlRpc::testResponse()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"requester@company-b.com/jrpc-client\""
        " from=\"responder@company-a.com/jrpc-server\""
        " type=\"result\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodResponse>"
        "<params>"
        "<param>"
        "<value><string>Colorado</string></value>"
        "</param>"
        "</params>"
        "</methodResponse>"
        "</query>"
        "</iq>");

    QXmppRpcResponseIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.faultCode(), 0);
    QCOMPARE(iq.faultString(), QString());
    QCOMPARE(iq.values(), QVariantList() << QString("Colorado"));
    serializePacket(iq, xml);
}

void TestXmlRpc::testResponseFault()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"requester@company-b.com/jrpc-client\""
        " from=\"responder@company-a.com/jrpc-server\""
        " type=\"result\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodResponse>"
        "<fault>"
        "<value>"
            "<struct>"
                "<member>"
                    "<name>faultCode</name>"
                    "<value><i4>404</i4></value>"
                "</member>"
                "<member>"
                    "<name>faultString</name>"
                    "<value><string>Not found</string></value>"
                "</member>"
            "</struct>"
        "</value>"
        "</fault>"
        "</methodResponse>"
        "</query>"
        "</iq>");

    QXmppRpcResponseIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.faultCode(), 404);
    QCOMPARE(iq.faultString(), QLatin1String("Not found"));
    QCOMPARE(iq.values(), QVariantList());
    serializePacket(iq, xml);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // run tests
    int errors = 0;

    TestUtils testUtils;
    errors += QTest::qExec(&testUtils);

    TestPackets testPackets;
    errors += QTest::qExec(&testPackets);

    TestJingle testJingle;
    errors += QTest::qExec(&testJingle);

    TestXmlRpc testXmlRpc;
    errors += QTest::qExec(&testXmlRpc);

    if (errors)
    {
        qWarning() << "Total failed tests:" << errors;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
};

