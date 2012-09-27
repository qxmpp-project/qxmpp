/*
 * Copyright (C) 2008-2012 The QXmpp developers
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
#include <QtTest/QtTest>

#include "QXmppArchiveIq.h"
#include "QXmppBindIq.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppPasswordChecker.h"
#include "QXmppPubSubIq.h"
#include "QXmppSessionIq.h"
#include "QXmppServer.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppVersionIq.h"
#include "QXmppGlobal.h"
#include "QXmppEntityTimeIq.h"

#include "codec.h"
#include "jingle.h"
#include "rpc.h"
#include "sasl.h"
#include "stun.h"
#include "tests.h"
#include "util.h"

void TestUtils::testCrc32()
{
    quint32 crc = QXmppUtils::generateCrc32(QByteArray());
    QCOMPARE(crc, 0u);

    crc = QXmppUtils::generateCrc32(QByteArray("Hi There"));
    QCOMPARE(crc, 0xDB143BBEu);
}

void TestUtils::testHmac()
{
    QByteArray hmac = QXmppUtils::generateHmacMd5(QByteArray(16, 0x0b), QByteArray("Hi There"));
    QCOMPARE(hmac, QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray("Jefe"), QByteArray("what do ya want for nothing?"));
    QCOMPARE(hmac, QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray(16, 0xaa), QByteArray(50, 0xdd));
    QCOMPARE(hmac, QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6"));
}

void TestUtils::testJid()
{
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com/resource"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid(QString()), QString());

    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com/resource"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain(QString()), QString());

    QCOMPARE(QXmppUtils::jidToResource("foo@example.com/resource"), QLatin1String("resource"));
    QCOMPARE(QXmppUtils::jidToResource("foo@example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource(QString()), QString());

    QCOMPARE(QXmppUtils::jidToUser("foo@example.com/resource"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("foo@example.com"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToUser(QString()), QString());
}

// FIXME: how should we test MIME detection without expose getImageType?
#if 0
QString getImageType(const QByteArray &contents);

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
#else
void TestUtils::testMime()
{
}
#endif

void TestUtils::testLibVersion()
{
    QCOMPARE(QXmppVersion(), QString("0.7.3"));
}

void TestUtils::testTimezoneOffset()
{
    // parsing
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("Z"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+01:30"), 5400);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-01:30"), -5400);

    // serialization
    QCOMPARE(QXmppUtils::timezoneOffsetToString(0), QLatin1String("Z"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(5400), QLatin1String("+01:30"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(-5400), QLatin1String("-01:30"));
}

void TestPackets::testArchiveList_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("max");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"list_1\" type=\"get\">"
        "<list xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\"/>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"list_1\" type=\"get\">"
        "<list xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</list>"
        "</iq>") << 30;
}

void TestPackets::testArchiveList()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, max);

    QXmppArchiveListIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("list_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.end(), QDateTime(QDate(1479, 7, 21), QTime(4, 0, 0), Qt::UTC));
    QCOMPARE(iq.resultSetQuery().max(), max);
    serializePacket(iq, xml);
}

void TestPackets::testArchiveChat_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("count");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"chat_1\" type=\"result\">"
        "<chat xmlns=\"urn:xmpp:archive\""
        " with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:56:15Z\""
        " subject=\"She speaks!\""
        " version=\"4\""
        ">"
        "<from secs=\"0\"><body>Art thou not Romeo, and a Montague?</body></from>"
        "<to secs=\"11\"><body>Neither, fair saint, if either thee dislike.</body></to>"
        "<from secs=\"7\"><body>How cam'st thou hither, tell me, and wherefore?</body></from>"
        "</chat>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"chat_1\" type=\"result\">"
        "<chat xmlns=\"urn:xmpp:archive\""
        " with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:56:15Z\""
        " subject=\"She speaks!\""
        " version=\"4\""
        ">"
        "<from secs=\"0\"><body>Art thou not Romeo, and a Montague?</body></from>"
        "<to secs=\"11\"><body>Neither, fair saint, if either thee dislike.</body></to>"
        "<from secs=\"7\"><body>How cam'st thou hither, tell me, and wherefore?</body></from>"
        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
        "<count>3</count>"
        "</set>"
        "</chat>"
        "</iq>") << 3;
}

void TestPackets::testArchiveChat()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, count);

    QXmppArchiveChatIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.id(), QLatin1String("chat_1"));
    QCOMPARE(iq.chat().with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.chat().messages().size(), 3);
    QCOMPARE(iq.chat().messages()[0].isReceived(), true);
    QCOMPARE(iq.chat().messages()[0].body(), QLatin1String("Art thou not Romeo, and a Montague?"));
    QCOMPARE(iq.chat().messages()[0].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 15), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].isReceived(), false);
    QCOMPARE(iq.chat().messages()[1].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 26), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].body(), QLatin1String("Neither, fair saint, if either thee dislike."));
    QCOMPARE(iq.chat().messages()[2].isReceived(), true);
    QCOMPARE(iq.chat().messages()[2].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 33), Qt::UTC));
    QCOMPARE(iq.chat().messages()[2].body(), QLatin1String("How cam'st thou hither, tell me, and wherefore?"));
    QCOMPARE(iq.resultSetReply().count(), count);
    serializePacket(iq, xml);
}

void TestPackets::testArchiveRemove()
{
    const QByteArray xml(
        "<iq id=\"remove_1\" type=\"set\">"
        "<remove xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\"/>"
        "</iq>");

    QXmppArchiveRemoveIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.id(), QLatin1String("remove_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.end(), QDateTime(QDate(1479, 7, 21), QTime(4, 0, 0), Qt::UTC));
    serializePacket(iq, xml);
}

void TestPackets::testArchiveRetrieve_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("max");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"retrieve_1\" type=\"get\">"
        "<retrieve xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\"/>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"retrieve_1\" type=\"get\">"
        "<retrieve xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</retrieve>"
        "</iq>") << 30;
}

void TestPackets::testArchiveRetrieve()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, max);

    QXmppArchiveRetrieveIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("retrieve_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.resultSetQuery().max(), max);
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

void TestPackets::testDiscovery()
{
    const QByteArray xml(
        "<iq id=\"disco1\" from=\"benvolio@capulet.lit/230193\" type=\"result\">"
        "<query xmlns=\"http://jabber.org/protocol/disco#info\">"
        "<identity category=\"client\" name=\"Exodus 0.9.1\" type=\"pc\"/>"
        "<feature var=\"http://jabber.org/protocol/caps\"/>"
        "<feature var=\"http://jabber.org/protocol/disco#info\"/>"
        "<feature var=\"http://jabber.org/protocol/disco#items\"/>"
        "<feature var=\"http://jabber.org/protocol/muc\"/>"
        "</query>"
        "</iq>");

    QXmppDiscoveryIq disco;
    parsePacket(disco, xml);
    QCOMPARE(disco.verificationString(), QByteArray::fromBase64("QgayPKawpkPSDYmwT/WM94uAlu0="));
    serializePacket(disco, xml);
}

void TestPackets::testDiscoveryWithForm()
{
    const QByteArray xml(
        "<iq id=\"disco1\" to=\"juliet@capulet.lit/chamber\" from=\"benvolio@capulet.lit/230193\" type=\"result\">"
        "<query xmlns=\"http://jabber.org/protocol/disco#info\" node=\"http://psi-im.org#q07IKJEyjvHSyhy//CH0CxmKi8w=\">"
        "<identity xml:lang=\"en\" category=\"client\" name=\"Psi 0.11\" type=\"pc\"/>"
        "<identity xml:lang=\"el\" category=\"client\" name=\"Ψ 0.11\" type=\"pc\"/>"
        "<feature var=\"http://jabber.org/protocol/caps\"/>"
        "<feature var=\"http://jabber.org/protocol/disco#info\"/>"
        "<feature var=\"http://jabber.org/protocol/disco#items\"/>"
        "<feature var=\"http://jabber.org/protocol/muc\"/>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:dataforms:softwareinfo</value>"
        "</field>"
        "<field type=\"text-multi\" var=\"ip_version\">"
        "<value>ipv4</value>"
        "<value>ipv6</value>"
        "</field>"
        "<field type=\"text-single\" var=\"os\">"
        "<value>Mac</value>"
        "</field>"
        "<field type=\"text-single\" var=\"os_version\">"
        "<value>10.5.1</value>"
        "</field>"
        "<field type=\"text-single\" var=\"software\">"
        "<value>Psi</value>"
        "</field>"
        "<field type=\"text-single\" var=\"software_version\">"
        "<value>0.11</value>"
        "</field>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppDiscoveryIq disco;
    parsePacket(disco, xml);
    QCOMPARE(disco.verificationString(), QByteArray::fromBase64("q07IKJEyjvHSyhy//CH0CxmKi8w="));
    serializePacket(disco, xml);
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
    QCOMPARE(features.authMechanisms(), QStringList());
    QCOMPARE(features.compressionMethods(), QStringList());
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
    QCOMPARE(features2.authMechanisms(), QStringList() << "PLAIN");
    QCOMPARE(features2.compressionMethods(), QStringList() << "zlib");
    serializePacket(features2, xml2);
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

void TestServer::testConnect_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<QString>("mechanism");
    QTest::addColumn<bool>("connected");

    QTest::newRow("plain-good") << "testuser" << "testpwd" << "PLAIN" << true;
    QTest::newRow("plain-bad-username") << "baduser" << "testpwd" << "PLAIN" << false;
    QTest::newRow("plain-bad-password") << "testuser" << "badpwd" << "PLAIN" << false;

    QTest::newRow("digest-good") << "testuser" << "testpwd" << "DIGEST-MD5" << true;
    QTest::newRow("digest-bad-username") << "baduser" << "testpwd" << "DIGEST-MD5" << false;
    QTest::newRow("digest-bad-password") << "testuser" << "badpwd" << "DIGEST-MD5" << false;
}

void TestServer::testConnect()
{
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(QString, mechanism);
    QFETCH(bool, connected);

    const QString testDomain("localhost");
    const QHostAddress testHost(QHostAddress::LocalHost);
    const quint16 testPort = 12345;

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    // prepare server
    TestPasswordChecker passwordChecker("testuser", "testpwd");

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
    config.setPort(testPort);
    config.setUser(username);
    config.setPassword(password);
    config.setSaslAuthMechanism(mechanism);
    client.connectToServer(config);
    loop.exec();
    QCOMPARE(client.isConnected(), connected);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QXmppPresence pres;
    pres.availableStatusType();

    // run tests
    int errors = 0;

    TestUtils testUtils;
    errors += QTest::qExec(&testUtils);

    TestPackets testPackets;
    errors += QTest::qExec(&testPackets);

#ifdef QXMPP_AUTOTEST_INTERNAL
    TestCodec testCodec;
    errors += QTest::qExec(&testCodec);
#endif

    TestJingle testJingle;
    errors += QTest::qExec(&testJingle);

    TestPubSub testPubSub;
    errors += QTest::qExec(&testPubSub);

#ifdef QXMPP_AUTOTEST_INTERNAL
    tst_QXmppSasl testSasl;
    errors += QTest::qExec(&testSasl);

    tst_QXmppSaslClient testSaslClient;
    errors += QTest::qExec(&testSaslClient);

    tst_QXmppSaslServer testSaslServer;
    errors += QTest::qExec(&testSaslServer);
#endif

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

