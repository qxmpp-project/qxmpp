/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include "QXmppBind.h"
#include "QXmppJingleIq.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include "QXmppSession.h"
#include "QXmppUtils.h"
#include "tests.h"
#include "xmlrpc.h"

void TestUtils::testHmac()
{
    QByteArray hmac = generateHmacMd5(QByteArray(16, 0x0b), QByteArray("Hi There"));
    QCOMPARE(hmac, QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d"));

    hmac = generateHmacMd5(QByteArray("Jefe"), QByteArray("what do ya want for nothing?"));
    QCOMPARE(hmac, QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738"));

    hmac = generateHmacMd5(QByteArray(16, 0xaa), QByteArray(50, 0xdd));
    QCOMPARE(hmac, QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6"));
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

void TestPackets::testBindNoResource()
{
    const QByteArray xml(
        "<iq id=\"bind_1\" type=\"set\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "</iq>");

    QXmppBind bind;
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

    QXmppBind bind;
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

    QXmppBind bind;
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

    QXmppSession session;
    parsePacket(session, xml);
    QCOMPARE(session.id(), QString("session_1"));
    QCOMPARE(session.to(), QString("example.com"));
    QCOMPARE(session.type(), QXmppIq::Set);
    serializePacket(session, xml);
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
    // parse
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    QStringList errors;
    QVariant test = XMLRPC::demarshall(element, errors);
    QCOMPARE(errors, QStringList());
    QCOMPARE(test, value);

    // serialise
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    XMLRPC::marshall(&writer, value);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

void TestXmlRpc::testBool()
{
    // FIXME : XML-RPC spec says we should write "0" or "1"
    checkVariant(false,
                 QByteArray("<value><boolean>false</boolean></value>"));
    checkVariant(true,
                 QByteArray("<value><boolean>true</boolean></value>"));
}

void TestXmlRpc::testDouble()
{
    checkVariant(double(-12.214),
                 QByteArray("<value><double>-12.214</double></value>"));
}

void TestXmlRpc::testString()
{
    checkVariant(QString("hello world"),
                 QByteArray("<value><string>hello world</string></value>"));
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

