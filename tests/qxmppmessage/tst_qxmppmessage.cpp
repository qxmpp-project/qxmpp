// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppEncryptedFileSource.h"
#include "QXmppMessage.h"
#include "QXmppMessageReaction.h"
#include "QXmppMixInvitation.h"
#include "QXmppOutOfBandUrl.h"
#include "QXmppTrustMessageElement.h"

#include <optional>

#include "util.h"
#include <QObject>

class tst_QXmppMessage : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic_data();
    Q_SLOT void testBasic();
    Q_SLOT void testIsXmppStanza();
    Q_SLOT void testUnknownXExtension();
    Q_SLOT void testMessageAttention();
    Q_SLOT void testMessageReceipt();
    Q_SLOT void testDelay_data();
    Q_SLOT void testDelay();
    Q_SLOT void testDelayWithMultipleStamp();
    Q_SLOT void testExtendedAddresses();
    Q_SLOT void testMucInvitation();
    Q_SLOT void testState_data();
    Q_SLOT void testState();
    Q_SLOT void testXhtml();
    Q_SLOT void testSubextensions();
    Q_SLOT void testChatMarkers();
    Q_SLOT void testPrivateMessage();
    Q_SLOT void testOutOfBandUrl();
    Q_SLOT void testMessageCorrect();
    Q_SLOT void testMessageAttaching();
    Q_SLOT void testMix();
    Q_SLOT void testEme();
    Q_SLOT void testSpoiler();
    Q_SLOT void testProcessingHints();
    Q_SLOT void testBobData();
    Q_SLOT void testFallbackIndication();
    Q_SLOT void testStanzaIds();
    Q_SLOT void testSlashMe_data();
    Q_SLOT void testSlashMe();
    Q_SLOT void testMixInvitation();
    Q_SLOT void testTrustMessageElement();
    Q_SLOT void testReaction();
    Q_SLOT void testE2eeFallbackBody();
    Q_SLOT void testFileSharing();
    Q_SLOT void testEncryptedFileSource();
};

void tst_QXmppMessage::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("body");
    QTest::addColumn<QString>("subject");
    QTest::addColumn<QString>("thread");
    QTest::addColumn<QString>("parentThread");

    QTest::newRow("error")
        << QByteArray(R"(<message to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="error"/>)")
        << int(QXmppMessage::Error)
        << QString() << QString() << QString() << QString();
    QTest::newRow("normal")
        << QByteArray(R"(<message to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="normal"/>)")
        << int(QXmppMessage::Normal)
        << QString() << QString() << QString() << QString();
    QTest::newRow("chat")
        << QByteArray(R"(<message to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="chat"/>)")
        << int(QXmppMessage::Chat)
        << QString() << QString() << QString() << QString();
    QTest::newRow("groupchat")
        << QByteArray(R"(<message to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="groupchat"/>)")
        << int(QXmppMessage::GroupChat)
        << QString() << QString() << QString() << QString();
    QTest::newRow("headline")
        << QByteArray(R"(<message to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="headline"/>)")
        << int(QXmppMessage::Headline)
        << QString() << QString() << QString() << QString();

    QTest::newRow("no-parent-thread")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
                      "<subject>test subject</subject>"
                      "<body>test body &amp; stuff</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>")
        << int(QXmppMessage::Normal)
        << "test body & stuff"
        << "test subject"
        << "0e3141cd80894871a68e6fe6b1ec56fa"
        << QString();

    QTest::newRow("full")
        << QByteArray("<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
                      "<subject>test subject</subject>"
                      "<body>test body &amp; stuff</body>"
                      "<thread parent=\"e0ffe42b28561960c6b12b944a092794b9683a38\">0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>")
        << int(QXmppMessage::Normal)
        << "test body & stuff"
        << "test subject"
        << "0e3141cd80894871a68e6fe6b1ec56fa"
        << "e0ffe42b28561960c6b12b944a092794b9683a38";
}

void tst_QXmppMessage::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);
    QFETCH(QString, body);
    QFETCH(QString, subject);
    QFETCH(QString, thread);
    QFETCH(QString, parentThread);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QVERIFY(message.extendedAddresses().isEmpty());
    QCOMPARE(int(message.type()), type);
    QCOMPARE(message.body(), body);
    QCOMPARE(message.subject(), subject);
    QCOMPARE(message.thread(), thread);
    QCOMPARE(message.parentThread(), parentThread);
    QCOMPARE(message.state(), QXmppMessage::None);
    QCOMPARE(message.isAttentionRequested(), false);
    QCOMPARE(message.isReceiptRequested(), false);
    QCOMPARE(message.receiptId(), QString());
    QCOMPARE(message.xhtml(), QString());
    QCOMPARE(message.encryptionMethod(), QXmpp::NoEncryption);
    QVERIFY(!message.isSpoiler());
    QVERIFY(!message.hasHint(QXmppMessage::NoPermanentStore));
    QVERIFY(!message.hasHint(QXmppMessage::NoStore));
    QVERIFY(!message.hasHint(QXmppMessage::NoCopy));
    QVERIFY(!message.hasHint(QXmppMessage::Store));
    QCOMPARE(message.bitsOfBinaryData(), QXmppBitsOfBinaryDataList());
    QVERIFY(!message.isFallback());
    QVERIFY(message.stanzaId().isNull());
    QVERIFY(message.stanzaIdBy().isNull());
    QVERIFY(message.originId().isNull());

    message = QXmppMessage();
    message.setTo(QStringLiteral("foo@example.com/QXmpp"));
    message.setFrom(QStringLiteral("bar@example.com/QXmpp"));
    message.setType(QXmppMessage::Type(type));
    message.setBody(body);
    message.setSubject(subject);
    message.setThread(thread);
    message.setParentThread(parentThread);
    serializePacket(message, xml);
}

void tst_QXmppMessage::testIsXmppStanza()
{
    QXmppMessage msg;
    QVERIFY(msg.isXmppStanza());
}

void tst_QXmppMessage::testUnknownXExtension()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<x xmlns=\"urn:xmpp:unknown:protocol\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    serializePacket(message, xml);
}

void tst_QXmppMessage::testMessageAttention()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<attention xmlns=\"urn:xmpp:attention:0\"/>"
        "</message>");

    QXmppMessage message;
    // parsing
    parsePacket(message, xml);
    QCOMPARE(message.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(message.from(), QString("bar@example.com/QXmpp"));
    QVERIFY(message.extendedAddresses().isEmpty());
    QCOMPARE(message.type(), QXmppMessage::Normal);
    QCOMPARE(message.body(), QString());
    QCOMPARE(message.isAttentionRequested(), true);
    QCOMPARE(message.isReceiptRequested(), false);
    QCOMPARE(message.receiptId(), QString());

    // serialization
    message = QXmppMessage(QStringLiteral("bar@example.com/QXmpp"), QStringLiteral("foo@example.com/QXmpp"));
    message.setType(QXmppMessage::Normal);
    message.setAttentionRequested(true);
    serializePacket(message, xml);
}

void tst_QXmppMessage::testMessageReceipt()
{
    const QByteArray xml(
        "<message id=\"richard2-4.1.247\" to=\"kingrichard@royalty.england.lit/throne\" from=\"northumberland@shakespeare.lit/westminster\" type=\"normal\">"
        "<body>My lord, dispatch; read o&apos;er these articles.</body>"
        "<request xmlns=\"urn:xmpp:receipts\"/>"
        "</message>");

    QXmppMessage message;
    // parsing
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

    // serialization
    message = QXmppMessage();
    message.setId(QStringLiteral("richard2-4.1.247"));
    message.setTo(QStringLiteral("kingrichard@royalty.england.lit/throne"));
    message.setFrom(QStringLiteral("northumberland@shakespeare.lit/westminster"));
    message.setType(QXmppMessage::Normal);
    message.setBody(QStringLiteral("My lord, dispatch; read o'er these articles."));
    message.setReceiptRequested(true);
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

    receipt = QXmppMessage();
    receipt.setId(QStringLiteral("bi29sg183b4v"));
    receipt.setTo(QStringLiteral("northumberland@shakespeare.lit/westminster"));
    receipt.setFrom(QStringLiteral("kingrichard@royalty.england.lit/throne"));
    receipt.setType(QXmppMessage::Normal);
    receipt.setReceiptId(QStringLiteral("richard2-4.1.247"));
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

    // test that an ID is generated
    QXmppMessage message2;
    QVERIFY(message2.id().isNull());
    message2.setReceiptRequested(true);
    QVERIFY(!message2.id().isEmpty());
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

void tst_QXmppMessage::testDelayWithMultipleStamp()
{
    // the XEP-0203 should override XEP-0091's value since XEP-0091 was no more standard protocol
    QByteArray xml(
        "<message type=\"normal\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-06-29T08:23:06.123Z\"/>"
        "<x xmlns=\"jabber:x:delay\" stamp=\"20100629T08:23:06\"/>"
        "</message>");
    QByteArray resultXml(
        "<message type=\"normal\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-06-29T08:23:06.123Z\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    qDebug() << message.stamp();
    QCOMPARE(message.stamp(), QDateTime(QDate(2010, 06, 29), QTime(8, 23, 6, 123), Qt::UTC));
    serializePacket(message, resultXml);
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

    message = QXmppMessage();
    message.setTo(QStringLiteral("hecate@shakespeare.lit"));
    message.setFrom(QStringLiteral("crone1@shakespeare.lit/desktop"));
    message.setType(QXmppMessage::Normal);
    message.setMucInvitationJid(QStringLiteral("darkcave@macbeth.shakespeare.lit"));
    message.setMucInvitationPassword(QStringLiteral("cauldronburn"));
    message.setMucInvitationReason(QStringLiteral("Hey Hecate, this is the place for all good witches!"));
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
        << QByteArray(R"(<message type="normal"><active xmlns="http://jabber.org/protocol/chatstates"/></message>)")
        << int(QXmppMessage::Active);

    QTest::newRow("inactive")
        << QByteArray(R"(<message type="normal"><inactive xmlns="http://jabber.org/protocol/chatstates"/></message>)")
        << int(QXmppMessage::Inactive);

    QTest::newRow("gone")
        << QByteArray(R"(<message type="normal"><gone xmlns="http://jabber.org/protocol/chatstates"/></message>)")
        << int(QXmppMessage::Gone);

    QTest::newRow("composing")
        << QByteArray(R"(<message type="normal"><composing xmlns="http://jabber.org/protocol/chatstates"/></message>)")
        << int(QXmppMessage::Composing);

    QTest::newRow("paused")
        << QByteArray(R"(<message type="normal"><paused xmlns="http://jabber.org/protocol/chatstates"/></message>)")
        << int(QXmppMessage::Paused);
}

void tst_QXmppMessage::testState()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, state);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(int(message.state()), state);

    message = QXmppMessage();
    message.setType(QXmppMessage::Normal);
    message.setState(QXmppMessage::State(state));
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

    message = QXmppMessage();
    message.setBody(QStringLiteral("hi!"));
    message.setType(QXmppMessage::Normal);
    message.setXhtml(QStringLiteral("<p style=\"font-weight:bold\">hi!</p>"));
    serializePacket(message, xml);
}

void tst_QXmppMessage::testSubextensions()
{
    const QByteArray xml = QByteArrayLiteral(
        "<message id=\"aeb214\" to=\"juliet@capulet.lit/chamber\" type=\"normal\">"
        "<result xmlns=\"urn:xmpp:mam:tmp\" id=\"5d398-28273-f7382\" queryid=\"f27\">"
        "<forwarded xmlns=\"urn:xmpp:forward:0\">"
        "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2010-07-10T23:09:32Z\"/>"
        "<message from=\"juliet@capulet.lit/balcony\" "
        "id=\"8a54s\" "
        "to=\"romeo@montague.lit/orchard\" "
        "type=\"chat\">"
        "<body>What man art thou that thus bescreen&apos;d in night so stumblest on my counsel?</body>"
        "</message>"
        "</forwarded>"
        "</result>"
        "<x xmlns=\"jabber:x:new-fancy-extension\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.extensions().size(), 2);
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
        "<body>My lord, dispatch; read o&apos;er these articles.</body>"
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

void tst_QXmppMessage::testPrivateMessage()
{
    const QByteArray xml = QByteArrayLiteral(
        "<message type=\"chat\">"
        "<private xmlns=\"urn:xmpp:carbons:2\"/>"
        "<body>My lord, dispatch; read o&apos;er these articles.</body>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.isPrivate(), true);

    message = QXmppMessage();
    message.setBody(QStringLiteral("My lord, dispatch; read o'er these articles."));
    message.setPrivate(true);
    serializePacket(message, xml);

    message.setPrivate(true);
    QCOMPARE(message.isPrivate(), true);
    message.setPrivate(false);
    QCOMPARE(message.isPrivate(), false);
}

void tst_QXmppMessage::testOutOfBandUrl()
{
    const QByteArray oobXml(
        "<message to=\"MaineBoy@jabber.org/home\" "
        "from=\"stpeter@jabber.org/work\" "
        "type=\"chat\">"
        "<body>Yeah, but do you have a license to Jabber?</body>"
        "<x xmlns=\"jabber:x:oob\">"
        "<url>http://www.jabber.org/images/psa-license.jpg</url>"
        "</x>"
        "<x xmlns=\"jabber:x:oob\">"
        "<url>https://xmpp.org/images/logos/xmpp-logo.svg</url>"
        "<desc>XMPP logo</desc>"
        "</x>"
        "</message>");
    const QString firstUrl = "http://www.jabber.org/images/psa-license.jpg";
    const QString newUrl = "https://xmpp.org/theme/images/xmpp-logo.svg";

    QXmppMessage oobMessage;
    parsePacket(oobMessage, oobXml);
    QCOMPARE(oobMessage.outOfBandUrl(), firstUrl);

    QCOMPARE(oobMessage.outOfBandUrls().size(), 2);

    QCOMPARE(oobMessage.outOfBandUrls().front().url(), QStringLiteral("http://www.jabber.org/images/psa-license.jpg"));
    QVERIFY(!oobMessage.outOfBandUrls().front().description().has_value());

    QCOMPARE(oobMessage.outOfBandUrls().at(1).url(), QStringLiteral("https://xmpp.org/images/logos/xmpp-logo.svg"));
    QCOMPARE(oobMessage.outOfBandUrls().at(1).description().value(), QStringLiteral("XMPP logo"));

    serializePacket(oobMessage, oobXml);

    oobMessage.setOutOfBandUrl(newUrl);
    QCOMPARE(oobMessage.outOfBandUrl(), newUrl);
    QCOMPARE(oobMessage.outOfBandUrls().size(), 1);
}

void tst_QXmppMessage::testMessageCorrect()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<body>This is the corrected version.</body>"
        "<replace xmlns=\"urn:xmpp:message-correct:0\" id=\"badmessage\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.replaceId(), QString("badmessage"));
    serializePacket(message, xml);

    message.setReplaceId("someotherid");
    QCOMPARE(message.replaceId(), QString("someotherid"));
}

void tst_QXmppMessage::testMessageAttaching()
{
    const QByteArray xml(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<body>This is the corrected version.</body>"
        "<attach-to xmlns=\"urn:xmpp:message-attaching:1\" id=\"SD24VCzSYQ\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.attachId(), QString("SD24VCzSYQ"));
    serializePacket(message, xml);

    message.setAttachId("someotherid");
    QCOMPARE(message.attachId(), QString("someotherid"));
}

void tst_QXmppMessage::testMix()
{
    const QByteArray xml(
        "<message to=\"hag66@shakespeare.example\" "
        "from=\"coven@mix.shakespeare.example/123456\" "
        "type=\"groupchat\">"
        "<mix xmlns=\"urn:xmpp:mix:core:1\">"
        "<jid>hag66@shakespeare.example</jid>"
        "<nick>thirdwitch</nick>"
        "</mix>"
        "<body>Harpier cries: &apos;tis time, &apos;tis time.</body>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    serializePacket(message, xml);

    QCOMPARE(message.mixUserJid(), QString("hag66@shakespeare.example"));
    QCOMPARE(message.mixUserNick(), QString("thirdwitch"));

    message.setMixUserJid("alexander@example.org");
    QCOMPARE(message.mixUserJid(), QString("alexander@example.org"));
    message.setMixUserNick("erik");
    QCOMPARE(message.mixUserNick(), QString("erik"));
}

void tst_QXmppMessage::testEme()
{
    // test standard encryption: OMEMO
    const QByteArray xmlOmemo(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<encryption xmlns=\"urn:xmpp:eme:0\" namespace=\"eu.siacs.conversations.axolotl\" name=\"OMEMO\"/>"
        "<body>This message is encrypted with OMEMO, but your client doesn&apos;t seem to support that.</body>"
        "</message>");

    QXmppMessage messageOmemo;
    parsePacket(messageOmemo, xmlOmemo);
    QCOMPARE(messageOmemo.encryptionMethodNs(), QString("eu.siacs.conversations.axolotl"));
    QCOMPARE(messageOmemo.encryptionMethod(), QXmpp::OMEMO);
    QCOMPARE(messageOmemo.encryptionName(), QString("OMEMO"));
    serializePacket(messageOmemo, xmlOmemo);

    // test custom encryption
    const QByteArray xmlCustom(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<encryption xmlns=\"urn:xmpp:eme:0\" namespace=\"im:example:customcrypt:1\" name=\"CustomCrypt\"/>"
        "<body>This message is encrypted with CustomCrypt, but your client doesn&apos;t seem to support that.</body>"
        "</message>");

    QXmppMessage messageCustom;
    parsePacket(messageCustom, xmlCustom);
    QCOMPARE(messageCustom.encryptionMethodNs(), QString("im:example:customcrypt:1"));
    QCOMPARE(messageCustom.encryptionMethod(), QXmpp::UnknownEncryption);
    QCOMPARE(messageCustom.encryptionName(), QString("CustomCrypt"));
    serializePacket(messageCustom, xmlCustom);

    // test setters/getters
    QXmppMessage message;
    message.setEncryptionMethod(QXmpp::LegacyOpenPGP);
    QCOMPARE(message.encryptionMethod(), QXmpp::LegacyOpenPGP);
    QCOMPARE(message.encryptionMethodNs(), QString("jabber:x:encrypted"));
    QCOMPARE(message.encryptionName(), QString("Legacy OpenPGP"));

    message.setEncryptionMethodNs("fancyorg:encryption:fancycrypt:0");
    message.setEncryptionName("FancyCrypt");
    QCOMPARE(message.encryptionMethod(), QXmpp::UnknownEncryption);
    QCOMPARE(message.encryptionMethodNs(), QString("fancyorg:encryption:fancycrypt:0"));
    QCOMPARE(message.encryptionName(), QString("FancyCrypt"));
}

void tst_QXmppMessage::testSpoiler()
{
    // test parsing with hint
    const QByteArray xmlWithHint(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<body>And at the end of the story, both of them die! It is so tragic!</body>"
        "<spoiler xmlns=\"urn:xmpp:spoiler:0\">Love story end</spoiler>"
        "</message>");

    QXmppMessage messageWithHint;
    parsePacket(messageWithHint, xmlWithHint);
    QVERIFY(messageWithHint.isSpoiler());
    QCOMPARE(messageWithHint.spoilerHint(), QString("Love story end"));
    serializePacket(messageWithHint, xmlWithHint);

    // test parsing without hint
    const QByteArray xmlWithoutHint(
        "<message to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\" type=\"normal\">"
        "<body>And at the end of the story, both of them die! It is so tragic!</body>"
        "<spoiler xmlns=\"urn:xmpp:spoiler:0\"></spoiler>"
        "</message>");

    QXmppMessage messageWithoutHint;
    parsePacket(messageWithoutHint, xmlWithoutHint);
    QVERIFY(messageWithoutHint.isSpoiler());
    QCOMPARE(messageWithoutHint.spoilerHint(), QString(""));
    serializePacket(messageWithoutHint, xmlWithoutHint);

    // test setters
    QXmppMessage message;
    message.setIsSpoiler(true);
    QVERIFY(message.isSpoiler());

    message.setIsSpoiler(false);
    message.setSpoilerHint("test hint");
    QCOMPARE(message.spoilerHint(), QString("test hint"));
    QVERIFY(message.isSpoiler());
}

void tst_QXmppMessage::testProcessingHints()
{
    const QByteArray xml(
        "<message to=\"juliet@capulet.lit/laptop\" "
        "from=\"romeo@montague.lit/laptop\" "
        "type=\"chat\">"
        "<no-permanent-store xmlns=\"urn:xmpp:hints\"/>"
        "<no-store xmlns=\"urn:xmpp:hints\"/>"
        "<no-copy xmlns=\"urn:xmpp:hints\"/>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "<body>V unir avtug&apos;f pybnx gb uvqr zr sebz gurve fvtug</body>"
        "</message>");

    // test parsing
    QXmppMessage message;
    parsePacket(message, xml);
    QVERIFY(message.hasHint(QXmppMessage::NoPermanentStore));
    QVERIFY(message.hasHint(QXmppMessage::NoStore));
    QVERIFY(message.hasHint(QXmppMessage::NoCopy));
    QVERIFY(message.hasHint(QXmppMessage::Store));

    // test serialization
    QXmppMessage message2;
    message2.setType(QXmppMessage::Chat);
    message2.setFrom(QString("romeo@montague.lit/laptop"));
    message2.setTo(QString("juliet@capulet.lit/laptop"));
    message2.setBody(QString("V unir avtug'f pybnx gb uvqr zr sebz gurve fvtug"));
    message2.addHint(QXmppMessage::NoPermanentStore);
    message2.addHint(QXmppMessage::NoStore);
    message2.addHint(QXmppMessage::NoCopy);
    message2.addHint(QXmppMessage::Store);
    serializePacket(message2, xml);

    // test remove hint
    message2.removeHint(QXmppMessage::NoCopy);
    QVERIFY(!message2.hasHint(QXmppMessage::NoCopy));

    // test remove all hints
    message2.removeAllHints();
    QVERIFY(!message2.hasHint(QXmppMessage::NoPermanentStore));
    QVERIFY(!message2.hasHint(QXmppMessage::NoStore));
    QVERIFY(!message2.hasHint(QXmppMessage::NoCopy));
    QVERIFY(!message2.hasHint(QXmppMessage::Store));
}

void tst_QXmppMessage::testBobData()
{
    const QByteArray xml = QByteArrayLiteral(
        "<message type=\"chat\">"
        "<data xmlns=\"urn:xmpp:bob\" "
        "cid=\"sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org\" "
        "max-age=\"86400\" "
        "type=\"image/png\">"
        "iVBORw0KGgoAAAANSUhEUgAAALQAAAA8BAMAAAA9AI20AAAAG1BMVEX///8AAADf39+"
        "/v79/f39fX1+fn58/Pz8fHx/8ACGJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAADS0lEQV"
        "RYhe2WS3MSQRCAYTf7OKY1kT0CxsRjHmh5BENIjqEk6pHVhFzdikqO7CGyP9t59Ox2z"
        "y6UeWBVqugLzM70Nz39mqnV1lIWgBWiYXV0BYfNZ0mvwypds1r62vH/gf76ZL/88Qlc"
        "41zeAnQrpx5H3z1Npfr5ovmHusa9SpRiNNIOcdrto6PJ5LLfb5bp9zM+VDq/vptxDEa"
        "a1sql9I3R5KhtfQsA5gNCWYyulV3TyTUDdfL56BvdDl4x7RiybDq9uBgxh1TTPUHDvA"
        "qNQb+LpT5sWehxJZKKcU2MZ6sDE7PMgW2mdlBGdy6ODe6fJFdMI+us95dNqftDMdwU6"
        "+MhpuTS9slcy5TFAcwq0Jt6qssJMTQGp4BGURlmSsNoo5oHL4kqc66NdkDO75mIfCxm"
        "RAlvHxMLdcb7JONavMJbttXXKoMSneYu3OQTlwkUh4mNayi6js55/2VcsZOQfXIYelz"
        "xLcntEGc3WVCsCORJVCc5r0ajAcq+EO1Q0oPm7n7+X/3jEReGdL6qT7Ml6FCjY+quJC"
        "r+D01f6BG0SaHG56ZG32DnY2jcEV1+pU0kxTaEwaGcekN7jyu50U/TV4q6YeieyiNTu"
        "klDKZLukyjKVNwotCUB3B0XO1WjHT3c0DHSO2zACwut8GOiljJIHaJsrlof/fpWNzGM"
        "os6TgIY0hZNpJshzSi4igOhy3cl4qK+YgnqHkAYcZEgdW6/HyrEK7afoY7RCFzArLl2"
        "LLDdrdmmHZfROajwIDfWj8yQG+rzwlA3WvdJiMHtjUekiNrp1oCbmyZDEyKROGjFVDr"
        "PRzlkR9UAfG/OErnPxrop5BwpoEpXQorq2zcGxbnBJndx8Bh0yljGiGv0B4E8+YP3Xp"
        "2rGydZNy4csW8W2pIvWhvijoujRJ0luXsoymV+8AXvE9HjII72+oReS6OfomHe3xWg/"
        "f2coSbDa1XZ1CvGMjy1nH9KBl83oPnQKi+vAXKLjCrRvvT2WCMkPmSFbquiVuTH1qjv"
        "p4j/u7CWyI5/Hn3KAaJJ90eP0Zp1Kjets4WPaElkxheF7cpBESzXuIdLwyFjSub07tB"
        "6JjxH3DGiu+zwHHimdtFsMvKqG/nBxm2TwbvyU6LWs5RnJX4dSldg3QhDLAAAAAElFT"
        "kSuQmCC"
        "</data>"
        "</message>");

    QXmppBitsOfBinaryData data;
    data.setCid(QXmppBitsOfBinaryContentId::fromContentId(
        QStringLiteral("sha1+5a4c38d44fc64805cbb2d92d8b208be13ff40c0f@bob.xmpp.org")));
    data.setContentType(QMimeDatabase().mimeTypeForName(QStringLiteral("image/png")));
    data.setData(QByteArray::fromBase64(QByteArrayLiteral(
        "iVBORw0KGgoAAAANSUhEUgAAALQAAAA8BAMAAAA9AI20AAAAG1BMVEX///8AAADf39+"
        "/v79/f39fX1+fn58/Pz8fHx/8ACGJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAADS0lEQV"
        "RYhe2WS3MSQRCAYTf7OKY1kT0CxsRjHmh5BENIjqEk6pHVhFzdikqO7CGyP9t59Ox2z"
        "y6UeWBVqugLzM70Nz39mqnV1lIWgBWiYXV0BYfNZ0mvwypds1r62vH/gf76ZL/88Qlc"
        "41zeAnQrpx5H3z1Npfr5ovmHusa9SpRiNNIOcdrto6PJ5LLfb5bp9zM+VDq/vptxDEa"
        "a1sql9I3R5KhtfQsA5gNCWYyulV3TyTUDdfL56BvdDl4x7RiybDq9uBgxh1TTPUHDvA"
        "qNQb+LpT5sWehxJZKKcU2MZ6sDE7PMgW2mdlBGdy6ODe6fJFdMI+us95dNqftDMdwU6"
        "+MhpuTS9slcy5TFAcwq0Jt6qssJMTQGp4BGURlmSsNoo5oHL4kqc66NdkDO75mIfCxm"
        "RAlvHxMLdcb7JONavMJbttXXKoMSneYu3OQTlwkUh4mNayi6js55/2VcsZOQfXIYelz"
        "xLcntEGc3WVCsCORJVCc5r0ajAcq+EO1Q0oPm7n7+X/3jEReGdL6qT7Ml6FCjY+quJC"
        "r+D01f6BG0SaHG56ZG32DnY2jcEV1+pU0kxTaEwaGcekN7jyu50U/TV4q6YeieyiNTu"
        "klDKZLukyjKVNwotCUB3B0XO1WjHT3c0DHSO2zACwut8GOiljJIHaJsrlof/fpWNzGM"
        "os6TgIY0hZNpJshzSi4igOhy3cl4qK+YgnqHkAYcZEgdW6/HyrEK7afoY7RCFzArLl2"
        "LLDdrdmmHZfROajwIDfWj8yQG+rzwlA3WvdJiMHtjUekiNrp1oCbmyZDEyKROGjFVDr"
        "PRzlkR9UAfG/OErnPxrop5BwpoEpXQorq2zcGxbnBJndx8Bh0yljGiGv0B4E8+YP3Xp"
        "2rGydZNy4csW8W2pIvWhvijoujRJ0luXsoymV+8AXvE9HjII72+oReS6OfomHe3xWg/"
        "f2coSbDa1XZ1CvGMjy1nH9KBl83oPnQKi+vAXKLjCrRvvT2WCMkPmSFbquiVuTH1qjv"
        "p4j/u7CWyI5/Hn3KAaJJ90eP0Zp1Kjets4WPaElkxheF7cpBESzXuIdLwyFjSub07tB"
        "6JjxH3DGiu+zwHHimdtFsMvKqG/nBxm2TwbvyU6LWs5RnJX4dSldg3QhDLAAAAAElFT"
        "kSuQmCC")));
    data.setMaxAge(86400);

    QXmppMessage message;
    parsePacket(message, xml);
    QCOMPARE(message.type(), QXmppMessage::Chat);
    QCOMPARE(message.id(), QStringLiteral(""));
    QCOMPARE(message.bitsOfBinaryData().size(), 1);
    QCOMPARE(message.bitsOfBinaryData().first().cid().algorithm(), data.cid().algorithm());
    QCOMPARE(message.bitsOfBinaryData().first().cid().hash(), data.cid().hash());
    QCOMPARE(message.bitsOfBinaryData().first().cid(), data.cid());
    QCOMPARE(message.bitsOfBinaryData().first().contentType(), data.contentType());
    QCOMPARE(message.bitsOfBinaryData().first().maxAge(), data.maxAge());
    QCOMPARE(message.bitsOfBinaryData().first().data(), data.data());
    QCOMPARE(message.bitsOfBinaryData().first(), data);
    serializePacket(message, xml);

    QXmppMessage msg;
    msg.setType(QXmppMessage::Chat);
    msg.setId(QStringLiteral(""));
    QXmppBitsOfBinaryDataList bobDataList;
    bobDataList << data;
    msg.setBitsOfBinaryData(bobDataList);
    serializePacket(msg, xml);

    QXmppMessage msg2;
    msg2.setType(QXmppMessage::Chat);
    msg2.setId(QStringLiteral(""));
    msg2.bitsOfBinaryData() << data;
    serializePacket(msg2, xml);

    // test const getter
    const QXmppMessage constMessage = msg;
    QCOMPARE(constMessage.bitsOfBinaryData(), msg.bitsOfBinaryData());
}

void tst_QXmppMessage::testFallbackIndication()
{
    const QByteArray xml = QByteArrayLiteral(
        "<message type=\"chat\">"
        "<fallback xmlns=\"urn:xmpp:fallback:0\"/>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);
    QVERIFY(message.isFallback());
    serializePacket(message, xml);

    QXmppMessage message2;
    message2.setIsFallback(true);
    serializePacket(message2, xml);
}

void tst_QXmppMessage::testStanzaIds()
{
    const QByteArray xml = QByteArrayLiteral(
        "<message type=\"chat\">"
        "<stanza-id xmlns=\"urn:xmpp:sid:0\" id=\"1236\" by=\"server.tld\"/>"
        "<origin-id xmlns=\"urn:xmpp:sid:0\" id=\"5678\"/>"
        "</message>");

    QXmppMessage msg;
    parsePacket(msg, xml);
    QCOMPARE(msg.stanzaId(), QStringLiteral("1236"));
    QCOMPARE(msg.stanzaIdBy(), QStringLiteral("server.tld"));
    QCOMPARE(msg.originId(), QStringLiteral("5678"));
    serializePacket(msg, xml);

    QXmppMessage msg2;
    msg2.setStanzaId(QStringLiteral("1236"));
    msg2.setStanzaIdBy(QStringLiteral("server.tld"));
    msg2.setOriginId(QStringLiteral("5678"));
    serializePacket(msg2, xml);
}

void tst_QXmppMessage::testSlashMe_data()
{
    QTest::addColumn<QString>("body");
    QTest::addColumn<QString>("actionText");
    QTest::addColumn<bool>("expected");

#define POSTIIVE(name, body, actionText) \
    QTest::newRow(QT_STRINGIFY(name)) << body << actionText << true

#define NEGATIVE(name, body) \
    QTest::newRow(QT_STRINGIFY(name)) << body << QString() << false

    POSTIIVE(correct, "/me is having lunch.", "is having lunch.");
    POSTIIVE(correct without text, "/me ", QString());

    NEGATIVE(empty, "");
    NEGATIVE(null, QString());
    NEGATIVE(missing space, "/meshrugs in disgust");
    NEGATIVE(with apostrophe, "/me's disgusted");
    NEGATIVE(space at the beginning, " /me shrugs in disgust");
    NEGATIVE(in quotation marks, R"("/me shrugs in disgust")");
    NEGATIVE(display interpretation, "* Atlas shrugs in disgust");
    NEGATIVE(quote, "Why did you say \"/me sleeps\"?");

#undef POSTIIVE
#undef NEGATIVE
}

void tst_QXmppMessage::testSlashMe()
{
    QFETCH(QString, body);
    QFETCH(QString, actionText);
    QFETCH(bool, expected);

    QCOMPARE(QXmppMessage::isSlashMeCommand(body), expected);
    QCOMPARE(QXmppMessage::slashMeCommandText(body), actionText);

    QXmppMessage msg;
    QVERIFY(!msg.isSlashMeCommand());
    QVERIFY(msg.slashMeCommandText().isNull());

    msg.setBody(body);
    QCOMPARE(msg.isSlashMeCommand(), expected);
    QCOMPARE(msg.slashMeCommandText(), actionText);
}

void tst_QXmppMessage::testMixInvitation()
{
    const QByteArray xml(
        "<message id=\"f5pp2toz\" to=\"cat@shakespeare.example\" from=\"hag66@shakespeare.example/UUID-h5z/0253\" type=\"normal\">"
        "<body>Would you like to join the coven?</body>"
        "<invitation xmlns=\"urn:xmpp:mix:misc:0\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>"
        "</message>");

    QXmppMessage message;
    parsePacket(message, xml);

    QVERIFY(message.mixInvitation().has_value());

    serializePacket(message, xml);
}

void tst_QXmppMessage::testTrustMessageElement()
{
    const QByteArray xml(
        "<message id=\"1\" to=\"alice@example.org/\" from=\"alice@example.org/A2\" type=\"chat\">"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "<trust-message xmlns=\"urn:xmpp:tm:1\" usage=\"urn:xmpp:atm:1\" encryption=\"urn:xmpp:omemo:2\">"
        "<key-owner jid=\"alice@example.org\">"
        "<trust>aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=</trust>"
        "<trust>IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=</trust>"
        "</key-owner>"
        "<key-owner jid=\"bob@example.com\">"
        "<trust>YjVI04NcbTPvXLaA95RO84HPcSvyOgEZ2r5cTyUs0C8=</trust>"
        "<distrust>tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=</distrust>"
        "<distrust>2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=</distrust>"
        "</key-owner>"
        "</trust-message>"
        "</message>");

    QXmppMessage message1;
    parsePacket(message1, xml);
    QVERIFY(message1.trustMessageElement());
    serializePacket(message1, xml);

    QXmppMessage message2;
    message2.setTrustMessageElement(QXmppTrustMessageElement());
    QVERIFY(message2.trustMessageElement());
}

void tst_QXmppMessage::testReaction()
{
    const QByteArray xml(
        "<message id=\"96d73204-a57a-11e9-88b8-4889e7820c76\" to=\"romeo@capulet.net/orchard\" type=\"chat\">"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "<reactions xmlns=\"urn:xmpp:reactions:0\" id=\"744f6e18-a57a-11e9-a656-4889e7820c76\">"
        "<reaction>🐢</reaction>"
        "<reaction>👋</reaction>"
        "</reactions>"
        "</message>");

    QXmppMessage message1;
    QVERIFY(!message1.reaction());

    parsePacket(message1, xml);
    QVERIFY(message1.reaction());
    serializePacket(message1, xml);

    QXmppMessage message2;
    message2.addHint(QXmppMessage::Store);
    message2.setReaction(QXmppMessageReaction());
    QVERIFY(message2.reaction());
}

void tst_QXmppMessage::testE2eeFallbackBody()
{
    const QByteArray xml(
        "<message type=\"chat\">"
        "<body>This message is encrypted with OMEMO 2 but could not be decrypted</body>"
        "</message>");

    // The custom de- / serialization lambda expressions are needed because of
    // "QXmpp::ScePublic".

    const auto parsePacket = [](QXmppMessage &packet, const QByteArray &xml) {
        // qDebug() << "parsing" << xml;
        packet.parse(xmlToDom(xml), QXmpp::ScePublic);
    };

    const auto packetToXml = [](const QXmppMessage &packet) {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        QXmlStreamWriter writer(&buffer);
        packet.toXml(&writer, QXmpp::ScePublic);
        auto data = buffer.data();
        data.replace(u'\'', "&apos;");
        return data;
    };

    const auto serializePacket = [=](QXmppMessage &packet, const QByteArray &xml) {
        auto processedXml = xml;
        processedXml.replace(u'\'', u'"');

        const auto data = packetToXml(packet);
        qDebug() << "expect " << processedXml;
        qDebug() << "writing" << data;
        QCOMPARE(data, processedXml);
    };

    QXmppMessage message1;
    parsePacket(message1, xml);
    QCOMPARE(message1.e2eeFallbackBody(), QStringLiteral("This message is encrypted with OMEMO 2 but could not be decrypted"));
    serializePacket(message1, xml);

    QXmppMessage message2;
    message2.setE2eeFallbackBody(QStringLiteral("This message is encrypted with OMEMO 2 but could not be decrypted"));
    serializePacket(message2, xml);
}

void tst_QXmppMessage::testFileSharing()
{
    const QByteArray xml(
        "<message id='sharing-a-file' to='juliet@shakespeare.lit' from='romeo@montague.lit/resource' type='normal'>"
        "<file-sharing xmlns='urn:xmpp:sfs:0' disposition='inline'>"
        "<file xmlns='urn:xmpp:file:metadata:0'>"
        "<desc>Photo from the summit.</desc>"
        "<hash xmlns='urn:xmpp:hashes:2' algo='sha3-256'>2XarmwTlNxDAMkvymloX3S5+VbylNrJt/l5QyPa+YoU=</hash>"
        "<hash xmlns='urn:xmpp:hashes:2' algo='blake2b-256'>2AfMGH8O7UNPTvUVAM9aK13mpCY=</hash>"
        "<media-type>image/jpeg</media-type>"
        "<name>summit.jpg</name>"
        "<size>3032449</size>"
        "<thumbnail xmlns='urn:xmpp:thumbs:1' uri='cid:sha1+ffd7c8d28e9c5e82afea41f97108c6b4@bob.xmpp.org' media-type='image/png' width='128' height='96'/>"
        "</file>"
        "<sources>"
        "<url-data xmlns='http://jabber.org/protocol/url-data' target='https://download.montague.lit/4a771ac1-f0b2-4a4a-9700-f2a26fa2bb67/summit.jpg'/>"
        "</sources>"
        "</file-sharing>"
        "</message>");

    QXmppMessage message1;
    parsePacket(message1, xml);
    QVERIFY(!message1.sharedFiles().empty());
    serializePacket(message1, xml);
}

void tst_QXmppMessage::testEncryptedFileSource()
{
    {
        QByteArray xml(
            "<encrypted xmlns='urn:xmpp:esfs:0' cipher='urn:xmpp:ciphers:aes-256-gcm-nopadding:0'>"
            "<key>SuRJ2agVm/pQbJQlPq/B23Xt1YOOJCcEGJA5HrcYOGQ=</key>"
            "<iv>T8RDMBaiqn6Ci4Nw</iv>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='sha3-256'>BgKI2gp2kNCRsARNvhFmw5kFf9BBo2pTbV2D8XHTMWI=</hash>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='blake2b-256'>id4cnqqy9/ssfCkM4vYSkiXXrlE=</hash>"
            "<sources xmlns='urn:xmpp:sfs:0'>"
            "<url-data xmlns='http://jabber.org/protocol/url-data' target='https://download.montague.lit/4a771ac1-f0b2-4a4a-9700-f2a26fa2bb67/encrypted.jpg'/>"
            "</sources>"
            "</encrypted>");

        QXmppEncryptedFileSource encryptedSource;
        parsePacket(encryptedSource, xml);
        QCOMPARE(encryptedSource.key(), QByteArray::fromBase64("SuRJ2agVm/pQbJQlPq/B23Xt1YOOJCcEGJA5HrcYOGQ="));
        QCOMPARE(encryptedSource.iv(), QByteArray::fromBase64("T8RDMBaiqn6Ci4Nw"));
        QCOMPARE(encryptedSource.httpSources().front().url(), QUrl("https://download.montague.lit/4a771ac1-f0b2-4a4a-9700-f2a26fa2bb67/encrypted.jpg"));
        QCOMPARE(encryptedSource.cipher(), QXmpp::Aes256GcmNoPad);
        QVERIFY(!encryptedSource.hashes().empty());
        serializePacket(encryptedSource, xml);
    }

    {
        QByteArray xml(
            "<encrypted xmlns='urn:xmpp:esfs:0' cipher='urn:xmpp:ciphers:aes-128-gcm-nopadding:0'>"
            "<key>SuRJ2agVm/pQbJQlPq/B23Xt1YOOJCcEGJA5HrcYOGQ=</key>"
            "<iv>T8RDMBaiqn6Ci4Nw</iv>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='sha3-256'>BgKI2gp2kNCRsARNvhFmw5kFf9BBo2pTbV2D8XHTMWI=</hash>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='blake2b-256'>id4cnqqy9/ssfCkM4vYSkiXXrlE=</hash>"
            "<sources xmlns='urn:xmpp:sfs:0'>"
            "<url-data xmlns='http://jabber.org/protocol/url-data' target='https://download.montague.lit/4a771ac1-f0b2-4a4a-9700-f2a26fa2bb67/encrypted.jpg'/>"
            "</sources>"
            "</encrypted>");

        QXmppEncryptedFileSource encryptedSource;
        parsePacket(encryptedSource, xml);
        serializePacket(encryptedSource, xml);
    }

    {
        QByteArray xml(
            "<encrypted xmlns='urn:xmpp:esfs:0' cipher='urn:xmpp:ciphers:aes-256-cbc-pkcs7:0'>"
            "<key>SuRJ2agVm/pQbJQlPq/B23Xt1YOOJCcEGJA5HrcYOGQ=</key>"
            "<iv>T8RDMBaiqn6Ci4Nw</iv>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='sha3-256'>BgKI2gp2kNCRsARNvhFmw5kFf9BBo2pTbV2D8XHTMWI=</hash>"
            "<hash xmlns='urn:xmpp:hashes:2' algo='blake2b-256'>id4cnqqy9/ssfCkM4vYSkiXXrlE=</hash>"
            "<sources xmlns='urn:xmpp:sfs:0'>"
            "<url-data xmlns='http://jabber.org/protocol/url-data' target='https://download.montague.lit/4a771ac1-f0b2-4a4a-9700-f2a26fa2bb67/encrypted.jpg'/>"
            "</sources>"
            "</encrypted>");

        QXmppEncryptedFileSource encryptedSource;
        parsePacket(encryptedSource, xml);
        serializePacket(encryptedSource, xml);
    }
}

QTEST_MAIN(tst_QXmppMessage)
#include "tst_qxmppmessage.moc"
