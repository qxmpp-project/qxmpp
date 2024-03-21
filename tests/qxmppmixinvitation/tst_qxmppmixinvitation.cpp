// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixInvitation.h"

#include "util.h"

#include <QDomDocument>
#include <QObject>

class tst_QXmppMixInvitation : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testInvitation();
    Q_SLOT void testIsInvitation();
};

void tst_QXmppMixInvitation::testInvitation()
{
    const QByteArray xml(
        "<invitation xmlns=\"urn:xmpp:mix:misc:0\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");

    QXmppMixInvitation invitation1;

    QVERIFY(invitation1.inviterJid().isEmpty());
    QVERIFY(invitation1.inviteeJid().isEmpty());
    QVERIFY(invitation1.channelJid().isEmpty());
    QVERIFY(invitation1.token().isEmpty());

    parsePacket(invitation1, xml);

    QCOMPARE(invitation1.inviterJid(), QStringLiteral("hag66@shakespeare.example"));
    QCOMPARE(invitation1.inviteeJid(), QStringLiteral("cat@shakespeare.example"));
    QCOMPARE(invitation1.channelJid(), QStringLiteral("coven@mix.shakespeare.example"));
    QCOMPARE(invitation1.token(), QStringLiteral("ABCDEF"));

    serializePacket(invitation1, xml);

    QXmppMixInvitation invitation2;

    invitation2.setInviterJid(QStringLiteral("hag66@shakespeare.example"));
    invitation2.setInviteeJid(QStringLiteral("cat@shakespeare.example"));
    invitation2.setChannelJid(QStringLiteral("coven@mix.shakespeare.example"));
    invitation2.setToken(QStringLiteral("ABCDEF"));

    QCOMPARE(invitation2.inviterJid(), QStringLiteral("hag66@shakespeare.example"));
    QCOMPARE(invitation2.inviteeJid(), QStringLiteral("cat@shakespeare.example"));
    QCOMPARE(invitation2.channelJid(), QStringLiteral("coven@mix.shakespeare.example"));
    QCOMPARE(invitation2.token(), QStringLiteral("ABCDEF"));
}

void tst_QXmppMixInvitation::testIsInvitation()
{
    QDomDocument doc;
    QDomElement element;

    const QByteArray correctInvitationWithNamespace(
        "<invitation xmlns=\"urn:xmpp:mix:misc:0\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");
    QVERIFY(doc.setContent(correctInvitationWithNamespace, true));
    element = doc.documentElement();
    QVERIFY(QXmppMixInvitation::isMixInvitation(element));

    const QByteArray invitationWithoutNamespace(
        "<invitation>"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");
    QVERIFY(doc.setContent(invitationWithoutNamespace, true));
    element = doc.documentElement();
    QVERIFY(!QXmppMixInvitation::isMixInvitation(element));

    const QByteArray invitationWithIncorrectNamespace(
        "<invitation xmlns=\"urn:xmpp:example\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");
    QVERIFY(doc.setContent(invitationWithIncorrectNamespace, true));
    element = doc.documentElement();
    QVERIFY(!QXmppMixInvitation::isMixInvitation(element));
}

QTEST_MAIN(tst_QXmppMixInvitation)
#include "tst_qxmppmixinvitation.moc"
