/*
 * Copyright (C) 2008-2012 The QXmpp developers
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

#include "QXmppJingleIq.h"

#include "jingle.h"
#include "tests.h"

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
