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

    QCOMPARE(invitation1.inviterJid(), u"hag66@shakespeare.example"_s);
    QCOMPARE(invitation1.inviteeJid(), u"cat@shakespeare.example"_s);
    QCOMPARE(invitation1.channelJid(), u"coven@mix.shakespeare.example"_s);
    QCOMPARE(invitation1.token(), u"ABCDEF"_s);

    serializePacket(invitation1, xml);

    QXmppMixInvitation invitation2;

    invitation2.setInviterJid(u"hag66@shakespeare.example"_s);
    invitation2.setInviteeJid(u"cat@shakespeare.example"_s);
    invitation2.setChannelJid(u"coven@mix.shakespeare.example"_s);
    invitation2.setToken(u"ABCDEF"_s);

    QCOMPARE(invitation2.inviterJid(), u"hag66@shakespeare.example"_s);
    QCOMPARE(invitation2.inviteeJid(), u"cat@shakespeare.example"_s);
    QCOMPARE(invitation2.channelJid(), u"coven@mix.shakespeare.example"_s);
    QCOMPARE(invitation2.token(), u"ABCDEF"_s);
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
