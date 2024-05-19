// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDiscoveryManager.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixInvitation.h"
#include "QXmppMixIq.h"
#include "QXmppMixManager.h"
#include "QXmppMixParticipantItem.h"
#include "QXmppPubSubEvent.h"
#include "QXmppPubSubManager.h"

#include "TestClient.h"

struct Tester {
    Tester()
    {
        client.addNewExtension<QXmppDiscoveryManager>();
        client.addNewExtension<QXmppPubSubManager>();
        manager = client.addNewExtension<QXmppMixManager>();
    }

    Tester(const QString &jid)
        : Tester()
    {
        client.configuration().setJid(jid);
    }

    TestClient client;
    QXmppMixManager *manager;
};

class tst_QXmppMixManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testDiscoveryFeatures();
    Q_SLOT void testSupportedByServer();
    Q_SLOT void testArchivingSupportedByServer();
    Q_SLOT void testService();
    Q_SLOT void testServices();
    Q_SLOT void testResetCachedData();
    Q_SLOT void testHandleDiscoInfo();
    Q_SLOT void testAddJidToNode();
    Q_SLOT void testRequestJids();
    Q_SLOT void testJoinChannelPrivate();
    Q_SLOT void testPrepareJoinIq();
    Q_SLOT void testHandlePubSubEvent();
    Q_SLOT void testOnRegistered();
    Q_SLOT void testOnUnregistered();
    Q_SLOT void testCreateChannel();
    Q_SLOT void testCreateChannelWithId();
    Q_SLOT void testRequestChannelJids();
    Q_SLOT void testRequestChannelNodes();
    Q_SLOT void testRequestChannelConfiguration();
    Q_SLOT void testUpdateChannelConfiguration();
    Q_SLOT void testRequestChannelInformation();
    Q_SLOT void testUpdateChannelInformation();
    Q_SLOT void testJoinChannel();
    Q_SLOT void testJoinChannelWithNickname();
    Q_SLOT void testJoinChannelWithNodes();
    Q_SLOT void testJoinChannelViaInvitation();
    Q_SLOT void testJoinChannelViaInvitationWithNickname();
    Q_SLOT void testJoinChannelViaInvitationWithNodes();
    Q_SLOT void testUpdateNickname();
    Q_SLOT void testUpdateSubscriptions();
    Q_SLOT void testRequestInvitation();
    Q_SLOT void testRequestAllowedJids();
    Q_SLOT void testAllowJid();
    Q_SLOT void testDisallowJid();
    Q_SLOT void testDisallowAllJids();
    Q_SLOT void testRequestBannedJids();
    Q_SLOT void testBanJid();
    Q_SLOT void testUnbanJid();
    Q_SLOT void testUnbanAllJids();
    Q_SLOT void testRequestParticipants();
    Q_SLOT void testLeaveChannel();
    Q_SLOT void testDeleteChannel();

    template<typename T>
    void testErrorFromChannel(QXmppTask<T> &task, TestClient &client);
    template<typename T>
    void testErrorFromChannel(QXmppTask<T> &task, TestClient &client, const QString &id);
    template<typename T>
    void testError(QXmppTask<T> &task, TestClient &client, const QString &id, const QString &from);
};

void tst_QXmppMixManager::testDiscoveryFeatures()
{
    QXmppMixManager manager;
    QCOMPARE(manager.discoveryFeatures(), QStringList { "urn:xmpp:mix:core:1" });
}

void tst_QXmppMixManager::testSupportedByServer()
{
    QXmppMixManager manager;
    QSignalSpy spy(&manager, &QXmppMixManager::supportedByServerChanged);

    QVERIFY(!manager.supportedByServer());
    manager.setSupportedByServer(true);
    QVERIFY(manager.supportedByServer());
    QCOMPARE(spy.size(), 1);
}

void tst_QXmppMixManager::testArchivingSupportedByServer()
{
    QXmppMixManager manager;
    QSignalSpy spy(&manager, &QXmppMixManager::archivingSupportedByServerChanged);

    QVERIFY(!manager.archivingSupportedByServer());
    manager.setArchivingSupportedByServer(true);
    QVERIFY(manager.archivingSupportedByServer());
    QCOMPARE(spy.size(), 1);
}

void tst_QXmppMixManager::testService()
{
    QXmppMixManager::Service service1;

    QVERIFY(service1.jid.isEmpty());
    QVERIFY(!service1.channelsSearchable);
    QVERIFY(!service1.channelCreationAllowed);

    service1.jid = u"mix.shakespeare.example"_s;
    service1.channelsSearchable = true;
    service1.channelCreationAllowed = false;

    QXmppMixManager::Service service2;
    service2.jid = u"mix.shakespeare.example"_s;
    service2.channelsSearchable = true;
    service2.channelCreationAllowed = false;

    QCOMPARE(service1, service2);

    QXmppMixManager::Service service3;
    service3.jid = u"mix.shakespeare.example"_s;
    service3.channelsSearchable = true;
    service3.channelCreationAllowed = true;

    QVERIFY(!(service1 == service3));
}

void tst_QXmppMixManager::testServices()
{
    QXmppMixManager manager;
    QSignalSpy spy(&manager, &QXmppMixManager::servicesChanged);

    QXmppMixManager::Service service;
    service.jid = u"mix.shakespeare.example"_s;

    QVERIFY(manager.services().isEmpty());

    manager.addService(service);
    QCOMPARE(manager.services().size(), 1);
    QCOMPARE(manager.services().at(0).jid, service.jid);
    manager.addService(service);
    QCOMPARE(spy.size(), 1);

    manager.removeService(u"mix1.shakespeare.example"_s);
    QCOMPARE(manager.services().size(), 1);
    QCOMPARE(spy.size(), 1);

    manager.removeService(service.jid);
    QVERIFY(manager.services().isEmpty());
    QCOMPARE(spy.size(), 2);

    manager.addService(service);
    service.channelsSearchable = true;
    manager.addService(service);
    QCOMPARE(manager.services().size(), 1);
    QCOMPARE(manager.services().at(0).jid, service.jid);
    QCOMPARE(manager.services().at(0).channelsSearchable, service.channelsSearchable);
    QCOMPARE(spy.size(), 4);

    service.jid = u"mix1.shakespeare.example"_s;
    manager.addService(service);
    manager.removeServices();
    QVERIFY(manager.services().isEmpty());
    QCOMPARE(spy.size(), 6);
}

void tst_QXmppMixManager::testResetCachedData()
{
    QXmppMixManager manager;

    QXmppMixManager::Service service;
    service.jid = u"mix.shakespeare.example"_s;

    manager.setSupportedByServer(true);
    manager.setArchivingSupportedByServer(true);
    manager.addService(service);

    manager.resetCachedData();

    QVERIFY(!manager.supportedByServer());
    QVERIFY(!manager.archivingSupportedByServer());
    QVERIFY(manager.services().isEmpty());
}

void tst_QXmppMixManager::testHandleDiscoInfo()
{
    auto [client, manager] = Tester(u"hag66@shakespeare.example"_s);

    QXmppDiscoveryIq::Identity identity;
    identity.setCategory(u"conference"_s);
    identity.setType(u"mix"_s);

    QXmppDiscoveryIq iq;
    iq.setFeatures({ u"urn:xmpp:mix:pam:2"_s,
                     u"urn:xmpp:mix:pam:2#archive"_s,
                     u"urn:xmpp:mix:core:1"_s,
                     u"urn:xmpp:mix:core:1#searchable"_s,
                     u"urn:xmpp:mix:core:1#create-channel"_s });
    iq.setIdentities({ identity });

    manager->handleDiscoInfo(iq);

    QVERIFY(manager->supportedByServer());
    QVERIFY(manager->archivingSupportedByServer());
    QCOMPARE(manager->services().at(0).jid, u"shakespeare.example"_s);
    QVERIFY(manager->services().at(0).channelsSearchable);
    QVERIFY(manager->services().at(0).channelCreationAllowed);

    iq.setFeatures({});
    iq.setIdentities({});

    manager->handleDiscoInfo(iq);

    QVERIFY(!manager->supportedByServer());
    QVERIFY(!manager->archivingSupportedByServer());
    QVERIFY(manager->services().isEmpty());
}

void tst_QXmppMixManager::testAddJidToNode()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->addJidToNode(u"coven@mix.shakespeare.example"_s, u"urn:xmpp:mix:nodes:allowed"_s, u"alice@wonderland.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:allowed'>"
                                 "<item id='alice@wonderland.example'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestJids(u"coven@mix.shakespeare.example"_s, u"urn:xmpp:mix:nodes:allowed"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:allowed'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:allowed'>"
                                 "<item id='shakespeare.example'/>"
                                 "<item id='alice@wonderland.example'/>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto jids = expectFutureVariant<QVector<QXmppMixManager::Jid>>(task);
    QCOMPARE(jids.at(0), u"shakespeare.example"_s);
    QCOMPARE(jids.at(1), u"alice@wonderland.example"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testJoinChannelPrivate()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        QXmppMixInvitation invitation;
        invitation.setInviterJid(u"hag66@shakespeare.example"_s);
        invitation.setInviteeJid(u"cat@shakespeare.example"_s);
        invitation.setChannelJid(u"coven@mix.shakespeare.example"_s);
        invitation.setToken(u"ABCDEF"_s);

        QXmppMixIq iq;
        iq.setType(QXmppIq::Set);
        iq.setTo(u"hag66@shakespeare.example"_s);
        iq.setActionType(QXmppMixIq::ClientJoin);
        iq.setChannelJid(invitation.channelJid());
        iq.setNick(u"third witch"_s);
        iq.setSubscriptions(QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids);
        iq.setInvitation(invitation);

        return manager->joinChannel(std::move(iq));
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='hag66@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:banned'/>"
                                 "<nick>third witch</nick>"
                                 "<invitation xmlns='urn:xmpp:mix:misc:0'>"
                                 "<inviter>hag66@shakespeare.example</inviter>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "<channel>coven@mix.shakespeare.example</channel>"
                                 "<token>ABCDEF</token>"
                                 "</invitation>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123456'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<nick>third witch 2</nick>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, u"123456"_s);
    QCOMPARE(result.nickname, u"third witch 2"_s);
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::AllowedJids);

    testError(task = call(), client, u"qxmpp1"_s, u"hag66@shakespeare.example"_s);
}

void tst_QXmppMixManager::testPrepareJoinIq()
{
    auto [client, manager] = Tester(u"hag66@shakespeare.example"_s);
    auto iq = manager->prepareJoinIq(u"coven@mix.shakespeare.example"_s, u"third witch"_s, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);

    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.to(), u"hag66@shakespeare.example"_s);
    QCOMPARE(iq.actionType(), QXmppMixIq::ClientJoin);
    QCOMPARE(iq.channelJid(), u"coven@mix.shakespeare.example"_s);
    QCOMPARE(iq.nick(), u"third witch"_s);
    QCOMPARE(iq.subscriptions(), QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
}

void tst_QXmppMixManager::testHandlePubSubEvent()
{
    QXmppMixManager manager;
    QSignalSpy jidAllowedSpy(&manager, &QXmppMixManager::jidAllowed);
    QSignalSpy allJidsAllowedSpy(&manager, &QXmppMixManager::allJidsAllowed);
    QSignalSpy jidDisallowedSpy(&manager, &QXmppMixManager::jidDisallowed);
    QSignalSpy allJidsDisallowedSpy(&manager, &QXmppMixManager::allJidsDisallowed);
    QSignalSpy jidBannedSpy(&manager, &QXmppMixManager::jidBanned);
    QSignalSpy jidUnbannedSpy(&manager, &QXmppMixManager::jidUnbanned);
    QSignalSpy allJidsUnbannedSpy(&manager, &QXmppMixManager::allJidsUnbanned);

    QSignalSpy channelConfigurationUpdatedSpy(&manager, &QXmppMixManager::channelConfigurationUpdated);
    QSignalSpy channelInformationUpdatedSpy(&manager, &QXmppMixManager::channelInformationUpdated);
    QSignalSpy participantReceivedSpy(&manager, &QXmppMixManager::participantReceived);
    QSignalSpy participantLeftSpy(&manager, &QXmppMixManager::participantLeft);
    QSignalSpy channelDeletedSpy(&manager, &QXmppMixManager::channelDeleted);

    const auto channelJid = u"coven@mix.shakespeare.example"_s;
    const auto channelName = u"The Coven"_s;
    const QStringList nodes = { u"urn:xmpp:mix:nodes:allowed"_s, u"urn:xmpp:mix:nodes:banned"_s };
    const auto configurationNode = u"urn:xmpp:mix:nodes:config"_s;
    const auto informationNode = u"urn:xmpp:mix:nodes:info"_s;
    const auto participantNode = u"urn:xmpp:mix:nodes:participants"_s;
    const QStringList jids = { u"hag66@shakespeare.example"_s, u"cat@shakespeare.example"_s };

    const QVector<QXmppPubSubEventBase::EventType> eventTypes = { QXmppPubSubEventBase::EventType::Configuration,
                                                                  QXmppPubSubEventBase::EventType::Delete,
                                                                  QXmppPubSubEventBase::EventType::Items,
                                                                  QXmppPubSubEventBase::EventType::Retract,
                                                                  QXmppPubSubEventBase::EventType::Purge,
                                                                  QXmppPubSubEventBase::EventType::Subscription };

    QXmppPubSubBaseItem allowedOrBannedJidsItem1;
    allowedOrBannedJidsItem1.setId(jids.at(0));

    QXmppPubSubBaseItem allowedOrBannedJidsItem2;
    allowedOrBannedJidsItem2.setId(jids.at(1));

    QXmppPubSubEvent<QXmppPubSubBaseItem> allowedOrBannedJidsEvent;
    allowedOrBannedJidsEvent.setItems({ allowedOrBannedJidsItem1, allowedOrBannedJidsItem2 });
    allowedOrBannedJidsEvent.setRetractIds(jids);

    QXmppMixParticipantItem participantItem1;
    participantItem1.setJid(jids.at(0));

    QXmppMixParticipantItem participantItem2;
    participantItem2.setJid(jids.at(1));

    QXmppPubSubEvent<QXmppMixParticipantItem> participantEvent;
    participantEvent.setItems({ participantItem1, participantItem2 });
    participantEvent.setRetractIds(jids);

    QXmppMixConfigItem configurationItem;
    configurationItem.setFormType(QXmppDataForm::Result);
    configurationItem.setOwnerJids(jids);

    QXmppPubSubEvent<QXmppMixConfigItem> configurationEvent;
    configurationEvent.setItems({ configurationItem });
    configurationEvent.setRetractIds(jids);

    QXmppMixInfoItem informationItem;
    informationItem.setFormType(QXmppDataForm::Result);
    informationItem.setName(channelName);

    QXmppPubSubEvent<QXmppMixInfoItem> informationEvent;
    informationEvent.setItems({ informationItem });
    informationEvent.setRetractIds(jids);

    for (const auto &node : nodes) {
        for (auto eventType : eventTypes) {
            allowedOrBannedJidsEvent.setEventType(eventType);
            manager.handlePubSubEvent(writePacketToDom(allowedOrBannedJidsEvent), channelJid, node);
        }
    }

    for (auto eventType : eventTypes) {
        participantEvent.setEventType(eventType);
        manager.handlePubSubEvent(writePacketToDom(participantEvent), channelJid, participantNode);
    }

    for (auto eventType : eventTypes) {
        configurationEvent.setEventType(eventType);
        manager.handlePubSubEvent(writePacketToDom(configurationEvent), channelJid, configurationNode);
    }

    for (auto eventType : eventTypes) {
        informationEvent.setEventType(eventType);
        manager.handlePubSubEvent(writePacketToDom(informationEvent), channelJid, informationNode);
    }

    for (const auto &spy : { &jidAllowedSpy, &jidDisallowedSpy, &jidBannedSpy, &jidUnbannedSpy, &participantLeftSpy }) {
        QCOMPARE(spy->size(), 2);

        for (auto i = 0; i < spy->size(); i++) {
            const auto &arguments = spy->at(i);
            QCOMPARE(arguments.at(0).toString(), channelJid);
            QCOMPARE(arguments.at(1).toString(), jids.at(i));
        }
    }

    for (const auto &spy : { &allJidsAllowedSpy, &allJidsDisallowedSpy }) {
        QCOMPARE(spy->size(), 1);
        auto arguments = spy->constFirst();
        QCOMPARE(arguments.at(0).toString(), channelJid);
    }

    for (const auto &spy : { &allJidsUnbannedSpy, &channelDeletedSpy }) {
        QCOMPARE(spy->size(), 2);
        for (const auto &arguments : *spy) {
            QCOMPARE(arguments.at(0).toString(), channelJid);
        }
    }

    QCOMPARE(participantReceivedSpy.size(), 2);
    for (auto i = 0; i < participantReceivedSpy.size(); i++) {
        const auto &arguments = participantReceivedSpy.at(i);
        QCOMPARE(arguments.at(0).toString(), channelJid);
        QCOMPARE(arguments.at(1).value<QXmppMixParticipantItem>().jid(), participantEvent.items().at(i).jid());
    }

    for (const auto &spy : { &channelConfigurationUpdatedSpy, &channelInformationUpdatedSpy }) {
        QCOMPARE(spy->size(), 1);
        auto arguments = spy->constFirst();
        QCOMPARE(arguments.at(0).toString(), channelJid);
    }

    QCOMPARE(channelConfigurationUpdatedSpy.first().at(1).value<QXmppMixConfigItem>().ownerJids(), jids);
    QCOMPARE(channelInformationUpdatedSpy.first().at(1).value<QXmppMixInfoItem>().name(), channelName);
}

void tst_QXmppMixManager::testOnRegistered()
{
    TestClient client;
    QXmppMixManager manager;

    client.addNewExtension<QXmppDiscoveryManager>();
    client.addNewExtension<QXmppPubSubManager>();

    client.configuration().setJid(u"hag66@shakespeare.example"_s);
    client.addExtension(&manager);

    QXmppMixManager::Service service;
    service.jid = u"mix.shakespeare.example"_s;

    manager.setSupportedByServer(true);
    manager.setArchivingSupportedByServer(true);
    manager.addService(service);

    client.setStreamManagementState(QXmppClient::NewStream);
    Q_EMIT client.connected();
    QVERIFY(!manager.supportedByServer());
    QVERIFY(!manager.archivingSupportedByServer());
    QVERIFY(manager.services().isEmpty());

    QXmppDiscoveryIq iq;
    iq.setFeatures({ u"urn:xmpp:mix:pam:2"_s });
    Q_EMIT manager.client()->findExtension<QXmppDiscoveryManager>()->infoReceived(iq);
    QVERIFY(manager.supportedByServer());
}

void tst_QXmppMixManager::testOnUnregistered()
{
    QXmppClient client;
    QXmppMixManager manager;

    client.addNewExtension<QXmppDiscoveryManager>();
    client.addNewExtension<QXmppPubSubManager>();

    client.configuration().setJid(u"hag66@shakespeare.example"_s);
    client.addExtension(&manager);

    QXmppMixManager::Service service;
    service.jid = u"mix.shakespeare.example"_s;

    manager.setSupportedByServer(true);
    manager.setArchivingSupportedByServer(true);
    manager.addService(service);

    manager.onUnregistered(&client);

    QVERIFY(!manager.supportedByServer());
    QVERIFY(!manager.archivingSupportedByServer());
    QVERIFY(manager.services().isEmpty());

    QXmppDiscoveryIq::Identity identity;
    identity.setCategory(u"conference"_s);
    identity.setType(u"mix"_s);

    QXmppDiscoveryIq iq;
    iq.setFeatures({ u"urn:xmpp:mix:pam:2"_s,
                     u"urn:xmpp:mix:pam:2#archive"_s,
                     u"urn:xmpp:mix:core:1"_s,
                     u"urn:xmpp:mix:core:1#searchable"_s,
                     u"urn:xmpp:mix:core:1#create-channel"_s });
    iq.setIdentities({ identity });

    Q_EMIT manager.client()->findExtension<QXmppDiscoveryManager>()->infoReceived(iq);
    QVERIFY(!manager.supportedByServer());
    QVERIFY(!manager.archivingSupportedByServer());
    QVERIFY(manager.services().isEmpty());

    manager.setSupportedByServer(true);
    manager.setArchivingSupportedByServer(true);
    manager.addService(service);

    Q_EMIT client.connected();
    QVERIFY(manager.supportedByServer());
    QVERIFY(manager.archivingSupportedByServer());
    QVERIFY(!manager.services().isEmpty());
}

void tst_QXmppMixManager::testCreateChannel()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->createChannel(u"mix.shakespeare.example"_s);
    };

    auto task = call();

    client.inject(QStringLiteral("<iq id='qxmpp1' from='mix.shakespeare.example' type='result'>"
                                 "<create xmlns='urn:xmpp:mix:core:1' channel='A1B2C345'/>"
                                 "</iq>"));
    client.expect(QStringLiteral("<iq id='qxmpp1' to='mix.shakespeare.example' type='set'>"
                                 "<create xmlns='urn:xmpp:mix:core:1'/>"
                                 "</iq>"));

    auto channelJid = expectFutureVariant<QXmppMixManager::ChannelJid>(task);
    QCOMPARE(channelJid, u"A1B2C345@mix.shakespeare.example"_s);

    testError(task = call(), client, u"qxmpp1"_s, u"mix.shakespeare.example"_s);
}

void tst_QXmppMixManager::testCreateChannelWithId()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->createChannel(u"mix.shakespeare.example"_s, u"coven"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='mix.shakespeare.example' type='set'>"
                                 "<create xmlns='urn:xmpp:mix:core:1' channel='coven'/>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='mix.shakespeare.example' type='result'>"
                                 "<create xmlns='urn:xmpp:mix:core:1' channel='coven'/>"
                                 "</iq>"));

    auto channelJid = expectFutureVariant<QXmppMixManager::ChannelJid>(task);
    QCOMPARE(channelJid, u"coven@mix.shakespeare.example"_s);

    testError(task = call(), client, u"qxmpp1"_s, u"mix.shakespeare.example"_s);
}

void tst_QXmppMixManager::testRequestChannelJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestChannelJids(u"mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='mix.shakespeare.example' type='get'>"
                                 "<query xmlns='http://jabber.org/protocol/disco#items'/>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='mix.shakespeare.example' type='result'>"
                                 "<query xmlns='http://jabber.org/protocol/disco#items'>"
                                 "<item jid='coven@mix.shakespeare.example'/>"
                                 "<item jid='spells@mix.shakespeare.example'/>"
                                 "<item jid='wizards@mix.shakespeare.example'/>"
                                 "</query>"
                                 "</iq>"));

    auto jids = expectFutureVariant<QVector<QXmppMixManager::ChannelJid>>(task);
    QCOMPARE(jids.size(), 3);
    QCOMPARE(jids.at(0), u"coven@mix.shakespeare.example"_s);
    QCOMPARE(jids.at(1), u"spells@mix.shakespeare.example"_s);
    QCOMPARE(jids.at(2), u"wizards@mix.shakespeare.example"_s);

    testError(task = call(), client, u"qxmpp1"_s, u"mix.shakespeare.example"_s);
}

void tst_QXmppMixManager::testRequestChannelNodes()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestChannelNodes(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<query xmlns='http://jabber.org/protocol/disco#items' node='mix'/>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<query xmlns='http://jabber.org/protocol/disco#items' node='mix'>"
                                 "<item jid='coven@mix.shakespeare.example' node='urn:xmpp:mix:nodes:presence'/>"
                                 "<item jid='coven@mix.shakespeare.example' node='urn:xmpp:mix:nodes:allowed'/>"
                                 "</query>"
                                 "</iq>"));

    auto nodes = expectFutureVariant<QXmppMixConfigItem::Nodes>(task);
    QCOMPARE(nodes, QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::Presence);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestChannelConfiguration()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->requestChannelConfiguration(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:config'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:config'>"
                                 "<item id='2016-05-30T09:00:00'>"
                                 "<x xmlns='jabber:x:data' type='result'>"
                                 "<field type='hidden' var='FORM_TYPE'>"
                                 "<value>urn:xmpp:mix:admin:0</value>"
                                 "</field>"
                                 "<field type='jid-single' var='Last Change Made By'>"
                                 "<value>greymalkin@shakespeare.example</value>"
                                 "</field>"
                                 "</x>"
                                 "</item>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto configuration = expectFutureVariant<QXmppMixConfigItem>(task);
    QCOMPARE(configuration.lastEditorJid(), u"greymalkin@shakespeare.example"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testUpdateChannelConfiguration()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    QXmppMixConfigItem configuration;
    configuration.setId(u"2016-05-30T09:00:00"_s);
    configuration.setOwnerJids({ u"greymalkin@shakespeare.example"_s });

    auto call = [manager, configuration]() {
        return manager->updateChannelConfiguration(u"coven@mix.shakespeare.example"_s, configuration);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:config'>"
                                 "<item id='2016-05-30T09:00:00'>"
                                 "<x xmlns='jabber:x:data' type='submit'>"
                                 "<field type='hidden' var='FORM_TYPE'>"
                                 "<value>urn:xmpp:mix:admin:0</value>"
                                 "</field>"
                                 "<field type='jid-multi' var='Owner'>"
                                 "<value>greymalkin@shakespeare.example</value>"
                                 "</field>"
                                 "</x>"
                                 "</item>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:config'>"
                                 "<item id='2016-05-30T09:00:00'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestChannelInformation()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->requestChannelInformation(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:info'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:info'>"
                                 "<item id='2016-05-30T09:00:00'>"
                                 "<x xmlns='jabber:x:data' type='result'>"
                                 "<field type='hidden' var='FORM_TYPE'>"
                                 "<value>urn:xmpp:mix:core:1</value>"
                                 "</field>"
                                 "<field type='text-single' var='Name'>"
                                 "<value>Witches Coven</value>"
                                 "</field>"
                                 "</x>"
                                 "</item>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto information = expectFutureVariant<QXmppMixInfoItem>(task);
    QCOMPARE(information.name(), u"Witches Coven"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testUpdateChannelInformation()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    QXmppMixInfoItem information;
    information.setId(u"2016-05-30T09:00:00"_s);
    information.setName(u"The Coven"_s);

    auto call = [manager, information]() {
        return manager->updateChannelInformation(u"coven@mix.shakespeare.example"_s, information);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:info'>"
                                 "<item id='2016-05-30T09:00:00'>"
                                 "<x xmlns='jabber:x:data' type='submit'>"
                                 "<field type='hidden' var='FORM_TYPE'>"
                                 "<value>urn:xmpp:mix:core:1</value>"
                                 "</field>"
                                 "<field type='text-single' var='Name'>"
                                 "<value>The Coven</value>"
                                 "</field>"
                                 "</x>"
                                 "</item>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:info'>"
                                 "<item id='2016-05-30T09:00:00'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testJoinChannel()
{
    auto tester = Tester(u"hag66@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        return manager->joinChannel(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='hag66@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<subscribe node='urn:xmpp:avatar:data'/>"
                                 "<subscribe node='urn:xmpp:avatar:metadata'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:banned'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:jidmap'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:participants'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123456'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, u"123456"_s);
    QVERIFY(result.nickname.isEmpty());
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);

    testError(task = call(), client, u"qxmpp1"_s, u"hag66@shakespeare.example"_s);
}

void tst_QXmppMixManager::testJoinChannelWithNickname()
{
    auto [client, manager] = Tester(u"hag66@shakespeare.example"_s);

    auto task = manager->joinChannel(u"coven@mix.shakespeare.example"_s, u"third witch"_s);

    client.expect(QStringLiteral("<iq id='qxmpp1' to='hag66@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<subscribe node='urn:xmpp:avatar:data'/>"
                                 "<subscribe node='urn:xmpp:avatar:metadata'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:banned'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:jidmap'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:participants'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<nick>third witch</nick>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123456'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<nick>third witch</nick>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, u"123456"_s);
    QCOMPARE(result.nickname, u"third witch"_s);
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
}

void tst_QXmppMixManager::testJoinChannelWithNodes()
{
    auto [client, manager] = Tester(u"hag66@shakespeare.example"_s);

    auto task = manager->joinChannel(u"coven@mix.shakespeare.example"_s, {}, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);

    client.expect(QStringLiteral("<iq id='qxmpp1' to='hag66@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123456'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, "123456");
    QVERIFY(result.nickname.isEmpty());
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
}

void tst_QXmppMixManager::testJoinChannelViaInvitation()
{
    auto tester = Tester(u"cat@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [manager]() {
        QXmppMixInvitation invitation;
        invitation.setInviterJid(u"hag66@shakespeare.example"_s);
        invitation.setInviteeJid(u"cat@shakespeare.example"_s);
        invitation.setChannelJid(u"coven@mix.shakespeare.example"_s);
        invitation.setToken(u"ABCDEF"_s);

        return manager->joinChannel(invitation);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='cat@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<subscribe node='urn:xmpp:avatar:data'/>"
                                 "<subscribe node='urn:xmpp:avatar:metadata'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:banned'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:jidmap'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:participants'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<invitation xmlns='urn:xmpp:mix:misc:0'>"
                                 "<inviter>hag66@shakespeare.example</inviter>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "<channel>coven@mix.shakespeare.example</channel>"
                                 "<token>ABCDEF</token>"
                                 "</invitation>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123457'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, u"123457"_s);
    QVERIFY(result.nickname.isEmpty());
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);

    testError(task = call(), client, u"qxmpp1"_s, u"cat@shakespeare.example"_s);
}

void tst_QXmppMixManager::testJoinChannelViaInvitationWithNickname()
{
    auto [client, manager] = Tester(u"cat@shakespeare.example"_s);

    QXmppMixInvitation invitation;
    invitation.setInviterJid(u"hag66@shakespeare.example"_s);
    invitation.setInviteeJid(u"cat@shakespeare.example"_s);
    invitation.setChannelJid(u"coven@mix.shakespeare.example"_s);
    invitation.setToken(u"ABCDEF"_s);

    auto task = manager->joinChannel(invitation, u"fourth witch"_s);

    client.expect(QStringLiteral("<iq id='qxmpp1' to='cat@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:allowed'/>"
                                 "<subscribe node='urn:xmpp:avatar:data'/>"
                                 "<subscribe node='urn:xmpp:avatar:metadata'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:banned'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:jidmap'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:participants'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<nick>fourth witch</nick>"
                                 "<invitation xmlns='urn:xmpp:mix:misc:0'>"
                                 "<inviter>hag66@shakespeare.example</inviter>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "<channel>coven@mix.shakespeare.example</channel>"
                                 "<token>ABCDEF</token>"
                                 "</invitation>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123457'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<nick>fourth witch</nick>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, u"123457"_s);
    QCOMPARE(result.nickname, u"fourth witch"_s);
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
}

void tst_QXmppMixManager::testJoinChannelViaInvitationWithNodes()
{
    auto [client, manager] = Tester(u"cat@shakespeare.example"_s);

    QXmppMixInvitation invitation;
    invitation.setInviterJid(u"hag66@shakespeare.example"_s);
    invitation.setInviteeJid(u"cat@shakespeare.example"_s);
    invitation.setChannelJid(u"coven@mix.shakespeare.example"_s);
    invitation.setToken(u"ABCDEF"_s);

    auto task = manager->joinChannel(invitation, {}, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);

    client.expect(QStringLiteral("<iq id='qxmpp1' to='cat@shakespeare.example' type='set'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<join xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<invitation xmlns='urn:xmpp:mix:misc:0'>"
                                 "<inviter>hag66@shakespeare.example</inviter>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "<channel>coven@mix.shakespeare.example</channel>"
                                 "<token>ABCDEF</token>"
                                 "</invitation>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-join xmlns='urn:xmpp:mix:pam:2'>"
                                 "<join xmlns='urn:xmpp:mix:core:1' id='123457'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "</join>"
                                 "</client-join>"
                                 "</iq>"));

    auto result = expectFutureVariant<QXmppMixManager::Participation>(task);
    QCOMPARE(result.participantId, "123457");
    QVERIFY(result.nickname.isEmpty());
    QCOMPARE(result.subscriptions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
}

void tst_QXmppMixManager::testUpdateNickname()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->updateNickname(u"coven@mix.shakespeare.example"_s, u"third witch"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<setnick xmlns='urn:xmpp:mix:core:1'>"
                                 "<nick>third witch</nick>"
                                 "</setnick>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<setnick xmlns='urn:xmpp:mix:core:1'>"
                                 "<nick>third witch 2</nick>"
                                 "</setnick>"
                                 "</iq>"));

    auto nickname = expectFutureVariant<QXmppMixManager::Nickname>(task);
    QCOMPARE(nickname, "third witch 2");

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testUpdateSubscriptions()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->updateSubscriptions(u"coven@mix.shakespeare.example"_s, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence, QXmppMixConfigItem::Node::Configuration | QXmppMixConfigItem::Node::Information);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<update-subscription xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<unsubscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<unsubscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "</update-subscription>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<update-subscription xmlns='urn:xmpp:mix:core:1'>"
                                 "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
                                 "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
                                 "<unsubscribe node='urn:xmpp:mix:nodes:config'/>"
                                 "<unsubscribe node='urn:xmpp:mix:nodes:info'/>"
                                 "</update-subscription>"
                                 "</iq>"));

    auto subscription = expectFutureVariant<QXmppMixManager::Subscription>(task);
    QCOMPARE(subscription.additions, QXmppMixConfigItem::Node::Messages | QXmppMixConfigItem::Node::Presence);
    QCOMPARE(subscription.removals, QXmppMixConfigItem::Node::Configuration | QXmppMixConfigItem::Node::Information);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestInvitation()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;
    auto logger = client.logger();

    auto call = [&client, manager]() {
        return manager->requestInvitation(u"coven@mix.shakespeare.example"_s, u"cat@shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<invite xmlns='urn:xmpp:mix:misc:0'>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "</invite>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<invite xmlns='urn:xmpp:mix:misc:0'>"
                                 "<invitation xmlns='urn:xmpp:mix:misc:0'>"
                                 "<inviter>hag66@shakespeare.example</inviter>"
                                 "<invitee>cat@shakespeare.example</invitee>"
                                 "<channel>coven@mix.shakespeare.example</channel>"
                                 "<token>ABCDEF</token>"
                                 "</invitation>"
                                 "</invite>"
                                 "</iq>"));

    const auto invitation = expectFutureVariant<QXmppMixInvitation>(task);
    QCOMPARE(invitation.token(), u"ABCDEF"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestAllowedJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestAllowedJids(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:allowed'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:allowed'>"
                                 "<item id='shakespeare.example'/>"
                                 "<item id='alice@wonderland.example'/>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto allowedJids = expectFutureVariant<QVector<QXmppMixManager::Jid>>(task);
    QCOMPARE(allowedJids.at(0), u"shakespeare.example"_s);
    QCOMPARE(allowedJids.at(1), u"alice@wonderland.example"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testAllowJid()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->allowJid(u"coven@mix.shakespeare.example"_s, u"alice@wonderland.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:allowed'>"
                                 "<item id='alice@wonderland.example'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testDisallowJid()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->disallowJid(u"coven@mix.shakespeare.example"_s, u"alice@wonderland.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<retract node='urn:xmpp:mix:nodes:allowed'>"
                                 "<item id='alice@wonderland.example'/>"
                                 "</retract>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testDisallowAllJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->disallowAllJids(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                                 "<purge node='urn:xmpp:mix:nodes:allowed'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestBannedJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestBannedJids(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:banned'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:banned'>"
                                 "<item id='lear@shakespeare.example'/>"
                                 "<item id='macbeth@shakespeare.example'/>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto allowedJids = expectFutureVariant<QVector<QXmppMixManager::Jid>>(task);
    QCOMPARE(allowedJids.at(0), u"lear@shakespeare.example"_s);
    QCOMPARE(allowedJids.at(1), u"macbeth@shakespeare.example"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testBanJid()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->banJid(u"coven@mix.shakespeare.example"_s, u"macbeth@shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<publish node='urn:xmpp:mix:nodes:banned'>"
                                 "<item id='macbeth@shakespeare.example'/>"
                                 "</publish>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testUnbanJid()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->unbanJid(u"coven@mix.shakespeare.example"_s, u"macbeth@shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<retract node='urn:xmpp:mix:nodes:banned'>"
                                 "<item id='macbeth@shakespeare.example'/>"
                                 "</retract>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testUnbanAllJids()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->unbanAllJids(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='set'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                                 "<purge node='urn:xmpp:mix:nodes:banned'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testRequestParticipants()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->requestParticipants(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='coven@mix.shakespeare.example' type='get'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:participants'/>"
                                 "</pubsub>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' from='coven@mix.shakespeare.example' type='result'>"
                                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                 "<items node='urn:xmpp:mix:nodes:participants'>"
                                 "<item id='123456'>"
                                 "<participant xmlns='urn:xmpp:mix:core:1'>"
                                 "<nick>thirdwitch</nick>"
                                 "<jid>hag66@shakespeare.example</jid>"
                                 "</participant>"
                                 "</item>"
                                 "<item id='123457'>"
                                 "<participant xmlns='urn:xmpp:mix:core:1'>"
                                 "<nick>fourthwitch</nick>"
                                 "<jid>hag67@shakespeare.example</jid>"
                                 "</participant>"
                                 "</item>"
                                 "</items>"
                                 "</pubsub>"
                                 "</iq>"));

    auto participants = expectFutureVariant<QVector<QXmppMixParticipantItem>>(task);
    QCOMPARE(participants.at(0).jid(), u"hag66@shakespeare.example"_s);
    QCOMPARE(participants.at(1).jid(), u"hag67@shakespeare.example"_s);

    testErrorFromChannel(task = call(), client);
}

void tst_QXmppMixManager::testLeaveChannel()
{
    auto tester = Tester(u"hag66@shakespeare.example"_s);
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->leaveChannel(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='hag66@shakespeare.example' type='set'>"
                                 "<client-leave xmlns='urn:xmpp:mix:pam:2' channel='coven@mix.shakespeare.example'>"
                                 "<leave xmlns='urn:xmpp:mix:core:1'/>"
                                 "</client-leave>"
                                 "</iq>"));
    client.inject(QStringLiteral("<iq id='qxmpp1' type='result'>"
                                 "<client-leave xmlns='urn:xmpp:mix:pam:2'>"
                                 "<leave xmlns='urn:xmpp:mix:core:1'/>"
                                 "</client-leave>"
                                 "</iq>"));

    expectFutureVariant<QXmpp::Success>(task);

    testError(task = call(), client, u"qxmpp1"_s, u"hag66@shakespeare.example"_s);
}

void tst_QXmppMixManager::testDeleteChannel()
{
    auto tester = Tester();
    auto &client = tester.client;
    auto manager = tester.manager;

    auto call = [&client, manager]() {
        return manager->deleteChannel(u"coven@mix.shakespeare.example"_s);
    };

    auto task = call();

    client.expect(QStringLiteral("<iq id='qxmpp1' to='mix.shakespeare.example' type='set'>"
                                 "<destroy xmlns='urn:xmpp:mix:core:1' channel='coven'/>"
                                 "</iq>"));
    client.inject(u"<iq id='qxmpp1' from='mix.shakespeare.example' type='result'/>"_s);

    expectFutureVariant<QXmpp::Success>(task);

    testError(task = call(), client, u"qxmpp1"_s, u"mix.shakespeare.example"_s);
}

template<typename T>
void tst_QXmppMixManager::testErrorFromChannel(QXmppTask<T> &task, TestClient &client)
{
    testErrorFromChannel(task, client, u"qxmpp1"_s);
}

template<typename T>
void tst_QXmppMixManager::testErrorFromChannel(QXmppTask<T> &task, TestClient &client, const QString &id)
{
    testError(task, client, id, u"coven@mix.shakespeare.example"_s);
}

template<typename T>
void tst_QXmppMixManager::testError(QXmppTask<T> &task, TestClient &client, const QString &id, const QString &from)
{
    client.ignore();
    client.inject(QStringLiteral("<iq id='%1' from='%2' type='error'>"
                                 "<error type='cancel'>"
                                 "<not-allowed xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>"
                                 "</error>"
                                 "</iq>")
                      .arg(id, from));

    expectFutureVariant<QXmppError>(task);
}

QTEST_MAIN(tst_QXmppMixManager)
#include "tst_qxmppmixmanager.moc"
