// SPDX-FileCopyrightText: 2023 Filipe Azevedo <pasnox@gmail.com>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAccountMigrationManager.h"
#include "QXmppClient.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils_p.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

#include "TestClient.h"
#include "util.h"

bool operator==(const QXmppRosterIq::Item &left, const QXmppRosterIq::Item &right)
{
    return left.bareJid() == right.bareJid() && left.groups() == right.groups() && left.name() == right.name() && left.subscriptionStatus() == right.subscriptionStatus() && left.subscriptionType() == right.subscriptionType() && left.isApproved() == right.isApproved() && left.isMixChannel() == right.isMixChannel() && left.mixParticipantId() == right.mixParticipantId();
}

bool operator==(const QXmppRosterIq &left, const QXmppRosterIq &right)
{
    return left.version() == right.version() && left.items() == right.items() &&
        left.mixAnnotate() == right.mixAnnotate();
}

class tst_QXmppAccountMigrationManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasicImportExport();
    Q_SLOT void testImportExport();
    Q_SLOT void testSerializeParse();

    QXmppRosterIq::Item newRosterItem(const QString &bareJid, const QString &name, const QSet<QString> &groups = {}) const
    {
        QXmppRosterIq::Item item;
        item.setBareJid(bareJid);
        item.setName(name);
        item.setGroups(groups);
        item.setSubscriptionType(QXmppRosterIq::Item::NotSet);
        return item;
    }

    QXmppRosterIq newRoster(TestClient *client, int version, const std::optional<QString> &id, const std::optional<QXmppIq::Type> &type = {}) const
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
                roster.addItem(newRosterItem(QStringLiteral("1@bare.com"), QStringLiteral("1 Bare"), { QStringLiteral("all") }));
                roster.addItem(newRosterItem(QStringLiteral("2@bare.com"), QStringLiteral("2 Bare"), { QStringLiteral("all") }));
                roster.addItem(newRosterItem(QStringLiteral("3@bare.com"), QStringLiteral("3 Bare"), { QStringLiteral("all") }));
                break;
            case 1:
                roster.addItem(newRosterItem(QStringLiteral("4@gamer.com"), QStringLiteral("4 Gamer"), { QStringLiteral("gamers") }));
                roster.addItem(newRosterItem(QStringLiteral("5@gamer.com"), QStringLiteral("5 Gamer"), { QStringLiteral("gamers") }));
                roster.addItem(newRosterItem(QStringLiteral("6@gamer.com"), QStringLiteral("6 Gamer"), { QStringLiteral("gamers") }));
                break;
            default:
                Q_UNREACHABLE();
            }
        }

        return roster;
    }

    QXmppVCardIq newClientVCard(TestClient *client, int version, const std::optional<QString> &id, const std::optional<QXmppIq::Type> &type = {}) const
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
                vcard.setFirstName(QStringLiteral("Nox"));
                vcard.setLastName(QStringLiteral("PasNox"));
                vcard.setNickName(QStringLiteral("It is me PasNox"));
                break;
            case 1:
                vcard.setFirstName(QStringLiteral("Nox"));
                vcard.setLastName(QStringLiteral("Bookri"));
                vcard.setNickName(QStringLiteral("It is me Bookri"));
                break;
            default:
                Q_UNREACHABLE();
            }
        }

        return vcard;
    }

    std::unique_ptr<TestClient> newClient(bool withManagers)
    {
        auto client = std::make_unique<TestClient>();

        client->addNewExtension<QXmppAccountMigrationManager>();
        client->configuration().setJid("pasnox@xmpp.example");

        if (withManagers) {
            client->addNewExtension<QXmppRosterManager>(client.get());
            client->addNewExtension<QXmppVCardManager>();
        }

        return client;
    }
};

void tst_QXmppAccountMigrationManager::testBasicImportExport()
{
    struct VCard {
        QString jid;
    };

    struct Roster {
        int count;
    };

    const auto VCardIndex = std::type_index(typeid(VCard));
    const auto RosterIndex = std::type_index(typeid(Roster));
    auto client = newClient(false);
    auto migrationManager = client->findExtension<QXmppAccountMigrationManager>();
    bool validTasks = false;
    QXmppAccountData importedAccount;

    QVERIFY(migrationManager);

    // No registered extensions initially
    {
        QCOMPARE(migrationManager->registeredExtensionsCount(), 0);
    }

    // Register extensions
    {
        migrationManager->registerExtension<VCard>([&](VCard data) {
            QXmppPromise<QXmppAccountMigrationManager::ImportDataResult> promise;
            if (validTasks) {
                importedAccount.addExtension(data);
                promise.finish(QXmpp::Success {});
            } else {
                promise.finish(QXmppError { tr("Not implemented"), {} });
            }
            return promise.task(); }, [&]() {
            QXmppPromise<QXmppAccountMigrationManager::ExportDataResult<VCard>> promise;
            if (validTasks) {
                promise.finish(VCard { QStringLiteral("pasnox@jid.com") });
            } else {
                promise.finish(QXmppError { tr("Not implemented"), {} });
            }
            return promise.task(); });

        migrationManager->registerExtension<Roster>([&](Roster data) {
            QXmppPromise<QXmppAccountMigrationManager::ImportDataResult> promise;
            if (validTasks) {
                importedAccount.addExtension(data);
                promise.finish(QXmpp::Success {});
            } else {
                promise.finish(QXmppError { tr("Not implemented"), {} });
            }
            return promise.task(); }, [&]() {
            QXmppPromise<QXmppAccountMigrationManager::ExportDataResult<Roster>> promise;
            if (validTasks) {
                promise.finish(Roster { 42 });
            } else {
                promise.finish(QXmppError { tr("Not implemented"), {} });
            }
            return promise.task(); });

        QCOMPARE(migrationManager->registeredExtensionsCount(), 2);
    }

    // Invalid import
    {
        importedAccount = {};
        validTasks = false;

        auto task = migrationManager->importData(QXmppAccountData {});

        expectFutureVariant<QXmppError>(task);
    }

    // Valid import
    {
        importedAccount = {};
        validTasks = true;

        QXmppAccountData account;
        account.setSourceBareJid("raynox@xmpp.example");
        account.setExtensions({
            VCard { QStringLiteral("bookri@jid.com") },
            Roster { 64 },
        });

        auto task = migrationManager->importData(account);

        expectFutureVariant<QXmpp::Success>(task);

        QCOMPARE(importedAccount.extensionsCount(), 2);

        for (const auto &extension : importedAccount.extensions()) {
            const auto index = std::type_index(extension.type());

            if (index == VCardIndex) {
                const auto vcard = std::any_cast<VCard>(extension);
                QCOMPARE(vcard.jid, QStringLiteral("bookri@jid.com"));
            } else if (index == RosterIndex) {
                const auto roster = std::any_cast<Roster>(extension);
                QCOMPARE(roster.count, 64);
            } else {
                QVERIFY(false);
            }
        }
    }

    // Invalid export
    {
        importedAccount = {};
        validTasks = false;

        auto task = migrationManager->exportData();

        expectFutureVariant<QXmppError>(task);
    }

    // Valid export
    {
        importedAccount = {};
        validTasks = true;

        auto task = migrationManager->exportData();

        const auto account = expectFutureVariant<QXmppAccountData>(task);

        QCOMPARE(account.sourceBareJid(), client->configuration().jidBare());
        QCOMPARE(account.extensionsCount(), 2);

        for (const auto &extension : account.extensions()) {
            const auto index = std::type_index(extension.type());

            if (index == VCardIndex) {
                const auto vcard = std::any_cast<VCard>(extension);
                QCOMPARE(vcard.jid, QStringLiteral("pasnox@jid.com"));
            } else if (index == RosterIndex) {
                const auto roster = std::any_cast<Roster>(extension);
                QCOMPARE(roster.count, 42);
            } else {
                QVERIFY(false);
            }
        }
    }

    // Unregister extensions
    {
        migrationManager->unregisterExtension<VCard>();
        migrationManager->unregisterExtension<Roster>();

        QCOMPARE(migrationManager->registeredExtensionsCount(), 0);
    }
}

void tst_QXmppAccountMigrationManager::testImportExport()
{
    const auto VCardIndex = std::type_index(typeid(QXmppVCardIq));
    const auto RosterIndex = std::type_index(typeid(QXmppRosterIq));
    auto client = newClient(true);
    auto migrationManager = client->findExtension<QXmppAccountMigrationManager>();
    auto rosterManager = client->findExtension<QXmppRosterManager>();
    auto vcardManager = client->findExtension<QXmppVCardManager>();

    QVERIFY(migrationManager);
    QVERIFY(rosterManager);
    QVERIFY(vcardManager);

    // 2 registered extensions initially
    {
        QCOMPARE(migrationManager->registeredExtensionsCount(), 2);
    }

    // Receive VCard
    {
        client->resetIdCount();
        auto task = vcardManager->requestClientVCard();

        // We could use: m_client->expect(packetToXml(newClientVCard(0, QStringLiteral("qxmpp1"), QXmppIq::Get)));
        // But we prefer to ensure data integrity manually
        client->expect(QStringLiteral("<iq id='qxmpp1' type='get'>"
                                      "<vCard xmlns='vcard-temp'>"
                                      "<TITLE/>"
                                      "<ROLE/>"
                                      "</vCard>"
                                      "</iq>"));

        client->inject(packetToXml(newClientVCard(client.get(), 0, QStringLiteral("qxmpp1"), QXmppIq::Result)));

        auto result = expectFutureVariant<QXmppVCardIq>(task);

        QCOMPARE(result, vcardManager->clientVCard());
        QCOMPARE(result, newClientVCard(client.get(), 0, QStringLiteral("qxmpp1"), QXmppIq::Result));
    }

    // Receive Roster
    {
        client->resetIdCount();
        auto task = rosterManager->requestRoster();

        // We could use: m_client->expect(packetToXml(newRoster(0, QStringLiteral("qxmpp1"), QXmppIq::Get)));
        // But we prefer to ensure data integrity manually
        client->expect(QStringLiteral("<iq id='qxmpp1' from='pasnox@xmpp.example/QXmpp' type='get'>"
                                      "<query xmlns='jabber:iq:roster'>"
                                      "<annotate xmlns='urn:xmpp:mix:roster:0'/>"
                                      "</query>"
                                      "</iq>"));

        client->inject(packetToXml(newRoster(client.get(), 0, QStringLiteral("qxmpp1"), QXmppIq::Result)));

        auto result = expectFutureVariant<QXmppRosterIq>(task);

        QCOMPARE(result, rosterManager->roster());
        QCOMPARE(result, newRoster(client.get(), 0, QStringLiteral("qxmpp1"), QXmppIq::Result));
    }

    // Import
    {
        QXmppAccountData account;
        account.setSourceBareJid("raynox@xmpp.example");
        account.setExtensions({
            newClientVCard(client.get(), 1, QStringLiteral("qxmpp1")),
            newRoster(client.get(), 1, QStringLiteral("qxmpp2")),
        });

        auto task = migrationManager->importData(account);

        client->expect(QStringLiteral("<iq id='qxmpp1' type='set'>"
                                      "<vCard xmlns='vcard-temp'>"
                                      "<NICKNAME>It is me Bookri</NICKNAME>"
                                      "<N><GIVEN>Nox</GIVEN><FAMILY>Bookri</FAMILY></N>"
                                      "<TITLE/>"
                                      "<ROLE/>"
                                      "</vCard>"
                                      "</iq>"));

        client->inject(packetToXml(newClientVCard(client.get(), 1, QStringLiteral("qxmpp1"), QXmppIq::Result)));

        client->expect(QStringLiteral("<iq id='qxmpp2' type='set'>"
                                      "<query xmlns='jabber:iq:roster'>"
                                      "<item jid='4@gamer.com' name='4 Gamer'>"
                                      "<group>gamers</group>"
                                      "</item>"
                                      "<item jid='5@gamer.com' name='5 Gamer'>"
                                      "<group>gamers</group>"
                                      "</item>"
                                      "<item jid='6@gamer.com' name='6 Gamer'>"
                                      "<group>gamers</group>"
                                      "</item>"
                                      "</query>"
                                      "</iq>"));

        client->inject(packetToXml(newRoster(client.get(), 1, QStringLiteral("qxmpp2"), QXmppIq::Result)));

        expectFutureVariant<QXmpp::Success>(task);

        QCOMPARE(vcardManager->clientVCard(), newClientVCard(client.get(), 1, QStringLiteral("qxmpp1")));
        /*
        // New items are appended to existing roster entries
        QCOMPARE(m_rosterManager->roster().items(), newRoster(client.get(), 0, {}).items() + newRoster(client.get(), 1, QStringLiteral("qxmpp2")).items());
        */
        QCOMPARE(rosterManager->roster().items(), newRoster(client.get(), 1, QStringLiteral("qxmpp2")).items());
    }

    // Export
    {
        auto task = migrationManager->exportData();

        auto data = expectFutureVariant<QXmppAccountData>(task);

        QCOMPARE(data.sourceBareJid(), client->configuration().jidBare());
        QCOMPARE(data.extensionsCount(), 2);

        const auto extensions = data.extensions();

        for (const auto &extension : extensions) {
            const auto index = std::type_index(extension.type());

            if (index == VCardIndex) {
                const auto expectedVCard = vcardManager->clientVCard();
                const auto vcard = std::any_cast<QXmppVCardIq>(extension);
                QCOMPARE(vcard, expectedVCard);
            } else if (index == RosterIndex) {
                const auto expectedRoster = rosterManager->roster();
                const auto roster = std::any_cast<QXmppRosterIq>(extension);
                QCOMPARE(roster.items(), expectedRoster.items());
            } else {
                QVERIFY(false);
            }
        }
    }
}

void tst_QXmppAccountMigrationManager::testSerializeParse()
{
    auto client = newClient(true);
    auto migrationManager = client->findExtension<QXmppAccountMigrationManager>();
    auto rosterManager = client->findExtension<QXmppRosterManager>();
    auto vcardManager = client->findExtension<QXmppVCardManager>();

    QVERIFY(migrationManager);
    QVERIFY(rosterManager);
    QVERIFY(vcardManager);

    const auto toXml = [](const QXmppAccountData &account) {
        QString xml;

        {
            QXmlStreamWriter writer(&xml);
            account.toXml(&writer);
        }

        return xml;
    };
    const auto toFormattedXml = [](const QString &content) -> QString {
        QDomDocument doc;
        QVERIFY_RV(doc.setContent(content, true), "Can not format xml");
        return doc.toString(4);
    };
    const auto account = [this, &client]() {
        QXmppAccountData data;

        data.setSourceBareJid("pasnox@xmpp.example");
        data.setExtensions({
            newClientVCard(client.get(), 1, QStringLiteral("qxmpp1")),
            newRoster(client.get(), 1, QStringLiteral("qxmpp2")),
        });

        return data;
    }();
    const QString xml1 = toXml(account);
    const QString xml2 = toXml([&xml1]() -> QXmppAccountData {
        QDomDocument doc;

        QVERIFY_RV(doc.setContent(xml1, true), "Can not set doc content");

        const auto result = QXmppAccountData::fromDom(doc.documentElement());

        QVERIFY_RV(std::holds_alternative<QXmppAccountData>(result), "Not holding QXmppAccountData");

        return std::get<QXmppAccountData>(result);
    }());

    QCOMPARE(toFormattedXml(xml1), toFormattedXml(R"(<?xml version='1.0'?>
<n1:account-data xmlns:n1="org.qxmpp.account-data" sourceBareJid="pasnox@xmpp.example">
    <iq id="qxmpp1" type="result">
        <vCard xmlns="vcard-temp">
            <NICKNAME xmlns="vcard-temp">It is me Bookri</NICKNAME>
            <N xmlns="vcard-temp">
                <GIVEN xmlns="vcard-temp">Nox</GIVEN>
                <FAMILY xmlns="vcard-temp">Bookri</FAMILY>
            </N>
            <TITLE xmlns="vcard-temp"/>
            <ROLE xmlns="vcard-temp"/>
        </vCard>
    </iq>
    <iq id="qxmpp2" type="result">
        <query xmlns="jabber:iq:roster">
            <item xmlns="jabber:iq:roster" name="4 Gamer" jid="4@gamer.com">
                <group xmlns="jabber:iq:roster">gamers</group>
            </item>
            <item xmlns="jabber:iq:roster" name="5 Gamer" jid="5@gamer.com">
                <group xmlns="jabber:iq:roster">gamers</group>
            </item>
            <item xmlns="jabber:iq:roster" name="6 Gamer" jid="6@gamer.com">
                <group xmlns="jabber:iq:roster">gamers</group>
            </item>
        </query>
    </iq>
</n1:account-data>

)"));

    QCOMPARE(xml2, xml1);
}

QTEST_MAIN(tst_QXmppAccountMigrationManager)
#include "tst_qxmppaccountmigrationmanager.moc"
