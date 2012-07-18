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

#include "QXmppMessage.h"
#include "message.h"
#include "tests.h"

void tst_QXmppMessage::testMessage()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\"/>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString());
    QCOMPARE(message.subject(), QString());
    QCOMPARE(message.thread(), QString());
    QCOMPARE(message.state(), QXmppMessage::None);
    QCOMPARE(message.isAttentionRequested(), false);
    QCOMPARE(message.isReceiptRequested(), false);
    QCOMPARE(message.receiptId(), QString());
    serializePacket(message, xml);
}

void tst_QXmppMessage::testMessageAttention()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
          "<attention xmlns=\"urn:xmpp:attention:0\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString());
    QCOMPARE(message.isAttentionRequested(), true);
    QCOMPARE(message.isReceiptRequested(), false);
    QCOMPARE(message.receiptId(), QString());
    serializePacket(message, xml);
}

void tst_QXmppMessage::testMessageReceipt()
{
    const QByteArray xml(
        "<message id=\"richard2-4.1.247\" to=\"kingrichard@royalty.england.lit/throne\" from=\"northumberland@shakespeare.lit/westminster\" type=\"normal\">"
          "<body>My lord, dispatch; read o'er these articles.</body>"
          "<request xmlns=\"urn:xmpp:receipts\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.id(), QString("richard2-4.1.247"));
    QCOMPARE(message.to(), QString("kingrichard@royalty.england.lit/throne"));
    QCOMPARE(message.from(), QString("northumberland@shakespeare.lit/westminster"));
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString("My lord, dispatch; read o'er these articles."));
    QCOMPARE(message.isAttentionRequested(), false);
    QCOMPARE(message.isReceiptRequested(), true);
    QCOMPARE(message.receiptId(), QString());
    serializePacket(message, xml);

    const QByteArray receiptXml(
        "<message id=\"bi29sg183b4v\" to=\"northumberland@shakespeare.lit/westminster\" from=\"kingrichard@royalty.england.lit/throne\" type=\"normal\">"
          "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
        "</message>");

    QXmppMessage receipt;
    parsePacket(receipt, receiptXml);
    QCOMPARE(receipt.id(), QString("bi29sg183b4v"));
    QCOMPARE(receipt.to(), QString("northumberland@shakespeare.lit/westminster"));
    QCOMPARE(receipt.from(), QString("kingrichard@royalty.england.lit/throne"));
    QCOMPARE(receipt.type(), QXmppMessage::Normal);
    QCOMPARE(receipt.body(), QString());
    QCOMPARE(receipt.isAttentionRequested(), false);
    QCOMPARE(receipt.isReceiptRequested(), false);
    QCOMPARE(receipt.receiptId(), QString("richard2-4.1.247"));
    serializePacket(receipt, receiptXml);

    const QByteArray oldXml(
        "<message id=\"richard2-4.1.247\" to=\"northumberland@shakespeare.lit/westminster\" from=\"kingrichard@royalty.england.lit/throne\" type=\"normal\">"
          "<received xmlns=\"urn:xmpp:receipts\"/>"
        "</message>");

    QXmppMessage old;
    parsePacket(old, oldXml);
    QCOMPARE(old.id(), QString("richard2-4.1.247"));
    QCOMPARE(old.to(), QString("northumberland@shakespeare.lit/westminster"));
    QCOMPARE(old.from(), QString("kingrichard@royalty.england.lit/throne"));
    QCOMPARE(old.type(), QXmppMessage::Normal);
    QCOMPARE(old.body(), QString());
    QCOMPARE(old.isAttentionRequested(), false);
    QCOMPARE(old.isReceiptRequested(), false);
    QCOMPARE(old.receiptId(), QString("richard2-4.1.247"));
}

void tst_QXmppMessage::testMessageFull()
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

void tst_QXmppMessage::testMessageDelay()
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

void tst_QXmppMessage::testMessageLegacyDelay()
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

