// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessageReaction.h"

#include "util.h"
#include <QObject>

class tst_QXmppMessageReaction : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testIsMessageReaction_data();
    Q_SLOT void testIsMessageReaction();
    Q_SLOT void testMessageReaction();
    Q_SLOT void testMessageReactionWithDuplicateEmojis();
    Q_SLOT void testMessageReactionRemoval();
};

void tst_QXmppMessageReaction::testIsMessageReaction_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<reactions xmlns=\"urn:xmpp:reactions:0\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:reactions:0\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<reactions xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppMessageReaction::testIsMessageReaction()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppMessageReaction::isMessageReaction(xmlToDom(xml)), isValid);
}

void tst_QXmppMessageReaction::testMessageReaction()
{
    const QByteArray xml(
        "<reactions xmlns=\"urn:xmpp:reactions:0\" id=\"744f6e18-a57a-11e9-a656-4889e7820c76\">"
        "<reaction>🐢</reaction>"
        "<reaction>👋</reaction>"
        "</reactions>");

    QXmppMessageReaction reaction1;
    QVERIFY(reaction1.messageId().isEmpty());
    QVERIFY(reaction1.emojis().isEmpty());

    parsePacket(reaction1, xml);
    QCOMPARE(reaction1.messageId(), QStringLiteral("744f6e18-a57a-11e9-a656-4889e7820c76"));
    QCOMPARE(reaction1.emojis().at(0), QStringLiteral("🐢"));
    QCOMPARE(reaction1.emojis().at(1), QStringLiteral("👋"));

    serializePacket(reaction1, xml);

    QXmppMessageReaction reaction2;
    reaction2.setMessageId(QStringLiteral("744f6e18-a57a-11e9-a656-4889e7820c76"));
    reaction2.setEmojis({ QStringLiteral("🐢"), QStringLiteral("👋") });

    QCOMPARE(reaction1.messageId(), QStringLiteral("744f6e18-a57a-11e9-a656-4889e7820c76"));
    QCOMPARE(reaction1.emojis().at(0), QStringLiteral("🐢"));
    QCOMPARE(reaction1.emojis().at(1), QStringLiteral("👋"));

    serializePacket(reaction2, xml);
}

void tst_QXmppMessageReaction::testMessageReactionWithDuplicateEmojis()
{
    const QByteArray xml(
        "<reactions xmlns=\"urn:xmpp:reactions:0\" id=\"744f6e18-a57a-11e9-a656-4889e7820c76\">"
        "<reaction>🐢</reaction>"
        "<reaction>👋</reaction>"
        "<reaction>🐢</reaction>"
        "<reaction>👋</reaction>"
        "</reactions>");

    QXmppMessageReaction reaction;

    parsePacket(reaction, xml);
    QCOMPARE(reaction.messageId(), QStringLiteral("744f6e18-a57a-11e9-a656-4889e7820c76"));
    QCOMPARE(reaction.emojis().size(), 2);
    QCOMPARE(reaction.emojis().at(0), QStringLiteral("🐢"));
    QCOMPARE(reaction.emojis().at(1), QStringLiteral("👋"));
}

void tst_QXmppMessageReaction::testMessageReactionRemoval()
{
    const QByteArray xml(
        "<reactions xmlns=\"urn:xmpp:reactions:0\" id=\"744f6e18-a57a-11e9-a656-4889e7820c76\"/>");

    QXmppMessageReaction reaction;

    parsePacket(reaction, xml);
    QCOMPARE(reaction.messageId(), QStringLiteral("744f6e18-a57a-11e9-a656-4889e7820c76"));
    QCOMPARE(reaction.emojis().size(), 0);

    serializePacket(reaction, xml);
}

QTEST_MAIN(tst_QXmppMessageReaction)
#include "tst_qxmppmessagereaction.moc"
