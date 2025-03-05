// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixConfigItem.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixParticipantItem.h"

#include "util.h"

class tst_QXmppMixItem : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testConfig();
    Q_SLOT void testIsConfigItem();
    Q_SLOT void testInfo();
    Q_SLOT void testIsInfoItem();
    Q_SLOT void testParticipant();
    Q_SLOT void testIsParticipantItem();
};

void tst_QXmppMixItem::testConfig()
{
    const QByteArray xml(
        "<item id='2016-05-30T09:00:00'>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:admin:0</value>"
        "</field>"
        "<field type=\"jid-single\" var=\"Last Change Made By\">"
        "<value>greymalkin@shakespeare.example</value>"
        "</field>"
        "<field type=\"jid-multi\" var=\"Owner\">"
        "<value>hecate@shakespeare.example</value>"
        "<value>greymalkin@shakespeare.example</value>"
        "</field>"
        "<field type=\"jid-multi\" var=\"Administrator\">"
        "<value>juliet@shakespeare.example</value>"
        "<value>romeo@shakespeare.example</value>"
        "</field>"
        "<field type=\"text-single\" var=\"End of Life\">"
        "<value>2023-12-31T12:30:00Z</value>"
        "</field>"
        "<field type=\"list-multi\" var=\"Nodes Present\">"
        "<value>allowed</value>"
        "<value>information</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Messages Node Subscription\">"
        "<value>allowed</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Administrator Message Retraction Rights\">"
        "<value>nobody</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Presence Node Subscription\">"
        "<value>allowed</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Participants Node Subscription\">"
        "<value>admins</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Information Node Subscription\">"
        "<value>anyone</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Information Node Update Rights\">"
        "<value>owners</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Allowed Node Subscription\">"
        "<value>allowed</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Banned Node Subscription\">"
        "<value>allowed</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Configuration Node Access\">"
        "<value>allowed</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Avatar Nodes Update Rights\">"
        "<value>participants</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Mandatory Nicks\">"
        "<value>false</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Participants Must Provide Presence\">"
        "<value>true</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Open Presence\">"
        "<value>false</value>"
        "</field>"
        "<field type=\"list-single\" var=\"User Message Retraction\">"
        "<value>true</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Participation Addition by Invitation from Participant\">"
        "<value>true</value>"
        "</field>"
        "<field type=\"list-single\" var=\"Private Messages\">"
        "<value>false</value>"
        "</field>"
        "</x>"
        "</item>");

    QXmppMixConfigItem item1;
    const QDateTime channelDeletion { { 2023, 12, 31 }, { 12, 30 } };

    QCOMPARE(item1.formType(), QXmppDataForm::None);
    QVERIFY(item1.lastEditorJid().isEmpty());
    QVERIFY(item1.ownerJids().isEmpty());
    QVERIFY(item1.administratorJids().isEmpty());
    QVERIFY(!item1.channelDeletion().isValid());
    QVERIFY(!item1.nicknameRequired());
    QVERIFY(!item1.presenceRequired());
    QVERIFY(!item1.onlyParticipantsPermittedToSubmitPresence());
    QVERIFY(!item1.ownMessageRetractionPermitted());
    QVERIFY(!item1.invitationsPermitted());
    QVERIFY(!item1.privateMessagesPermitted());

    parsePacket(item1, xml);
    QCOMPARE(item1.formType(), QXmppDataForm::Result);
    QCOMPARE(item1.lastEditorJid(), u"greymalkin@shakespeare.example"_s);
    QCOMPARE(item1.ownerJids(), QStringList({ u"hecate@shakespeare.example"_s, u"greymalkin@shakespeare.example"_s }));
    QCOMPARE(item1.administratorJids(), QStringList({ u"juliet@shakespeare.example"_s, u"romeo@shakespeare.example"_s }));
    QCOMPARE(item1.channelDeletion(), QDateTime({ { 2023, 12, 31 }, { 12, 30 }, TimeZoneUTC }));
    QCOMPARE(item1.nodes(), QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::Information);
    QCOMPARE(item1.messagesSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item1.messagesRetractRole(), QXmppMixConfigItem::Role::Nobody);
    QCOMPARE(item1.presenceSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item1.participantsSubscribeRole(), QXmppMixConfigItem::Role::Administrator);
    QCOMPARE(item1.informationSubscribeRole(), QXmppMixConfigItem::Role::Anyone);
    QCOMPARE(item1.informationUpdateRole(), QXmppMixConfigItem::Role::Owner);
    QCOMPARE(item1.allowedJidsSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item1.bannedJidsSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item1.configurationReadRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item1.avatarUpdateRole(), QXmppMixConfigItem::Role::Participant);
    QVERIFY(item1.nicknameRequired());
    QVERIFY(!*item1.nicknameRequired());
    QVERIFY(item1.presenceRequired());
    QVERIFY(*item1.presenceRequired());
    QVERIFY(item1.onlyParticipantsPermittedToSubmitPresence());
    QVERIFY(!*item1.onlyParticipantsPermittedToSubmitPresence());
    QVERIFY(item1.ownMessageRetractionPermitted());
    QVERIFY(*item1.ownMessageRetractionPermitted());
    QVERIFY(item1.invitationsPermitted());
    QVERIFY(*item1.invitationsPermitted());
    QVERIFY(item1.privateMessagesPermitted());
    QVERIFY(!*item1.privateMessagesPermitted());
    serializePacket(item1, xml);

    QXmppMixConfigItem item2;
    item2.setId(u"2016-05-30T09:00:00"_s);
    item2.setFormType(QXmppDataForm::Result);
    item2.setLastEditorJid(u"greymalkin@shakespeare.example"_s);
    item2.setOwnerJids(QStringList({ u"hecate@shakespeare.example"_s,
                                     u"greymalkin@shakespeare.example"_s }));
    item2.setAdministratorJids(QStringList({ u"juliet@shakespeare.example"_s,
                                             u"romeo@shakespeare.example"_s }));
    item2.setChannelDeletion({ { 2023, 12, 31 }, { 12, 30 }, TimeZoneUTC });
    item2.setNodes(QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::Information);
    item2.setMessagesSubscribeRole(QXmppMixConfigItem::Role::Allowed);
    item2.setMessagesRetractRole(QXmppMixConfigItem::Role::Nobody);
    item2.setPresenceSubscribeRole(QXmppMixConfigItem::Role::Allowed);
    item2.setParticipantsSubscribeRole(QXmppMixConfigItem::Role::Administrator);
    item2.setInformationSubscribeRole(QXmppMixConfigItem::Role::Anyone);
    item2.setInformationUpdateRole(QXmppMixConfigItem::Role::Owner);
    item2.setAllowedJidsSubscribeRole(QXmppMixConfigItem::Role::Allowed);
    item2.setBannedJidsSubscribeRole(QXmppMixConfigItem::Role::Allowed);
    item2.setConfigurationReadRole(QXmppMixConfigItem::Role::Allowed);
    item2.setAvatarUpdateRole(QXmppMixConfigItem::Role::Participant);
    item2.setNicknameRequired(false);
    item2.setPresenceRequired(true);
    item2.setOnlyParticipantsPermittedToSubmitPresence(false);
    item2.setOwnMessageRetractionPermitted(true);
    item2.setInvitationsPermitted(true);
    item2.setPrivateMessagesPermitted(false);

    QCOMPARE(item2.formType(), QXmppDataForm::Result);
    QCOMPARE(item2.lastEditorJid(), u"greymalkin@shakespeare.example"_s);
    QCOMPARE(item2.ownerJids(), QStringList({ u"hecate@shakespeare.example"_s, u"greymalkin@shakespeare.example"_s }));
    QCOMPARE(item2.administratorJids(), QStringList({ u"juliet@shakespeare.example"_s, u"romeo@shakespeare.example"_s }));
    QCOMPARE(item2.channelDeletion(), QDateTime({ { 2023, 12, 31 }, { 12, 30 }, TimeZoneUTC }));
    QCOMPARE(item2.nodes(), QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::Information);
    QCOMPARE(item2.messagesSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item2.messagesRetractRole(), QXmppMixConfigItem::Role::Nobody);
    QCOMPARE(item2.presenceSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item2.participantsSubscribeRole(), QXmppMixConfigItem::Role::Administrator);
    QCOMPARE(item2.informationSubscribeRole(), QXmppMixConfigItem::Role::Anyone);
    QCOMPARE(item2.informationUpdateRole(), QXmppMixConfigItem::Role::Owner);
    QCOMPARE(item2.allowedJidsSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item2.bannedJidsSubscribeRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item2.configurationReadRole(), QXmppMixConfigItem::Role::Allowed);
    QCOMPARE(item2.avatarUpdateRole(), QXmppMixConfigItem::Role::Participant);
    QVERIFY(item2.nicknameRequired());
    QVERIFY(!*item2.nicknameRequired());
    QVERIFY(item2.presenceRequired());
    QVERIFY(*item2.presenceRequired());
    QVERIFY(item2.onlyParticipantsPermittedToSubmitPresence());
    QVERIFY(!*item2.onlyParticipantsPermittedToSubmitPresence());
    QVERIFY(item2.ownMessageRetractionPermitted());
    QVERIFY(*item2.ownMessageRetractionPermitted());
    QVERIFY(item2.invitationsPermitted());
    QVERIFY(*item2.invitationsPermitted());
    QVERIFY(item2.privateMessagesPermitted());
    QVERIFY(!*item2.privateMessagesPermitted());
    serializePacket(item2, xml);
}

void tst_QXmppMixItem::testIsConfigItem()
{
    const QByteArray xmlCorrect(
        "<item>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:admin:0</value>"
        "</field>"
        "</x>"
        "</item>");
    QVERIFY(QXmppMixConfigItem::isItem(xmlToDom(xmlCorrect)));

    const QByteArray xmlWrong(
        "<item>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>other:namespace</value>"
        "</field>"
        "</x>"
        "</item>");
    QVERIFY(!QXmppMixConfigItem::isItem(xmlToDom(xmlWrong)));
}

void tst_QXmppMixItem::testInfo()
{
    const QByteArray xml(
        "<item>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:core:1</value>"
        "</field>"
        "<field type=\"text-single\" var=\"Name\">"
        "<value>Witches Coven</value>"
        "</field>"
        "<field type=\"text-single\" var=\"Description\">"
        "<value>A location not far from the blasted heath where the "
        "three witches meet</value>"
        "</field>"
        "<field type=\"jid-multi\" var=\"Contact\">"
        "<value>greymalkin@shakespeare.example</value>"
        "<value>joan@shakespeare.example</value>"
        "</field>"
        "</x>"
        "</item>");

    QXmppMixInfoItem item;
    QCOMPARE(item.formType(), QXmppDataForm::None);
    QVERIFY(item.name().isEmpty());
    QVERIFY(item.description().isEmpty());
    QVERIFY(item.contactJids().isEmpty());

    parsePacket(item, xml);
    QCOMPARE(item.formType(), QXmppDataForm::Result);
    QCOMPARE(item.name(), u"Witches Coven"_s);
    QCOMPARE(item.description(), QStringLiteral("A location not far from the blasted "
                                                "heath where the three witches meet"));
    QCOMPARE(item.contactJids(), QStringList() << "greymalkin@shakespeare.example" << "joan@shakespeare.example");

    serializePacket(item, xml);

    // test setters
    item.setFormType(QXmppDataForm::Submit);
    QCOMPARE(item.formType(), QXmppDataForm::Submit);
    item.setName("Skynet Development");
    QCOMPARE(item.name(), u"Skynet Development"_s);
    item.setDescription("Very cool development group.");
    QCOMPARE(item.description(), u"Very cool development group."_s);
    item.setContactJids(QStringList() << "somebody@example.org");
    QCOMPARE(item.contactJids(), QStringList() << "somebody@example.org");
}

void tst_QXmppMixItem::testIsInfoItem()
{
    const QByteArray xmlCorrect(
        "<item>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:core:1</value>"
        "</field>"
        "</x>"
        "</item>");
    QVERIFY(QXmppMixInfoItem::isItem(xmlToDom(xmlCorrect)));

    const QByteArray xmlWrong(
        "<item>"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>other:namespace</value>"
        "</field>"
        "</x>"
        "</item>");
    QVERIFY(!QXmppMixInfoItem::isItem(xmlToDom(xmlWrong)));
}

void tst_QXmppMixItem::testParticipant()
{
    const QByteArray xml(
        "<item>"
        "<participant xmlns=\"urn:xmpp:mix:core:1\">"
        "<jid>hag66@shakespeare.example</jid>"
        "<nick>thirdwitch</nick>"
        "</participant>"
        "</item>");

    QXmppMixParticipantItem item;
    QVERIFY(item.nick().isEmpty());
    QVERIFY(item.jid().isEmpty());

    parsePacket(item, xml);
    QCOMPARE(item.nick(), u"thirdwitch"_s);
    QCOMPARE(item.jid(), u"hag66@shakespeare.example"_s);
    serializePacket(item, xml);

    // test setters
    item.setNick("thomasd");
    QCOMPARE(item.nick(), u"thomasd"_s);
    item.setJid("thomas@d.example");
    QCOMPARE(item.jid(), u"thomas@d.example"_s);
}

void tst_QXmppMixItem::testIsParticipantItem()
{
    const QByteArray xmlCorrect(
        "<item>"
        "<participant xmlns=\"urn:xmpp:mix:core:1\">"
        "</participant>"
        "</item>");
    QVERIFY(QXmppMixParticipantItem::isItem(xmlToDom(xmlCorrect)));

    const QByteArray xmlWrong(
        "<item>"
        "<participant xmlns=\"other:namespace:1\">"
        "</participant>"
        "</item>");
    QVERIFY(!QXmppMixParticipantItem::isItem(xmlToDom(xmlWrong)));
}

QTEST_MAIN(tst_QXmppMixItem)
#include "tst_qxmppmixitems.moc"
