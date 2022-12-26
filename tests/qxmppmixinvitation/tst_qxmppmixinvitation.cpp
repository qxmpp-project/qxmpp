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

    QXmppMixInvitation invitation;
    parsePacket(invitation, xml);

    QCOMPARE(invitation.inviterJid(), QStringLiteral("hag66@shakespeare.example"));
    QCOMPARE(invitation.inviteeJid(), QStringLiteral("cat@shakespeare.example"));
    QCOMPARE(invitation.channelJid(), QStringLiteral("coven@mix.shakespeare.example"));
    QCOMPARE(invitation.token(), QStringLiteral("ABCDEF"));

    serializePacket(invitation, xml);

    invitation.setInviterJid("hag66@shakespeare.example");
    invitation.setInviteeJid("cat@shakespeare.example");
    invitation.setChannelJid("coven@mix.shakespeare.example");
    invitation.setToken("ABCDEF");

    QCOMPARE(invitation.inviterJid(), QStringLiteral("hag66@shakespeare.example"));
    QCOMPARE(invitation.inviteeJid(), QStringLiteral("cat@shakespeare.example"));
    QCOMPARE(invitation.channelJid(), QStringLiteral("coven@mix.shakespeare.example"));
    QCOMPARE(invitation.token(), QStringLiteral("ABCDEF"));
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
