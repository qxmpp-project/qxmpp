// SPDX-FileCopyrightText: 2023 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAccountMigrationManager.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMixManager.h"
#include "QXmppPubSubManager.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils_p.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

#include "StringLiterals.h"
#include "TestClient.h"
#include "util.h"

using Manager = QXmppAccountMigrationManager;
using namespace QXmpp;
using namespace QXmpp::Private;

bool operator==(const QXmppRosterIq::Item &left, const QXmppRosterIq::Item &right)
{
    return left.bareJid() == right.bareJid() && left.groups() == right.groups() && left.name() == right.name() && left.subscriptionStatus() == right.subscriptionStatus() && left.subscriptionType() == right.subscriptionType() && left.isApproved() == right.isApproved() && left.isMixChannel() == right.isMixChannel() && left.mixParticipantId() == right.mixParticipantId();
}

bool operator==(const QXmppRosterIq &left, const QXmppRosterIq &right)
{
    return left.version() == right.version() && left.items() == right.items() &&
        left.mixAnnotate() == right.mixAnnotate();
}

static QXmppRosterIq::Item newRosterItem(const QString &bareJid, const QString &name, const QSet<QString> &groups = {})
{
    QXmppRosterIq::Item item;
    item.setBareJid(bareJid);
    item.setName(name);
    item.setGroups(groups);
    item.setSubscriptionType(QXmppRosterIq::Item::NotSet);
    return item;
}

static QXmppRosterIq::Item newMixRosterItem(const QString &channelId, const QString &channelName, const QString &participantId)
{
    QXmppRosterIq::Item item;
    item.setBareJid(channelId);
    item.setName(channelName);
    item.setIsMixChannel(true);
    item.setMixParticipantId(participantId);
    item.setSubscriptionType(QXmppRosterIq::Item::NotSet);
    return item;
}

static QXmppRosterIq newRoster(TestClient *client, int version, const std::optional<QString> &id, const std::optional<QXmppIq::Type> &type = {}, int index = -1)
{
    QXmppRosterIq roster;
    roster.setId(id.value_or(QString()));
    roster.setType(type.value_or(QXmppIq::Result));

    if (roster.type() == QXmppIq::Get) {
        roster.setFrom(client->configuration().jid());
        roster.setMixAnnotate(true);
    }

    if (roster.type() == QXmppIq::Result || roster.type() == QXmppIq::Set) {
        switch (version) {
        case 0:
            if (index == -1 || index == 0) {
                roster.addItem(newRosterItem(u"1@bare.com"_s, u"1 Bare"_s, { u"all"_s }));
            }
            if (index == -1 || index == 1) {
                roster.addItem(newMixRosterItem(u"mix1@bare.com"_s, u"Mix 1 Bare"_s, u"mix1BareId"_s));
            }
            break;
        case 1:
            if (index == -1 || index == 0) {
                roster.addItem(newRosterItem(u"3@gamer.com"_s, u"3 Gamer"_s, { u"gamers"_s }));
            }
            if (index == -1 || index == 1) {
                roster.addItem(newMixRosterItem(u"mix2@gamer.com"_s, u"Mix 2 Gamer"_s, u"mix2BareId"_s));
            }
            break;
        default:
            Q_UNREACHABLE();
        }
    }

    return roster;
}

static QXmppVCardIq newClientVCard(TestClient *client, int version, const std::optional<QString> &id, const std::optional<QXmppIq::Type> &type = {})
{
    Q_UNUSED(client)

    QXmppVCardIq vcard;
    vcard.setId(id.has_value() ? *id : QString());
    vcard.setType(type.has_value() ? *type : QXmppIq::Result);

    if (vcard.type() == QXmppIq::Get) {
    }

    if (vcard.type() == QXmppIq::Result || vcard.type() == QXmppIq::Set) {
        switch (version) {
        case 0:
            vcard.setFirstName(u"Nox"_s);
            vcard.setLastName(u"PasNox"_s);
            vcard.setNickName(u"It is me PasNox"_s);
            break;
        case 1:
            vcard.setFirstName(u"Nox"_s);
            vcard.setLastName(u"Bookri"_s);
            vcard.setNickName(u"It is me Bookri"_s);
            break;
        default:
            Q_UNREACHABLE();
        }
    }

    return vcard;
}

static std::unique_ptr<TestClient> newClient(bool withManagers, bool autoResetEnabled = true)
{
    auto client = std::make_unique<TestClient>(false, autoResetEnabled);

    client->addNewExtension<QXmppAccountMigrationManager>();
    client->configuration().setJid("pasnox@xmpp.example");

    if (withManagers) {
        client->addNewExtension<QXmppVCardManager>();
        client->addNewExtension<QXmppDiscoveryManager>();
        client->addNewExtension<QXmppPubSubManager>();
        client->addNewExtension<QXmppRosterManager>(client.get());
        client->addNewExtension<QXmppMixManager>();
    }

    return client;
}

class tst_QXmppAccountMigrationManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testImportExport();
    Q_SLOT void testRealImportExport();
    Q_SLOT void testSerialization();
};

struct DataExtension {
    QString data;
};

static auto parseDataExtension(const QDomElement &el)
{
    return Manager::Result<DataExtension>(DataExtension { el.text() });
}

static auto serializeDataExtension(const DataExtension &ext, QXmlStreamWriter &w)
{
    w.writeStartElement("extension");
    w.writeDefaultNamespace("org.qxmpp.tests");
    w.writeCharacters(ext.data);
    w.writeEndElement();
}

void tst_QXmppAccountMigrationManager::testImportExport()
{

    QXmppExportData::registerExtension<DataExtension, parseDataExtension, serializeDataExtension>(u"extension", u"org.qxmpp.tests");

    auto client = newClient(false);
    auto *manager = client->findExtension<QXmppAccountMigrationManager>();
    Q_ASSERT(manager);
    std::optional<DataExtension> currentState;

    manager->registerExportData<DataExtension>(
        [&](const DataExtension &data) {
            currentState = data;
            return makeReadyTask<Manager::Result<>>(Success());
        },
        [&]() {
            if (currentState) {
                return makeReadyTask<Manager::Result<DataExtension>>(*currentState);
            }
            return makeReadyTask<Manager::Result<DataExtension>>(QXmppError { "No data.", {} });
        });

    auto importTask = manager->importData(QXmppExportData {});
    expectFutureVariant<Success>(importTask);

    // currently no data in 'currentState': expect error
    auto exportTask = manager->exportData();
    expectFutureVariant<QXmppError>(exportTask);

    // set data and expect export to work
    currentState = { "Hello this is a test." };
    exportTask = manager->exportData();
    auto exportData = expectFutureVariant<QXmppExportData>(exportTask);

    // reset state and import data again
    currentState.reset();
    importTask = manager->importData(exportData);
    QVERIFY(currentState);
    QCOMPARE(currentState->data, "Hello this is a test.");

    manager->unregisterExportData<DataExtension>();

    // exporting/importing works without extensions
    // and import data with unknown extensions works
    exportTask = manager->exportData();
    importTask = manager->importData(exportData);
    expectFutureVariant<QXmppExportData>(exportTask);
    expectFutureVariant<Success>(importTask);
}

void tst_QXmppAccountMigrationManager::testRealImportExport()
{
    auto client = newClient(true, false);
    auto *manager = client->findExtension<QXmppAccountMigrationManager>();
    auto *rosterManager = client->findExtension<QXmppRosterManager>();
    auto *vcardManager = client->findExtension<QXmppVCardManager>();

    QVERIFY(manager);
    QVERIFY(rosterManager);
    QVERIFY(vcardManager);

    auto exportTask = manager->exportData();
    QVERIFY(!exportTask.isFinished());

    auto id = client->expectPacketRandomOrder(
        u"<iq from='pasnox@xmpp.example/QXmpp' type='get'>"
        "<query xmlns='jabber:iq:roster'>"
        "<annotate xmlns='urn:xmpp:mix:roster:0'/>"
        "</query>"
        "</iq>"_s);
    client->inject(packetToXml(newRoster(client.get(), 1, id, QXmppIq::Result)));

    id = client->expectPacketRandomOrder(
        u"<iq from='pasnox@xmpp.example/QXmpp' type='get'>"
        "<query xmlns='jabber:iq:roster'>"
        "<annotate xmlns='urn:xmpp:mix:roster:0'/>"
        "</query>"
        "</iq>"_s);
    client->inject(packetToXml(newRoster(client.get(), 1, id, QXmppIq::Result)));

    id = client->expectPacketRandomOrder(
        u"<iq to='pasnox@xmpp.example' type='get'>"
        "<vCard xmlns='vcard-temp'>"
        "<TITLE/>"
        "<ROLE/>"
        "</vCard>"
        "</iq>"_s);
    client->inject(packetToXml(newClientVCard(client.get(), 1, id, QXmppIq::Result)));

    id = client->expectPacketRandomOrder(
        u"<iq to='mix2@gamer.com' type='get'>"
        "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
        "<items node='urn:xmpp:mix:nodes:participants'/>"
        "</pubsub>"
        "</iq>"_s);
    client->inject(
        u"<iq id='%1' from='mix2@gamer.com' type='result'>"
        "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
        "<items node='urn:xmpp:mix:nodes:participants'>"
        "<item id='mix2BareId'>"
        "<participant xmlns='urn:xmpp:mix:core:1'>"
        "<nick>Joe @ Mix 2 Gamer</nick>"
        "<jid>mix_user@domain.ext</jid>"
        "</participant>"
        "</item>"
        "</items>"
        "</pubsub>"
        "</iq>"_s.arg(id));

    client->expectNoPacket();

    auto data = expectFutureVariant<QXmppExportData>(exportTask);

    // import exported data
    auto importTask = manager->importData(data);

    id = client->expectPacketRandomOrder(
        u"<iq to='pasnox@xmpp.example' type='set'>"
        "<client-join xmlns='urn:xmpp:mix:pam:2' channel='mix2@gamer.com'>"
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
        "<nick>Joe @ Mix 2 Gamer</nick>"
        "</join>"
        "</client-join>"
        "</iq>"_s);
    client->inject(
        u"<iq id='%1' type='result'>"
        "<client-join xmlns='urn:xmpp:mix:pam:2'>"
        "<join xmlns='urn:xmpp:mix:core:1' id='mix2BareId'>"
        "<subscribe node='urn:xmpp:mix:nodes:messages'/>"
        "<subscribe node='urn:xmpp:mix:nodes:presence'/>"
        "<nick>Joe @ Mix 2 Gamer</nick>"
        "</join>"
        "</client-join>"
        "</iq>"_s.arg(id));

    id = client->expectPacketRandomOrder(
        u"<iq to='pasnox@xmpp.example' type='set'>"
        "<vCard xmlns='vcard-temp'>"
        "<NICKNAME>It is me Bookri</NICKNAME>"
        "<N><GIVEN>Nox</GIVEN><FAMILY>Bookri</FAMILY></N>"
        "<TITLE/>"
        "<ROLE/>"
        "</vCard>"
        "</iq>"_s);
    client->inject(packetToXml(newClientVCard(client.get(), 1, id, QXmppIq::Result)));

    id = client->expectPacketRandomOrder(
        u"<iq type='set'>"
        "<query xmlns='jabber:iq:roster'>"
        "<item jid='3@gamer.com' name='3 Gamer'>"
        "<group>gamers</group>"
        "</item>"
        "</query>"
        "</iq>"_s);
    client->inject(packetToXml(newRoster(client.get(), 1, id, QXmppIq::Result, 0)));

    client->expectNoPacket();

    expectFutureVariant<Success>(importTask);
}

void tst_QXmppAccountMigrationManager::testSerialization()
{
    auto client = newClient(true, false);
    auto *manager = client->findExtension<QXmppAccountMigrationManager>();
    auto *rosterManager = client->findExtension<QXmppRosterManager>();
    auto *vcardManager = client->findExtension<QXmppVCardManager>();

    QVERIFY(manager);
    QVERIFY(rosterManager);
    QVERIFY(vcardManager);

    // generate export data
    auto exportTask = manager->exportData();
    QVERIFY(!exportTask.isFinished());

    client->expect(u"<iq id='qxmpp2' from='pasnox@xmpp.example/QXmpp' type='get'>"
                   "<query xmlns='jabber:iq:roster'>"
                   "<annotate xmlns='urn:xmpp:mix:roster:0'/>"
                   "</query>"
                   "</iq>"_s);
    client->inject(packetToXml(newRoster(client.get(), 1, "qxmpp2", QXmppIq::Result)));

    client->expect(u"<iq id='qxmpp3' from='pasnox@xmpp.example/QXmpp' type='get'>"
                   "<query xmlns='jabber:iq:roster'>"
                   "<annotate xmlns='urn:xmpp:mix:roster:0'/>"
                   "</query>"
                   "</iq>"_s);
    client->inject(packetToXml(newRoster(client.get(), 1, "qxmpp3", QXmppIq::Result)));

    client->expect(u"<iq id='qxmpp4' to='pasnox@xmpp.example' type='get'>"
                   "<vCard xmlns='vcard-temp'>"
                   "<TITLE/>"
                   "<ROLE/>"
                   "</vCard>"
                   "</iq>"_s);
    client->inject(packetToXml(newClientVCard(client.get(), 1, "qxmpp4", QXmppIq::Result)));

    client->expect(u"<iq id='qxmpp7' to='mix2@gamer.com' type='get'>"
                   "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                   "<items node='urn:xmpp:mix:nodes:participants'/>"
                   "</pubsub>"
                   "</iq>"_s);
    client->inject(u"<iq id='qxmpp7' from='mix2@gamer.com' type='result'>"
                   "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                   "<items node='urn:xmpp:mix:nodes:participants'>"
                   "<item id='mix2BareId'>"
                   "<participant xmlns='urn:xmpp:mix:core:1'>"
                   "<nick>Joe @ Mix 2 Gamer</nick>"
                   "<jid>mix_user@domain.ext</jid>"
                   "</participant>"
                   "</item>"
                   "</items>"
                   "</pubsub>"
                   "</iq>"_s);

    client->expectNoPacket();

    // test serialize
    const auto data = expectFutureVariant<QXmppExportData>(exportTask);

    const auto xml1 = packetToXml(data);
    const QByteArray xml2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<account-data xmlns=\"org.qxmpp.export\" jid=\"pasnox@xmpp.example\">"
        "<mix>"
        "<item jid=\"mix2@gamer.com\" nick=\"Joe @ Mix 2 Gamer\"/>"
        "</mix>"
        "<roster>"
        "<item xmlns=\"jabber:iq:roster\" jid=\"3@gamer.com\" name=\"3 Gamer\"><group>gamers</group></item>"
        "</roster>"
        "<vcard>"
        "<vCard xmlns=\"vcard-temp\">"
        "<NICKNAME>It is me Bookri</NICKNAME>"
        "<N><GIVEN>Nox</GIVEN><FAMILY>Bookri</FAMILY></N>"
        "<TITLE/><ROLE/>"
        "</vCard>"
        "</vcard>"
        "</account-data>\n";

    if (xml1 != xml2) {
        qDebug() << "Actual:\n"
                 << xml1.constData();
        qDebug() << "Expected:\n"
                 << xml2.constData();
    }
    QCOMPARE(xml1, xml2);

    // test parse (and re-serialize)
    auto parsedData = expectVariant<QXmppExportData>(QXmppExportData::fromDom(xmlToDom(xml2)));
    const auto xml3 = packetToXml(parsedData);
    const QByteArray xml4 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<account-data xmlns=\"org.qxmpp.export\" jid=\"pasnox@xmpp.example\">"
        "<mix>"
        "<item jid=\"mix2@gamer.com\" nick=\"Joe @ Mix 2 Gamer\"/>"
        "</mix>"
        "<roster>"
        "<item xmlns=\"jabber:iq:roster\" jid=\"3@gamer.com\" name=\"3 Gamer\"><group>gamers</group></item>"
        "</roster>"
        "<vcard>"
        "<vCard xmlns=\"vcard-temp\">"
        "<NICKNAME>It is me Bookri</NICKNAME>"
        "<N><GIVEN>Nox</GIVEN><FAMILY>Bookri</FAMILY></N>"
        "<TITLE/><ROLE/>"
        "</vCard>"
        "</vcard>"
        "</account-data>\n";

    if (xml3 != xml4) {
        qDebug() << "Actual:\n"
                 << xml3.constData();
        qDebug() << "Expected:\n"
                 << xml4.constData();
    }
    QCOMPARE(xml3, xml4);
}

QTEST_MAIN(tst_QXmppAccountMigrationManager)
#include "tst_qxmppaccountmigrationmanager.moc"
