/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin
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

#include "QXmppMixInvitation.h"

#include "util.h"
#include <QDomDocument>
#include <QObject>

class tst_QXmppMixInvitation : public QObject
{
    Q_OBJECT

private slots:
    void testInvitation();
    void testIsInvitation();
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
    QCOMPARE(doc.setContent(correctInvitationWithNamespace, true), true);
    element = doc.documentElement();
    QVERIFY(QXmppMixInvitation::isMixInvitation(element));

    const QByteArray invitationWithoutNamespace(
        "<invitation>"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");
    QCOMPARE(doc.setContent(invitationWithoutNamespace, true), true);
    element = doc.documentElement();
    QVERIFY(!QXmppMixInvitation::isMixInvitation(element));

    const QByteArray invitationWithIncorrectNamespace(
        "<invitation xmlns=\"urn:xmpp:example\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>");
    QCOMPARE(doc.setContent(invitationWithIncorrectNamespace, true), true);
    element = doc.documentElement();
    QVERIFY(!QXmppMixInvitation::isMixInvitation(element));
}

QTEST_MAIN(tst_QXmppMixInvitation)
#include "tst_qxmppmixinvitation.moc"
