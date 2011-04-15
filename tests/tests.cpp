/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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
#include <QEventLoop>
#include <QVariant>
#include <QtTest/QtTest>

#include "QXmppArchiveIq.h"
#include "QXmppBindIq.h"
#include "QXmppClient.h"
#include "QXmppCodec.h"
#include "QXmppJingleIq.h"
#include "QXmppMessage.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppPasswordChecker.h"
#include "QXmppPresence.h"
#include "QXmppPubSubIq.h"
#include "QXmppRpcIq.h"
#include "QXmppRtpChannel.h"
#include "QXmppSaslAuth.h"
#include "QXmppSessionIq.h"
#include "QXmppServer.h"
#include "QXmppStreamFeatures.h"
#include "QXmppStun.h"
#include "QXmppUtils.h"
#include "QXmppVCardIq.h"
#include "QXmppVersionIq.h"
#include "QXmppGlobal.h"
#include "QXmppEntityTimeIq.h"
#include "tests.h"

QString getImageType(const QByteArray &contents);

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
    QMap<QByteArray, QByteArray> empty = QXmppSaslDigestMd5::parseMessage(QByteArray());
    QCOMPARE(empty.size(), 0);
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(empty), QByteArray());

    // non-empty
    const QByteArray bytes("number=12345,quoted_plain=\"quoted string\",quoted_quote=\"quoted\\\\slash\\\"quote\",string=string");

    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(bytes);
    QCOMPARE(map.size(), 4);
    QCOMPARE(map["number"], QByteArray("12345"));
    QCOMPARE(map["quoted_plain"], QByteArray("quoted string"));
    QCOMPARE(map["quoted_quote"], QByteArray("quoted\\slash\"quote"));
    QCOMPARE(map["string"], QByteArray("string"));
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(map), bytes);
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

static void testMimeType(const QString &fileName, const QString fileType)
{
    // load file from resources
    QFile file(":/" + fileName);
    QCOMPARE(file.open(QIODevice::ReadOnly), true);
    QCOMPARE(getImageType(file.readAll()), fileType);
    file.close();
}

void TestUtils::testMime()
{
    testMimeType("test.bmp", "image/bmp");
    testMimeType("test.gif", "image/gif");
    testMimeType("test.jpg", "image/jpeg");
    testMimeType("test.mng", "video/x-mng");
    testMimeType("test.png", "image/png");
    testMimeType("test.svg", "image/svg+xml");
    testMimeType("test.xpm", "image/x-xpm");
}

void TestUtils::testLibVersion()
{
    QCOMPARE(QXmppVersion(), QString("0.3.0"));
}

void TestUtils::testTimezoneOffset()
{
    // parsing
    QCOMPARE(timezoneOffsetFromString("Z"), 0);
    QCOMPARE(timezoneOffsetFromString("+00:00"), 0);
    QCOMPARE(timezoneOffsetFromString("-00:00"), 0);
    QCOMPARE(timezoneOffsetFromString("+01:30"), 5400);
    QCOMPARE(timezoneOffsetFromString("-01:30"), -5400);

    // serialization
    QCOMPARE(timezoneOffsetToString(0), QLatin1String("Z"));
    QCOMPARE(timezoneOffsetToString(5400), QLatin1String("+01:30"));
    QCOMPARE(timezoneOffsetToString(-5400), QLatin1String("-01:30"));
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
        "<body>test body &amp; stuff</body>"
        "<thread>test thread</thread>"
        "<composing xmlns=\"http://jabber.org/protocol/chatstates\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString("test body & stuff"));
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
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\">"
        "<x xmlns=\"vcard-temp:x:update\"/></presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(presence.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(presence.photoHash(), QByteArray(""));
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateNotReady);
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
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateNone);
    serializePacket(presence, xml);
}

void TestPackets::testPresenceWithVCardUpdate()
{
    const QByteArray xml(
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\">"
        "<show>away</show>"
        "<status>In a meeting</status>"
        "<priority>5</priority>"
        "<x xmlns=\"vcard-temp:x:update\">"
        "<photo>73b908bc</photo>"
        "</x>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(presence.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(presence.status().type(), QXmppPresence::Status::Away);
    QCOMPARE(presence.status().statusText(), QString("In a meeting"));
    QCOMPARE(presence.status().priority(), 5);
    QCOMPARE(presence.photoHash(), QByteArray::fromHex("73b908bc"));
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateValidPhoto);
    serializePacket(presence, xml);
}

void TestPackets::testPresenceWithCapability()
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
    QCOMPARE(presence.status().type(), QXmppPresence::Status::Away);
    QCOMPARE(presence.status().statusText(), QString("In a meeting"));
    QCOMPARE(presence.status().priority(), 5);
    QCOMPARE(presence.photoHash(), QByteArray::fromHex("73b908bc"));
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateValidPhoto);
    QCOMPARE(presence.capabilityHash(), QString("sha-1"));
    QCOMPARE(presence.capabilityNode(), QString("http://code.google.com/p/qxmpp"));
    QCOMPARE(presence.capabilityVer(), QByteArray::fromBase64("QgayPKawpkPSDYmwT/WM94uAlu0="));

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
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.authMechanisms(), QList<QXmppConfiguration::SASLAuthMechanism>());
    QCOMPARE(features.compressionMethods(), QList<QXmppConfiguration::CompressionMethod>());
    serializePacket(features, xml);

    const QByteArray xml2("<stream:features>"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "<auth xmlns=\"http://jabber.org/features/iq-auth\"/>"
        "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>"
        "<compression xmlns=\"http://jabber.org/features/compress\"><method>zlib</method></compression>"
        "<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><mechanism>PLAIN</mechanism></mechanisms>"
        "</stream:features>");
    QXmppStreamFeatures features2;
    parsePacket(features2, xml2);
    QCOMPARE(features2.bindMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.sessionMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.nonSaslAuthMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.tlsMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.authMechanisms(), QList<QXmppConfiguration::SASLAuthMechanism>() << QXmppConfiguration::SASLPlain);
    QCOMPARE(features2.compressionMethods(), QList<QXmppConfiguration::CompressionMethod>() << QXmppConfiguration::ZlibCompression);
    serializePacket(features2, xml2);
}

void TestPackets::testVCard()
{
    const QByteArray xml(
        "<iq id=\"vcard1\" type=\"set\">"
        "<vCard xmlns=\"vcard-temp\">"
        "<BDAY>1983-09-14</BDAY>"
        "<EMAIL><INTERNET/><USERID>foo.bar@example.com</USERID></EMAIL>"
        "<FN>Foo Bar!</FN>"
        "<NICKNAME>FooBar</NICKNAME>"
        "<N><GIVEN>Foo</GIVEN><FAMILY>Wiz</FAMILY><MIDDLE>Baz</MIDDLE></N>"
        "<PHOTO>"
            "<TYPE>image/png</TYPE>"
            "<BINVAL>"
            "iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAIAAABLbSncAAAAAXNSR0IArs4c6QAAAAlwSFlzAAA"
            "UIgAAFCIBjw1HyAAAAAd0SU1FB9oIHQInNvuJovgAAAAiSURBVAjXY2TQ+s/AwMDAwPD/GiMDlP"
            "WfgYGBiQEHGJwSAK2BBQ1f3uvpAAAAAElFTkSuQmCC"
            "</BINVAL>"
        "</PHOTO>"
        "</vCard>"
        "</iq>");

    QXmppVCardIq vcard;
    parsePacket(vcard, xml);
    QCOMPARE(vcard.birthday(), QDate(1983, 9, 14));
    QCOMPARE(vcard.email(), QLatin1String("foo.bar@example.com"));
    QCOMPARE(vcard.nickName(), QLatin1String("FooBar"));
    QCOMPARE(vcard.fullName(), QLatin1String("Foo Bar!"));
    QCOMPARE(vcard.firstName(), QLatin1String("Foo"));
    QCOMPARE(vcard.middleName(), QLatin1String("Baz"));
    QCOMPARE(vcard.lastName(), QLatin1String("Wiz"));
    QCOMPARE(vcard.photo(), QByteArray::fromBase64(
        "iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAIAAABLbSncAAAAAXNSR0IArs4c6QAAAAlwSFlzAAA"
        "UIgAAFCIBjw1HyAAAAAd0SU1FB9oIHQInNvuJovgAAAAiSURBVAjXY2TQ+s/AwMDAwPD/GiMDlP"
        "WfgYGBiQEHGJwSAK2BBQ1f3uvpAAAAAElFTkSuQmCC"));
    QCOMPARE(vcard.photoType(), QLatin1String("image/png"));
    serializePacket(vcard, xml);
}

void TestPackets::testVersionGet()
{
    const QByteArray xmlGet(
    "<iq id=\"version_1\" to=\"juliet@capulet.com/balcony\" "
    "from=\"romeo@montague.net/orchard\" type=\"get\">"
    "<query xmlns=\"jabber:iq:version\"/></iq>");

    QXmppVersionIq verIqGet;
    parsePacket(verIqGet, xmlGet);
    QCOMPARE(verIqGet.id(), QLatin1String("version_1"));
    QCOMPARE(verIqGet.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(verIqGet.from(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(verIqGet.type(), QXmppIq::Get);
    serializePacket(verIqGet, xmlGet);
}

void TestPackets::testVersionResult()
{
    const QByteArray xmlResult(
    "<iq id=\"version_1\" to=\"romeo@montague.net/orchard\" "
    "from=\"juliet@capulet.com/balcony\" type=\"result\">"
    "<query xmlns=\"jabber:iq:version\">"
        "<name>qxmpp</name>"
        "<os>Windows-XP</os>"
        "<version>0.2.0</version>"
    "</query></iq>");

    QXmppVersionIq verIqResult;
    parsePacket(verIqResult, xmlResult);
    QCOMPARE(verIqResult.id(), QLatin1String("version_1"));
    QCOMPARE(verIqResult.to(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(verIqResult.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(verIqResult.type(), QXmppIq::Result);
    QCOMPARE(verIqResult.name(), QString("qxmpp"));
    QCOMPARE(verIqResult.version(), QString("0.2.0"));
    QCOMPARE(verIqResult.os(), QString("Windows-XP"));

    serializePacket(verIqResult, xmlResult);
}

void TestPackets::testEntityTimeGet()
{
    const QByteArray xml("<iq id=\"time_1\" "
        "to=\"juliet@capulet.com/balcony\" "
        "from=\"romeo@montague.net/orchard\" type=\"get\">"
      "<time xmlns=\"urn:xmpp:time\"/>"
    "</iq>");

    QXmppEntityTimeIq entityTime;
    parsePacket(entityTime, xml);
    QCOMPARE(entityTime.id(), QLatin1String("time_1"));
    QCOMPARE(entityTime.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(entityTime.from(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(entityTime.type(), QXmppIq::Get);
    serializePacket(entityTime, xml);
}

void TestPackets::testEntityTimeResult()
{
    const QByteArray xml(
    "<iq id=\"time_1\" to=\"romeo@montague.net/orchard\" from=\"juliet@capulet.com/balcony\" type=\"result\">"
      "<time xmlns=\"urn:xmpp:time\">"
        "<tzo>-06:00</tzo>"
        "<utc>2006-12-19T17:58:35Z</utc>"
      "</time>"
    "</iq>");

    QXmppEntityTimeIq entityTime;
    parsePacket(entityTime, xml);
    QCOMPARE(entityTime.id(), QLatin1String("time_1"));
    QCOMPARE(entityTime.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(entityTime.to(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(entityTime.type(), QXmppIq::Result);
    QCOMPARE(entityTime.tzo(), -21600);
    QCOMPARE(entityTime.utc(), QDateTime(QDate(2006, 12, 19), QTime(17, 58, 35), Qt::UTC));
    serializePacket(entityTime, xml);
}

void TestCodec::testTheoraDecoder()
{
#ifdef QXMPP_USE_THEORA
    QMap<QString, QString> params;
    params.insert("delivery-method", "inline");
    params.insert("configuration", "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");

    QXmppTheoraDecoder decoder;
    QCOMPARE(decoder.setParameters(params), true);

    QXmppVideoFormat format = decoder.format();
    QCOMPARE(format.frameSize(), QSize(320, 240));
    QCOMPARE(format.pixelFormat(), QXmppVideoFrame::Format_YUV420P);
#endif
}

void TestCodec::testTheoraEncoder()
{
#ifdef QXMPP_USE_THEORA
    QXmppVideoFormat format;
    format.setFrameSize(QSize(320, 240));
    format.setPixelFormat(QXmppVideoFrame::Format_YUV420P);

    QXmppTheoraEncoder encoder;
    encoder.setFormat(format);
 
    QMap<QString, QString> params = encoder.parameters();
    QCOMPARE(params.value("delivery-method"), QLatin1String("inline"));
    QCOMPARE(params.value("configuration"), QLatin1String("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"));
#endif
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

void TestJingle::testAudioPayloadType()
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

void TestJingle::testVideoPayloadType()
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

void TestPubSub::testItems()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"get\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<items node=\"storage:bookmarks\"/>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::ItemsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);
}

void TestPubSub::testItemsResponse()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"francisco@denmark.lit/barracks\""
        " from=\"pubsub.shakespeare.lit\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<items node=\"storage:bookmarks\">"
          "<item id=\"current\">"
            "<storage xmlns=\"storage:bookmarks\">"
              "<conference"
                         " autojoin=\"true\""
                         " jid=\"theplay@conference.shakespeare.lit\""
                         " name=\"The Play's the Thing\">"
                "<nick>JC</nick>"
              "</conference>"
            "</storage>"
          "</item>"
        "</items>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::ItemsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);
}

void TestPubSub::testPublish()
{
    const QByteArray xml(
        "<iq"
        " id=\"items1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<publish node=\"storage:bookmarks\">"
          "<item id=\"current\">"
            "<storage xmlns=\"storage:bookmarks\">"
              "<conference"
                         " autojoin=\"true\""
                         " jid=\"theplay@conference.shakespeare.lit\""
                         " name=\"The Play's the Thing\">"
                "<nick>JC</nick>"
              "</conference>"
            "</storage>"
          "</item>"
        "</publish>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("items1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::PublishQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QLatin1String("storage:bookmarks"));
    serializePacket(iq, xml);
}

void TestPubSub::testSubscribe()
{
    const QByteArray xml(
        "<iq"
        " id=\"sub1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"set\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscribe jid=\"francisco@denmark.lit\" node=\"princely_musings\"/>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("sub1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscribeQuery);
    QCOMPARE(iq.queryJid(), QLatin1String("francisco@denmark.lit"));
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    serializePacket(iq, xml);
}

void TestPubSub::testSubscription()
{
    const QByteArray xml(
        "<iq"
        " id=\"sub1\""
        " to=\"francisco@denmark.lit/barracks\""
        " from=\"pubsub.shakespeare.lit\""
        " type=\"result\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscription jid=\"francisco@denmark.lit\""
                     " node=\"princely_musings\""
                     " subid=\"ba49252aaa4f5d320c24d3766f0bdcade78c78d3\""
                     " subscription=\"subscribed\"/>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("sub1"));
    QCOMPARE(iq.to(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.from(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscriptionQuery);
    QCOMPARE(iq.queryJid(), QLatin1String("francisco@denmark.lit"));
    QCOMPARE(iq.queryNode(), QLatin1String("princely_musings"));
    QCOMPARE(iq.subscriptionId(), QLatin1String("ba49252aaa4f5d320c24d3766f0bdcade78c78d3"));
    serializePacket(iq, xml);
}

void TestPubSub::testSubscriptions()
{
    const QByteArray xml(
        "<iq"
        " id=\"subscriptions1\""
        " to=\"pubsub.shakespeare.lit\""
        " from=\"francisco@denmark.lit/barracks\""
        " type=\"get\">"
        "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
        "<subscriptions/>"
        "</pubsub>"
        "</iq>");

    QXmppPubSubIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("subscriptions1"));
    QCOMPARE(iq.to(), QLatin1String("pubsub.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("francisco@denmark.lit/barracks"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.queryType(), QXmppPubSubIq::SubscriptionsQuery);
    QCOMPARE(iq.queryJid(), QString());
    QCOMPARE(iq.queryNode(), QString());
    serializePacket(iq, xml);
}

class TestPasswordChecker : public QXmppPasswordChecker
{
public:
    TestPasswordChecker(const QString &username, const QString &password)
        : m_getPassword(true), m_username(username), m_password(password)
    {
    };

    /// Retrieves the password for the given username.
    QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password)
    {
        if (request.username() == m_username)
        {
            password = m_password;
            return QXmppPasswordReply::NoError;
        } else {
            return QXmppPasswordReply::AuthorizationError;
        }
    };

    /// Sets whether getPassword() is enabled.
    void setGetPassword(bool getPassword)
    {
        m_getPassword = getPassword;
    }

    /// Returns whether getPassword() is enabled.
    bool hasGetPassword() const
    {
        return m_getPassword;
    };

private:
    bool m_getPassword;
    QString m_username;
    QString m_password;
};

void TestServer::testConnect()
{
    const QString testDomain("localhost");
    const QString testPassword("testpwd");
    const QString testUser("testuser");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    // prepare server
    TestPasswordChecker passwordChecker(testUser, testPassword);

    QXmppServer server;
    server.setDomain(testDomain);
    server.setLogger(&logger);
    server.setPasswordChecker(&passwordChecker);
    server.listenForClients(testHost, testPort);

    // prepare client
    QXmppClient client;
    client.setLogger(&logger);

    QEventLoop loop;
    connect(&client, SIGNAL(connected()),
            &loop, SLOT(quit()));
    connect(&client, SIGNAL(disconnected()),
            &loop, SLOT(quit()));

    QXmppConfiguration config;
    config.setDomain(testDomain);
    config.setHost(testHost.toString());
    config.setUser(testUser);
    config.setPort(testPort);

    // check bad password fails
    config.setPassword("badpassword");
    client.connectToServer(config);
    loop.exec();
    QCOMPARE(client.isConnected(), false);

    // check correct password works
    config.setPassword(testPassword);
    client.connectToServer(config);
    loop.exec();
    QCOMPARE(client.isConnected(), true);
}

void TestStun::testFingerprint()
{
    // without fingerprint
    QXmppStunMessage msg;
    msg.setType(0x0001);
    QCOMPARE(msg.encode(QByteArray(), false),
             QByteArray("\x00\x01\x00\x00\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20));

    // with fingerprint
    QCOMPARE(msg.encode(QByteArray(), true),
             QByteArray("\x00\x01\x00\x08\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x28\x00\x04\xB2\xAA\xF9\xF6", 28));
}

void TestStun::testIntegrity()
{
    QXmppStunMessage msg;
    msg.setType(0x0001);
    QCOMPARE(msg.encode(QByteArray("somesecret"), false),
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x14\x96\x4B\x40\xD1\x84\x67\x6A\xFD\xB5\xE0\x7C\xC5\x1F\xFB\xBD\xA2\x61\xAF\xB1\x26", 44));
}

void TestStun::testIPv4Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.mappedHost = QHostAddress("127.0.0.1");
    msg.mappedPort = 12345;
    QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x0C\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x08\x00\x01\x30\x39\x7F\x00\x00\x01", 32));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.mappedHost, QHostAddress("127.0.0.1"));
    QCOMPARE(msg2.mappedPort, quint16(12345));
}

void TestStun::testIPv6Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.mappedHost = QHostAddress("::1");
    msg.mappedPort = 12345;
    const QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x14\x00\x02\x30\x39\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 44));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.mappedHost, QHostAddress("::1"));
    QCOMPARE(msg2.mappedPort, quint16(12345));
}

void TestStun::testXorIPv4Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.xorMappedHost = QHostAddress("127.0.0.1");
    msg.xorMappedPort = 12345;
    QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x0C\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x08\x00\x01\x11\x2B\x5E\x12\xA4\x43", 32));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.xorMappedHost, QHostAddress("127.0.0.1"));
    QCOMPARE(msg2.xorMappedPort, quint16(12345));
}

void TestStun::testXorIPv6Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.xorMappedHost = QHostAddress("::1");
    msg.xorMappedPort = 12345;
    const QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x14\x00\x02\x11\x2B\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 44));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.xorMappedHost, QHostAddress("::1"));
    QCOMPARE(msg2.xorMappedPort, quint16(12345));
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

    TestCodec testCodec;
    errors += QTest::qExec(&testCodec);

    TestJingle testJingle;
    errors += QTest::qExec(&testJingle);

    TestPubSub testPubSub;
    errors += QTest::qExec(&testPubSub);

    TestServer testServer;
    errors += QTest::qExec(&testServer);

    TestStun testStun;
    errors += QTest::qExec(&testStun);

    TestXmlRpc testXmlRpc;
    errors += QTest::qExec(&testXmlRpc);

    if (errors)
    {
        qWarning() << "Total failed tests:" << errors;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
};

