/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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
#include "QXmppMessage.h"
#include "util.h"

class tst_QXmppMessage : public QObject
{
    Q_OBJECT

private slots:
    void testBasic_data();
    void testBasic();
    void testMessageAttention();
    void testMessageReceipt();
    void testDelay_data();
    void testDelay();
    void testExtendedAddresses();
    void testMucInvitation();
    void testState_data();
    void testState();
    void testXhtml();
    void testSubextensions();
    void testChatMarkers();
};

void tst_QXmppMessage::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("body");
    QTest::addColumn<QString>("subject");
    QTest::addColumn<QString>("thread");

    QTest::newRow("error")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"error\"/>")
        << int(QXmppMessage::Error)
        << QString() << QString() << QString();
    QTest::newRow("normal")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\"/>")
        << int(QXmppMessage::Normal)
        << QString() << QString() << QString();
    QTest::newRow("chat")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"chat\"/>")
        << int(QXmppMessage::Chat)
        << QString() << QString() << QString();
    QTest::newRow("groupchat")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"groupchat\"/>")
        << int(QXmppMessage::GroupChat)
        << QString() << QString() << QString();
    QTest::newRow("headline")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"headline\"/>")
        << int(QXmppMessage::Headline)
        << QString() << QString() << QString();

    QTest::newRow("full")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<subject>test subject</subject>"
        "<body>test body &amp; stuff</body>"
        "<thread>test thread</thread>"
        "</message>")
        << int(QXmppMessage::Normal)
        << "test body & stuff" << "test subject" << "test thread";
}

void tst_QXmppMessage::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);
    QFETCH(QString, body);
    QFETCH(QString, subject);
    QFETCH(QString, thread);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QVERIFY(message.extendedAddresses().isEmpty());
    QCOMPARE(int(message.type()), type);
    QCOMPARE(message.body(), body);
    QCOMPARE(message.subject(), subject);
    QCOMPARE(message.thread(), thread);
    QCOMPARE(message.state(), QXmppMessage::None);
    QCOMPARE(message.isAttentionRequested(), false);
    QCOMPARE(message.isReceiptRequested(), false);
    QCOMPARE(message.receiptId(), QString());
    QCOMPARE(message.xhtml(), QString());
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
    QVERIFY(message.extendedAddresses().isEmpty());
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
    QVERIFY(message.extendedAddresses().isEmpty());
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
    QVERIFY(receipt.extendedAddresses().isEmpty());
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
    QVERIFY(old.extendedAddresses().isEmpty());
    QCOMPARE(old.type(), QXmppMessage::Normal);
    QCOMPARE(old.body(), QString());
    QCOMPARE(old.isAttentionRequested(), false);
    QCOMPARE(old.isReceiptRequested(), false);
    QCOMPARE(old.receiptId(), QString("richard2-4.1.247"));
}

void tst_QXmppMessage::testDelay_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QDateTime>("stamp");

    QTest::newRow("delay")
        << QByteArray("<message type=\"normal\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-06-29T08:23:06Z\"/>"
        "</message>")
        << QDateTime(QDate(2010, 06, 29), QTime(8, 23, 6), Qt::UTC);

    QTest::newRow("legacy")
        << QByteArray("<message type=\"normal\">"
        "<x xmlns=\"jabber:x:delay\" stamp=\"20100629T08:23:06\"/>"
        "</message>")
        << QDateTime(QDate(2010, 06, 29), QTime(8, 23, 6), Qt::UTC);
}

void tst_QXmppMessage::testDelay()
{
    QFETCH(QByteArray, xml);
    QFETCH(QDateTime, stamp);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.stamp(), stamp);
    serializePacket(message, xml);
}

void tst_QXmppMessage::testExtendedAddresses()
{
    QByteArray xml(
        "<message to=\"multicast.jabber.org\" type=\"normal\">"
            "<addresses xmlns=\"http://jabber.org/protocol/address\">"
                "<address desc=\"Joe Hildebrand\" jid=\"hildjj@jabber.org/Work\" type=\"to\"/>"
                "<address desc=\"Jeremie Miller\" jid=\"jer@jabber.org/Home\" type=\"cc\"/>"
            "</addresses>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.extendedAddresses().size(), 2);
    QCOMPARE(message.extendedAddresses()[0].description(), QLatin1String("Joe Hildebrand"));
    QCOMPARE(message.extendedAddresses()[0].jid(), QLatin1String("hildjj@jabber.org/Work"));
    QCOMPARE(message.extendedAddresses()[0].type(), QLatin1String("to"));
    QCOMPARE(message.extendedAddresses()[1].description(), QLatin1String("Jeremie Miller"));
    QCOMPARE(message.extendedAddresses()[1].jid(), QLatin1String("jer@jabber.org/Home"));
    QCOMPARE(message.extendedAddresses()[1].type(), QLatin1String("cc"));
    serializePacket(message, xml);
}

void tst_QXmppMessage::testMucInvitation()
{
    QByteArray xml(
        "<message to=\"hecate@shakespeare.lit\" from=\"crone1@shakespeare.lit/desktop\" type=\"normal\">"
            "<x xmlns=\"jabber:x:conference\" jid=\"darkcave@macbeth.shakespeare.lit\" password=\"cauldronburn\" reason=\"Hey Hecate, this is the place for all good witches!\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.mucInvitationJid(), QLatin1String("darkcave@macbeth.shakespeare.lit"));
    QCOMPARE(message.mucInvitationPassword(), QLatin1String("cauldronburn"));
    QCOMPARE(message.mucInvitationReason(), QLatin1String("Hey Hecate, this is the place for all good witches!"));
    serializePacket(message, xml);
}

void tst_QXmppMessage::testState_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("state");

    QTest::newRow("none")
        << QByteArray("<message type=\"normal\"/>")
        << int(QXmppMessage::None);

    QTest::newRow("active")
        << QByteArray("<message type=\"normal\"><active xmlns=\"http://jabber.org/protocol/chatstates\"/></message>")
        << int(QXmppMessage::Active);

    QTest::newRow("inactive")
        << QByteArray("<message type=\"normal\"><inactive xmlns=\"http://jabber.org/protocol/chatstates\"/></message>")
        << int(QXmppMessage::Inactive);

    QTest::newRow("gone")
        << QByteArray("<message type=\"normal\"><gone xmlns=\"http://jabber.org/protocol/chatstates\"/></message>")
        << int(QXmppMessage::Gone);

    QTest::newRow("composing")
        << QByteArray("<message type=\"normal\"><composing xmlns=\"http://jabber.org/protocol/chatstates\"/></message>")
        << int(QXmppMessage::Composing);

    QTest::newRow("paused")
        << QByteArray("<message type=\"normal\"><paused xmlns=\"http://jabber.org/protocol/chatstates\"/></message>")
        << int(QXmppMessage::Paused);
}

void tst_QXmppMessage::testState()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, state);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(int(message.state()), state);
    serializePacket(message, xml);
}

void tst_QXmppMessage::testXhtml()
{
    const QByteArray xml("<message type=\"normal\">"
        "<body>hi!</body>"
        "<html xmlns=\"http://jabber.org/protocol/xhtml-im\">"
        "<body xmlns=\"http://www.w3.org/1999/xhtml\">"
        "<p style=\"font-weight:bold\">hi!</p>"
        "</body>"
        "</html>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.xhtml(), QLatin1String("<p style=\"font-weight:bold\">hi!</p>"));
    serializePacket(message, xml);
}

void tst_QXmppMessage::testSubextensions()
{
    const QByteArray xml("<message id=\"aeb214\" to=\"juliet@capulet.lit/chamber\" type=\"normal\">"
        "<result xmlns=\"urn:xmpp:mam:tmp\" id=\"5d398-28273-f7382\" queryid=\"f27\">"
        "<forwarded xmlns=\"urn:xmpp:forward:0\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-07-10T23:09:32Z\"/>"
        "<message from=\"juliet@capulet.lit/balcony\" id=\"8a54s\" "
        "to=\"romeo@montague.lit/orchard\" type=\"chat\">"
        "<body>What man art thou that thus bescreen'd in night so stumblest on my counsel?</body>"
        "</message>"
        "</forwarded>"
        "</result>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.extensions().size(), 1);
    QCOMPARE(message.extensions().first().tagName(), QLatin1String("result"));
    serializePacket(message, xml);
}

void tst_QXmppMessage::testChatMarkers()
{
    const QByteArray markableXml(
                "<message "
                    "from='northumberland@shakespeare.lit/westminster' "
                    "id='message-1' "
                    "to='ingrichard@royalty.england.lit/throne'>"
                 "<thread>sleeping</thread>"
                 "<body>My lord, dispatch; read o'er these articles.</body>"
                 "<markable xmlns='urn:xmpp:chat-markers:0'/>"
                "</message>");

    QXmppMessage markableMessage;
    parsePacket(markableMessage, markableXml);
    QCOMPARE(markableMessage.isMarkable(), true);
    QCOMPARE(markableMessage.marker(), QXmppMessage::NoMarker);
    QCOMPARE(markableMessage.id(), QString("message-1"));
    QCOMPARE(markableMessage.markedId(), QString());
    QCOMPARE(markableMessage.thread(), QString("sleeping"));
    QCOMPARE(markableMessage.markedThread(), QString());

    const QByteArray receivedXml(
                "<message "
                    "from='kingrichard@royalty.england.lit/throne' "
                    "id='message-2' "
                    "to='northumberland@shakespeare.lit/westminster'>"
                  "<received xmlns='urn:xmpp:chat-markers:0' "
                               "id='message-1' "
                           "thread='sleeping'/>"
                "</message>");

    QXmppMessage receivedMessage;
    parsePacket(receivedMessage, receivedXml);
    QCOMPARE(receivedMessage.isMarkable(), false);
    QCOMPARE(receivedMessage.marker(), QXmppMessage::Received);
    QCOMPARE(receivedMessage.id(), QString("message-2"));
    QCOMPARE(receivedMessage.markedId(), QString("message-1"));
    QCOMPARE(receivedMessage.thread(), QString());
    QCOMPARE(receivedMessage.markedThread(), QString("sleeping"));

    const QByteArray displayedXml(
                "<message "
                    "from='kingrichard@royalty.england.lit/throne' "
                    "id='message-2' "
                    "to='northumberland@shakespeare.lit/westminster'>"
                  "<displayed xmlns='urn:xmpp:chat-markers:0' "
                               "id='message-1' "
                           "thread='sleeping'/>"
                "</message>");

    QXmppMessage displayedMessage;
    parsePacket(displayedMessage, displayedXml);
    QCOMPARE(displayedMessage.isMarkable(), false);
    QCOMPARE(displayedMessage.marker(), QXmppMessage::Displayed);
    QCOMPARE(displayedMessage.id(), QString("message-2"));
    QCOMPARE(displayedMessage.markedId(), QString("message-1"));
    QCOMPARE(displayedMessage.thread(), QString());
    QCOMPARE(displayedMessage.markedThread(), QString("sleeping"));

    const QByteArray acknowledgedXml(
                "<message "
                    "from='kingrichard@royalty.england.lit/throne' "
                    "id='message-2' "
                    "to='northumberland@shakespeare.lit/westminster'>"
                  "<acknowledged xmlns='urn:xmpp:chat-markers:0' "
                               "id='message-1' "
                           "thread='sleeping'/>"
                "</message>");

    QXmppMessage acknowledgedMessage;
    parsePacket(acknowledgedMessage, acknowledgedXml);
    QCOMPARE(acknowledgedMessage.isMarkable(), false);
    QCOMPARE(acknowledgedMessage.marker(), QXmppMessage::Acknowledged);
    QCOMPARE(acknowledgedMessage.id(), QString("message-2"));
    QCOMPARE(acknowledgedMessage.markedId(), QString("message-1"));
    QCOMPARE(acknowledgedMessage.thread(), QString());
    QCOMPARE(acknowledgedMessage.markedThread(), QString("sleeping"));

    const QByteArray emptyThreadXml(
                "<message "
                    "from='kingrichard@royalty.england.lit/throne' "
                    "id='message-2' "
                    "to='northumberland@shakespeare.lit/westminster'>"
                  "<received xmlns='urn:xmpp:chat-markers:0' "
                               "id='message-1'/>"
                "</message>");

    QXmppMessage emptyThreadMessage;
    parsePacket(emptyThreadMessage, emptyThreadXml);
    QCOMPARE(emptyThreadMessage.isMarkable(), false);
    QCOMPARE(emptyThreadMessage.marker(), QXmppMessage::Received);
    QCOMPARE(emptyThreadMessage.id(), QString("message-2"));
    QCOMPARE(emptyThreadMessage.markedId(), QString("message-1"));
    QCOMPARE(emptyThreadMessage.thread(), QString());
    QCOMPARE(emptyThreadMessage.markedThread(), QString());

    const QByteArray notMarkableSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\"/>");

    QXmppMessage serialisationMessage;
    serialisationMessage.setFrom("kingrichard@royalty.england.lit/throne");
    serialisationMessage.setTo("northumberland@shakespeare.lit/westminster");
    serialisationMessage.setId("message-3");
    serialisationMessage.setMarkable(false);
    serializePacket(serialisationMessage, notMarkableSerialisation);

    const QByteArray markableSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\">"
                    "<markable xmlns=\"urn:xmpp:chat-markers:0\"/>"
                "</message>");

    serialisationMessage.setMarkable(true);
    serializePacket(serialisationMessage, markableSerialisation);

    const QByteArray receivedSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\">"
                    "<received xmlns=\"urn:xmpp:chat-markers:0\" "
                               "id=\"message-2\"/>"
                "</message>");

    serialisationMessage.setMarkable(false);
    serialisationMessage.setMarker(QXmppMessage::Received);
    serialisationMessage.setMarkerId("message-2");
    serializePacket(serialisationMessage, receivedSerialisation);

    const QByteArray receivedThreadSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\">"
                    "<received xmlns=\"urn:xmpp:chat-markers:0\" "
                               "id=\"message-2\" "
                               "thread=\"sleeping\"/>"
                "</message>");

    serialisationMessage.setMarker(QXmppMessage::Received);
    serialisationMessage.setMarkerId("message-2");
    serialisationMessage.setMarkedThread("sleeping");
    serializePacket(serialisationMessage, receivedThreadSerialisation);

    const QByteArray displayedThreadSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\">"
                    "<displayed xmlns=\"urn:xmpp:chat-markers:0\" "
                               "id=\"message-2\" "
                               "thread=\"sleeping\"/>"
                "</message>");

    serialisationMessage.setMarker(QXmppMessage::Displayed);
    serialisationMessage.setMarkerId("message-2");
    serialisationMessage.setMarkedThread("sleeping");
    serializePacket(serialisationMessage, displayedThreadSerialisation);

    const QByteArray acknowledgedThreadSerialisation(
                "<message "
                    "id=\"message-3\" "
                    "to=\"northumberland@shakespeare.lit/westminster\" "
                    "from=\"kingrichard@royalty.england.lit/throne\" "
                    "type=\"chat\">"
                    "<acknowledged xmlns=\"urn:xmpp:chat-markers:0\" "
                               "id=\"message-2\" "
                               "thread=\"sleeping\"/>"
                "</message>");

    serialisationMessage.setMarker(QXmppMessage::Acknowledged);
    serialisationMessage.setMarkerId("message-2");
    serialisationMessage.setMarkedThread("sleeping");
    serializePacket(serialisationMessage, acknowledgedThreadSerialisation);
}

QTEST_MAIN(tst_QXmppMessage)
#include "tst_qxmppmessage.moc"
