/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
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

#include "QXmppAtmManager.h"
#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppConstants.cpp"
#include "QXmppConstants_p.h"
#include "QXmppTrustMemoryStorage.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppUtils.h"

#include "util.h"
#include <QObject>
#include <QSet>

Q_DECLARE_METATYPE(QList<QXmppTrustMessageKeyOwner>)

// time period (in ms) to wait for a trust message that should not be sent.
constexpr int UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT = 1000;

class tst_QXmppAtmManager : public QObject
{
    Q_OBJECT

signals:
    void unexpectedTrustMessageSent();

private slots:
    void initTestCase();
    void testSendTrustMessage();
    void testMakePostponedTrustDecisions();
    void testDistrustAutomaticallyTrustedKeys();
    void testDistrust();
    void testAuthenticate_data();
    void testAuthenticate();
    void testMakeTrustDecisions();
    void testHandleMessage_data();
    void testHandleMessage();
    void testMakeTrustDecisionsNoKeys();
    void testMakeTrustDecisionsOwnKeys();
    void testMakeTrustDecisionsOwnKeysNoOwnEndpoints();
    void testMakeTrustDecisionsOwnKeysNoOwnEndpointsWithAuthenticatedKeys();
    void testMakeTrustDecisionsOwnKeysNoContactsWithAuthenticatedKeys();
    void testMakeTrustDecisionsSoleOwnKeyDistrusted();
    void testMakeTrustDecisionsContactKeys();
    void testMakeTrustDecisionsContactKeysNoOwnEndpoints();
    void testMakeTrustDecisionsContactKeysNoOwnEndpointsWithAuthenticatedKeys();
    void testMakeTrustDecisionsSoleContactKeyDistrusted();

private:
    void testMakeTrustDecisionsOwnKeysDone();
    void testMakeTrustDecisionsContactKeysDone();
    void clearTrustStorage();

    QXmppClient m_client;
    QXmppLogger m_logger;
    QXmppAtmManager *m_manager;
    QXmppTrustMemoryStorage *m_trustStorage;
    QXmppCarbonManager *m_carbonManager;
};

void tst_QXmppAtmManager::initTestCase()
{
    m_trustStorage = new QXmppTrustMemoryStorage;
    m_manager = new QXmppAtmManager(m_trustStorage);
    m_client.addExtension(m_manager);
    m_client.configuration().setJid("alice@example.org/phone");

    m_carbonManager = new QXmppCarbonManager;
    m_carbonManager->setCarbonsEnabled(true);
    m_client.addExtension(m_carbonManager);

    m_logger.setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(&m_logger);
}

void tst_QXmppAtmManager::testSendTrustMessage()
{
    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                   QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
                                      QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    keyOwnerBob.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
                                    QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    bool isMessageSent = false;
    const QObject context;

    // trust message to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [=, &isMessageSent, &keyOwnerAlice, &keyOwnerBob](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            isMessageSent = true;

            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            const std::optional<QXmppTrustMessageElement> trustMessageElement = message.trustMessageElement();

            QVERIFY(trustMessageElement);
            QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
            QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

            const auto sentKeyOwners = trustMessageElement->keyOwners();

            QCOMPARE(sentKeyOwners.size(), 2);

            for (auto &sentKeyOwner : sentKeyOwners) {
                if (sentKeyOwner.jid() == keyOwnerAlice.jid()) {
                    QCOMPARE(sentKeyOwner.trustedKeys(), keyOwnerAlice.trustedKeys());
                    QCOMPARE(sentKeyOwner.distrustedKeys(), keyOwnerAlice.distrustedKeys());
                } else if (sentKeyOwner.jid() == keyOwnerBob.jid()) {
                    QCOMPARE(sentKeyOwner.trustedKeys(), keyOwnerBob.trustedKeys());
                    QCOMPARE(sentKeyOwner.distrustedKeys(), keyOwnerBob.distrustedKeys());
                } else {
                    QFAIL("Unexpected key owner sent!");
                }
            }
        }
    });

    m_manager->sendTrustMessage(ns_omemo, { keyOwnerAlice, keyOwnerBob }, QStringLiteral("alice@example.org"));

    QVERIFY(isMessageSent);
}

void tst_QXmppAtmManager::testMakePostponedTrustDecisions()
{
    clearTrustStorage();

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                   QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
                                      QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("c33b0b7420ed386508a0b90701037715db7ce862e3134ef5e85d269dc8bfa556"),
                                                      { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac"),
                                 QStringLiteral("ddba9d09f8506a5b0ea772dcac556e702401e29451582ca805357c28cfe83a16") });
    keyOwnerBob.setDistrustedKeys({ QStringLiteral("6da21f2f14214ebb58e49999bec2da53531e9c0535c3065c23507b33259ad08b"),
                                    QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("705dcb8b775d109cedf6bc3fd5e026312df5fc6fc6e194e97fef706c4b39d470"),
                                                      { keyOwnerBob });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QStringLiteral("3b145a90018ab57cae07db1d1f78090dad57cec575f0100c7157ff9b5bc3dd78") });
    keyOwnerCarol.setDistrustedKeys({ QStringLiteral("4ca6481a110c73e8320a0ac91320a77fb3adba804584eba939685dc0585f6419") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("6609344b11856de4a00f0fd96a7cdafe3ccdaebeadd4b5348d85f677b45178a6"),
                                                      { keyOwnerCarol });

    auto futureVoid = m_manager->makePostponedTrustDecisions(ns_omemo,
                                                             { QStringLiteral("c33b0b7420ed386508a0b90701037715db7ce862e3134ef5e85d269dc8bfa556"),
                                                               QStringLiteral("705dcb8b775d109cedf6bc3fd5e026312df5fc6fc6e194e97fef706c4b39d470") });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    auto futurePotsponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                          { QStringLiteral("c33b0b7420ed386508a0b90701037715db7ce862e3134ef5e85d269dc8bfa556"),
                                                                            QStringLiteral("705dcb8b775d109cedf6bc3fd5e026312df5fc6fc6e194e97fef706c4b39d470") });
    QVERIFY(futurePotsponed.isFinished());
    auto resultPostponed = futurePotsponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QString> trustedKeys = { { QStringLiteral("carol@example.net"),
                                                   QStringLiteral("3b145a90018ab57cae07db1d1f78090dad57cec575f0100c7157ff9b5bc3dd78") } };
    QMultiHash<QString, QString> distrustedKeys = { { QStringLiteral("carol@example.net"),
                                                      QStringLiteral("4ca6481a110c73e8320a0ac91320a77fb3adba804584eba939685dc0585f6419") } };

    futurePotsponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                     { QStringLiteral("6609344b11856de4a00f0fd96a7cdafe3ccdaebeadd4b5348d85f677b45178a6") });
    QVERIFY(futurePotsponed.isFinished());
    resultPostponed = futurePotsponed.result();
    QCOMPARE(
        resultPostponed,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));

    QMultiHash<QString, QString> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5") },
                                                       { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                                                       { QStringLiteral("bob@example.com"),
                                                         QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac") },
                                                       { QStringLiteral("bob@example.com"),
                                                         QStringLiteral("ddba9d09f8506a5b0ea772dcac556e702401e29451582ca805357c28cfe83a16") } };

    auto future = m_trustStorage->keys(ns_omemo,
                                       QXmppTrustStorage::Authenticated);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            QXmppTrustStorage::Authenticated,
            authenticatedKeys) }));

    QMultiHash<QString, QString> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b") },
                                                            { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") },
                                                            { QStringLiteral("bob@example.com"),
                                                              QStringLiteral("6da21f2f14214ebb58e49999bec2da53531e9c0535c3065c23507b33259ad08b") },
                                                            { QStringLiteral("bob@example.com"),
                                                              QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") } };

    future = m_trustStorage->keys(ns_omemo,
                                  QXmppTrustStorage::ManuallyDistrusted);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            QXmppTrustStorage::ManuallyDistrusted,
            manuallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testDistrustAutomaticallyTrustedKeys()
{
    clearTrustStorage();

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") },
        QXmppTrustStorage::Authenticated);

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("75955da0120d2b69fc06459e4f3560d2554e6a1e27ffd200fc8bd0a7352ea35c") },
        QXmppTrustStorage::AutomaticallyTrusted);

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("59efabd40fe48b10da77c7b7f37a139a13c3cbc83e179fe2adc309984113f0c0") },
        QXmppTrustStorage::ManuallyTrusted);

    m_manager->distrustAutomaticallyTrustedKeys(ns_omemo,
                                                { QStringLiteral("alice@example.org"),
                                                  QStringLiteral("bob@example.com") });

    QMultiHash<QString, QString> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                                                                 { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
                                                                 { QStringLiteral("bob@example.com"),
                                                                   QStringLiteral("75955da0120d2b69fc06459e4f3560d2554e6a1e27ffd200fc8bd0a7352ea35c") } };

    auto future = m_trustStorage->keys(ns_omemo,
                                       QXmppTrustStorage::AutomaticallyDistrusted);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            QXmppTrustStorage::AutomaticallyDistrusted,
            automaticallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testDistrust()
{
    clearTrustStorage();

    QMultiHash<QString, QString> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                                                       { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        authenticatedKeys.values(),
        QXmppTrustStorage::Authenticated);

    QMultiHash<QString, QString> automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                                QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        automaticallyTrustedKeys.values(),
        QXmppTrustStorage::AutomaticallyTrusted);

    QMultiHash<QString, QString> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("e858c90ca7305319dc1a46bc46fad3192868f8b5435ffec4d3ea62e6e7aa3814") },
                                                            { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("41f5d8cf0ee59a20f7428b68ea5da4c7e1ee335b66290616db0091faeefcabc0") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        manuallyDistrustedKeys.values(),
        QXmppTrustStorage::ManuallyDistrusted);

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                   QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
                                      QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                                      { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac") });
    keyOwnerBob.setDistrustedKeys({ QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66"),
                                                      { keyOwnerAlice, keyOwnerBob });

    // The entries for the sender key
    // b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363
    // and the keys of keyOwnerBob remain in the storage.
    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363"),
                                                      { keyOwnerBob });

    auto futureVoid = m_manager->distrust(ns_omemo, {});
    QVERIFY(futureVoid.isFinished());

    auto future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys),
                std::pair(
                    QXmppTrustStorage::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));

    futureVoid = m_manager->distrust(ns_omemo,
                                     { std::pair(
                                           QStringLiteral("alice@example.org"),
                                           QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42")),
                                       std::pair(
                                           QStringLiteral("bob@example.com"),
                                           QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66")) });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    authenticatedKeys = { { QStringLiteral("alice@example.org"),
                            QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") } };

    manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                 QStringLiteral("e858c90ca7305319dc1a46bc46fad3192868f8b5435ffec4d3ea62e6e7aa3814") },
                               { QStringLiteral("alice@example.org"),
                                 QStringLiteral("41f5d8cf0ee59a20f7428b68ea5da4c7e1ee335b66290616db0091faeefcabc0") },
                               { QStringLiteral("alice@example.org"),
                                 QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                               { QStringLiteral("bob@example.com"),
                                 QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66") } };

    future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));

    auto futurePostponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                          { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                                                            QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66") });
    QVERIFY(futurePostponed.isFinished());
    auto resultPostponed = futurePostponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QString> trustedKeys = { { QStringLiteral("bob@example.com"),
                                                   QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac") } };
    QMultiHash<QString, QString> distrustedKeys = { { QStringLiteral("bob@example.com"),
                                                      QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") } };

    futurePostponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                     { QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") });
    QVERIFY(futurePostponed.isFinished());
    resultPostponed = futurePostponed.result();
    QCOMPARE(
        resultPostponed,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));
}

void tst_QXmppAtmManager::testAuthenticate_data()
{
    QTest::addColumn<QXmppTrustStorage::SecurityPolicy>("securityPolicy");

    QTest::newRow("noSecurityPolicy")
        << QXmppTrustStorage::NoSecurityPolicy;

    QTest::newRow("toakafa")
        << QXmppTrustStorage::Toakafa;
}

void tst_QXmppAtmManager::testAuthenticate()
{
    clearTrustStorage();

    QFETCH(QXmppTrustStorage::SecurityPolicy, securityPolicy);
    m_trustStorage->setSecurityPolicies(ns_omemo, securityPolicy);

    QMultiHash<QString, QString> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("ad020bd9a95bb924758b4e84640a75b99f37f3351e120188ab6c21c2edecf998") },
                                                       { QStringLiteral("carol@example.net"),
                                                         QStringLiteral("f82419945cb175e4c681b3dbcbb62fbd94f760855c222fb513c3799d273a4130") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        authenticatedKeys.values(QStringLiteral("alice@example.org")),
        QXmppTrustStorage::Authenticated);

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        authenticatedKeys.values(QStringLiteral("carol@example.net")),
        QXmppTrustStorage::Authenticated);

    QMultiHash<QString, QString> automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                                QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66") },
                                                              { QStringLiteral("bob@example.com"),
                                                                QStringLiteral("fddaafd3e44dc8520f74c42227b99210958a544c45794044bd35f13ada883038") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        automaticallyTrustedKeys.values(),
        QXmppTrustStorage::AutomaticallyTrusted);

    QMultiHash<QString, QString> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("e858c90ca7305319dc1a46bc46fad3192868f8b5435ffec4d3ea62e6e7aa3814") },
                                                            { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("41f5d8cf0ee59a20f7428b68ea5da4c7e1ee335b66290616db0091faeefcabc0") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        manuallyDistrustedKeys.values(),
        QXmppTrustStorage::ManuallyDistrusted);

    QMultiHash<QString, QString> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                                                                 { QStringLiteral("alice@example.org"),
                                                                   QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") } };

    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        automaticallyDistrustedKeys.values(),
        QXmppTrustStorage::AutomaticallyDistrusted);

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                   QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
                                      QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                                      { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac") });
    keyOwnerBob.setDistrustedKeys({ QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") });

    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66"),
                                                      { keyOwnerAlice, keyOwnerBob });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QStringLiteral("8a4c33ca6a41b155f3dc0c6aa1f64a5923ecc0d2481a22c60f522d7c60509871") });
    keyOwnerCarol.setDistrustedKeys({ QStringLiteral("f82419945cb175e4c681b3dbcbb62fbd94f760855c222fb513c3799d273a4130") });

    // The keys of keyOwnerCarol are used for trust decisions once Bob's key
    // cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac is
    // authenticated by the authentication of key
    // 9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66.
    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac"),
                                                      { keyOwnerCarol });

    // The entries for the sender key
    // 2e9cf33953840a8e0ddcfe01ec2c5897b0c18421c16ed38135ae051ce2bea43e
    // and the keys of keyOwnerCarol are removed from the storage
    // because they are already used for trust decisions once Bob's key
    // cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac is
    // authenticated.
    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("2e9cf33953840a8e0ddcfe01ec2c5897b0c18421c16ed38135ae051ce2bea43e"),
                                                      { keyOwnerCarol });

    keyOwnerCarol.setTrustedKeys({ QStringLiteral("b3f7d174dd62bab51b6541c6767202ee5ee7965cefe80acbbb0b0ad46720239f") });
    keyOwnerCarol.setDistrustedKeys({ QStringLiteral("f43e44a243657217e059191f77b2fe729be4713082ab07f9b0ac1cc741df1dbb") });

    // The entries for the sender key
    // 2975673c8a9b6a4efeed767ee7c7643e87bac3770dfc6ca32a3f08749b5c6edf
    // and the keys of keyOwnerCarol remain in the storage.
    m_trustStorage->addKeysForPostponedTrustDecisions(ns_omemo,
                                                      QStringLiteral("2975673c8a9b6a4efeed767ee7c7643e87bac3770dfc6ca32a3f08749b5c6edf"),
                                                      { keyOwnerCarol });

    auto futureVoid = m_manager->authenticate(ns_omemo, {});
    QVERIFY(futureVoid.isFinished());

    auto future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys),
                std::pair(
                    QXmppTrustStorage::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys),
                std::pair(
                    QXmppTrustStorage::AutomaticallyDistrusted,
                    automaticallyDistrustedKeys) }));

    futureVoid = m_manager->authenticate(ns_omemo,
                                         { std::pair(
                                               QStringLiteral("alice@example.org"),
                                               QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42")),
                                           std::pair(
                                               QStringLiteral("bob@example.com"),
                                               QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66")) });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    authenticatedKeys = { { QStringLiteral("alice@example.org"),
                            QStringLiteral("ad020bd9a95bb924758b4e84640a75b99f37f3351e120188ab6c21c2edecf998") },
                          { QStringLiteral("alice@example.org"),
                            QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                          { QStringLiteral("bob@example.com"),
                            QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66") },
                          { QStringLiteral("alice@example.org"),
                            QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5") },
                          { QStringLiteral("alice@example.org"),
                            QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                          { QStringLiteral("bob@example.com"),
                            QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac") },
                          { QStringLiteral("carol@example.net"),
                            QStringLiteral("8a4c33ca6a41b155f3dc0c6aa1f64a5923ecc0d2481a22c60f522d7c60509871") } };

    manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                 QStringLiteral("e858c90ca7305319dc1a46bc46fad3192868f8b5435ffec4d3ea62e6e7aa3814") },
                               { QStringLiteral("alice@example.org"),
                                 QStringLiteral("41f5d8cf0ee59a20f7428b68ea5da4c7e1ee335b66290616db0091faeefcabc0") },
                               { QStringLiteral("alice@example.org"),
                                 QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b") },
                               { QStringLiteral("alice@example.org"),
                                 QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") },
                               { QStringLiteral("bob@example.com"),
                                 QStringLiteral("537f949e44e9d7682eb0a6f35b037496a0cb10f6f609d3313f86d8f39abda710") },
                               { QStringLiteral("carol@example.net"),
                                 QStringLiteral("f82419945cb175e4c681b3dbcbb62fbd94f760855c222fb513c3799d273a4130") } };

    if (securityPolicy == QXmppTrustStorage::NoSecurityPolicy) {
        automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") } };

        automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                       QStringLiteral("fddaafd3e44dc8520f74c42227b99210958a544c45794044bd35f13ada883038") } };
    } else if (securityPolicy == QXmppTrustStorage::Toakafa) {
        automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
                                        { QStringLiteral("bob@example.com"),
                                          QStringLiteral("fddaafd3e44dc8520f74c42227b99210958a544c45794044bd35f13ada883038") } };
    }

    future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    switch (securityPolicy) {
    case QXmppTrustStorage::NoSecurityPolicy:
        QCOMPARE(
            result,
            QHash({ std::pair(
                        QXmppTrustStorage::Authenticated,
                        authenticatedKeys),
                    std::pair(
                        QXmppTrustStorage::AutomaticallyTrusted,
                        automaticallyTrustedKeys),
                    std::pair(
                        QXmppTrustStorage::ManuallyDistrusted,
                        manuallyDistrustedKeys),
                    std::pair(
                        QXmppTrustStorage::AutomaticallyDistrusted,
                        automaticallyDistrustedKeys) }));
        break;
    case QXmppTrustStorage::Toakafa:
        QCOMPARE(
            result,
            QHash({ std::pair(
                        QXmppTrustStorage::Authenticated,
                        authenticatedKeys),
                    std::pair(
                        QXmppTrustStorage::ManuallyDistrusted,
                        manuallyDistrustedKeys),
                    std::pair(
                        QXmppTrustStorage::AutomaticallyDistrusted,
                        automaticallyDistrustedKeys) }));
        break;
    }

    auto futurePostponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                          { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                                                            QStringLiteral("9b04f41f0afb686d69fb1d2aeb41f45034849ebf1cafb871bf10c48451ab2e66"),
                                                                            QStringLiteral("cfa3155773071826642a06a99e0f214070a1e7b8999a5710a209939accb7fcac"),
                                                                            QStringLiteral("2e9cf33953840a8e0ddcfe01ec2c5897b0c18421c16ed38135ae051ce2bea43e") });
    QVERIFY(futurePostponed.isFinished());
    auto resultPostponed = futurePostponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QString> trustedKeys = { { QStringLiteral("carol@example.net"),
                                                   QStringLiteral("b3f7d174dd62bab51b6541c6767202ee5ee7965cefe80acbbb0b0ad46720239f") } };
    QMultiHash<QString, QString> distrustedKeys = { { QStringLiteral("carol@example.net"),
                                                      QStringLiteral("f43e44a243657217e059191f77b2fe729be4713082ab07f9b0ac1cc741df1dbb") } };

    futurePostponed = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                     { QStringLiteral("2975673c8a9b6a4efeed767ee7c7643e87bac3770dfc6ca32a3f08749b5c6edf") });
    QVERIFY(futurePostponed.isFinished());
    resultPostponed = futurePostponed.result();
    QCOMPARE(
        resultPostponed,
        QHash({ std::pair(
                    true,
                    trustedKeys),
                std::pair(
                    false,
                    distrustedKeys) }));
}

void tst_QXmppAtmManager::testMakeTrustDecisions()
{
    clearTrustStorage();

    QMultiHash<QString, QString> keysBeingAuthenticated = { { QStringLiteral("alice@example.org"),
                                                              QStringLiteral("6f85db0fb55a88c3721df6f672aecf2c64da5b788033be625d0a4b91caf7af43") },
                                                            { QStringLiteral("bob@example.com"),
                                                              QStringLiteral("3c9cfae387d86abb1810ed44099869aa6aed6e8001e25a8d8128e14229348d23") } };
    QMultiHash<QString, QString> keysBeingDistrusted = { { QStringLiteral("alice@example.org"),
                                                           QStringLiteral("3f0e0a676b8b74456e19359a7926f066c4acb41ccddbea6b2b4183783f07c8a0") },
                                                         { QStringLiteral("bob@example.com"),
                                                           QStringLiteral("3f0e0a676b8b74456e19359a7926f066c4acb41ccddbea6b2b4183783f07c8a0") } };

    auto futureVoid = m_manager->makeTrustDecisions(ns_omemo,
                                                    keysBeingAuthenticated,
                                                    keysBeingDistrusted);
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    auto future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::Authenticated,
                    keysBeingAuthenticated),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    keysBeingDistrusted) }));
}

void tst_QXmppAtmManager::testHandleMessage_data()
{
    QTest::addColumn<QXmppMessage>("message");
    QTest::addColumn<bool>("areTrustDecisionsValid");
    QTest::addColumn<bool>("isSenderKeyAuthenticated");

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ { QStringLiteral("60788b80ba44dddbe8cb831acb1c9c47e04004563dc3a0ffaca663527bb68d26") },
                                   { QStringLiteral("39ca796a1fc03c8e8f87cdcdddf14966a39b5fb472c20fbe0f8128c089a016bc") } });
    keyOwnerAlice.setDistrustedKeys({ { QStringLiteral("d0f3be3a1a53424b8cdc577f0ae85d595b9167362851f433394be970222f2994") },
                                      { QStringLiteral("7e470f60872da85f9bceebe477a75532ff33d04a45a00eec12e50d7bf96f131e") } });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ { QStringLiteral("9ca4facea151343aba1a9590215fc2c1b03ae5fa8df41a38a95c4c7c58f0975c") },
                                 { QStringLiteral("138cf9433f5c583b78f63f095f18d21c6950f57c7a6044815fbba47deb762e16") } });
    keyOwnerBob.setDistrustedKeys({ { QStringLiteral("6f712cbe8341814a62403f4a4479a8b0ffeb47b4fedc103ce0c430e0de9e6665") },
                                    { QStringLiteral("82e465668d105715f74dbcdd8723b0cdd968ac6d199e7768fcff4db16b4c924e") } });

    QList<QXmppTrustMessageKeyOwner> keyOwners;
    keyOwners << keyOwnerAlice << keyOwnerBob;

    QXmppTrustMessageElement trustMessageElement;
    trustMessageElement.setUsage(ns_atm);
    trustMessageElement.setEncryption(ns_omemo);
    trustMessageElement.setKeyOwners(keyOwners);

    QXmppMessage message;
    message.setFrom(m_client.configuration().jid());
    message.setSenderKey(QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"));
    message.setTrustMessageElement(trustMessageElement);

    QTest::newRow("carbonForOwnMessage")
        << message
        << false
        << true;

    message.setFrom(QStringLiteral("alice@example.org/desktop"));
    message.setTrustMessageElement({});

    QTest::newRow("noTrustMessageElement")
        << message
        << false
        << true;

    trustMessageElement.setUsage(QStringLiteral("invalid-usage"));
    message.setTrustMessageElement(trustMessageElement);

    QTest::newRow("trustMessageElementNotForAtm")
        << message
        << false
        << true;

    trustMessageElement.setUsage(ns_atm);
    trustMessageElement.setKeyOwners({});
    message.setTrustMessageElement(trustMessageElement);

    QTest::newRow("trustMessageElementWithoutKeyOwners")
        << message
        << false
        << true;

    trustMessageElement.setKeyOwners(keyOwners);
    trustMessageElement.setEncryption(ns_ox);
    message.setTrustMessageElement(trustMessageElement);

    QTest::newRow("wrongEncryption")
        << message
        << false
        << true;

    trustMessageElement.setEncryption(ns_omemo);
    message.setTrustMessageElement(trustMessageElement);
    message.setFrom(QStringLiteral("carol@example.com/tablet"));

    QTest::newRow("senderNotQualifiedForTrustDecisions")
        << message
        << false
        << true;

    message.setFrom(QStringLiteral("alice@example.org/desktop"));

    QTest::newRow("senderKeyFromOwnEndpointNotAuthenticated")
        << message
        << true
        << false;

    QTest::newRow("trustMessageFromOwnEndpoint")
        << message
        << true
        << true;

    message.setFrom(QStringLiteral("bob@example.com/notebook"));
    message.setSenderKey(QStringLiteral("a9f349b04319f23aeed1d4bbe83b58693c598e2550e65a495818b26948fd5065"));

    QTest::newRow("senderKeyFromContactNotAuthenticated")
        << message
        << true
        << false;

    QTest::newRow("trustMessageFromContactEndpoint")
        << message
        << true
        << true;
}

void tst_QXmppAtmManager::testHandleMessage()
{
    clearTrustStorage();

    QFETCH(QXmppMessage, message);
    QFETCH(bool, areTrustDecisionsValid);
    QFETCH(bool, isSenderKeyAuthenticated);

    const auto senderJid = QXmppUtils::jidToBareJid(message.from());
    const auto senderKey = message.senderKey();

    // Add the sender key in preparation for the test.
    if (areTrustDecisionsValid) {
        if (isSenderKeyAuthenticated) {
            m_trustStorage->addKeys(ns_omemo,
                                    senderJid,
                                    { senderKey },
                                    QXmppTrustStorage::Authenticated);
        } else {
            m_trustStorage->addKeys(ns_omemo,
                                    senderJid,
                                    { senderKey });
        }
    }

    auto future = m_manager->handleMessage(message);
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    // Remove the sender key as soon as the method being tested is executed.
    if (areTrustDecisionsValid) {
        m_trustStorage->removeKeys(ns_omemo, { senderKey });
    }

    if (areTrustDecisionsValid) {
        const auto isOwnMessage = senderJid == m_client.configuration().jidBare();
        const auto keyOwners = message.trustMessageElement()->keyOwners();

        if (isSenderKeyAuthenticated) {
            QMultiHash<QString, QString> authenticatedKeys;
            QMultiHash<QString, QString> manuallyDistrustedKeys;

            if (isOwnMessage) {
                for (const auto &keyOwner : keyOwners) {
                    for (const auto &trustedKey : keyOwner.trustedKeys()) {
                        authenticatedKeys.insert(keyOwner.jid(), trustedKey);
                    }

                    for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                        manuallyDistrustedKeys.insert(keyOwner.jid(), distrustedKey);
                    }
                }

                auto future = m_trustStorage->keys(ns_omemo);
                QVERIFY(future.isFinished());
                auto result = future.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                QXmppTrustStorage::Authenticated,
                                authenticatedKeys),
                            std::pair(
                                QXmppTrustStorage::ManuallyDistrusted,
                                manuallyDistrustedKeys) }));

            } else {
                for (const auto &keyOwner : keyOwners) {
                    if (keyOwner.jid() == senderJid) {
                        for (const auto &trustedKey : keyOwner.trustedKeys()) {
                            authenticatedKeys.insert(keyOwner.jid(), trustedKey);
                        }

                        for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                            manuallyDistrustedKeys.insert(keyOwner.jid(), distrustedKey);
                        }
                    }
                }

                auto future = m_trustStorage->keys(ns_omemo);
                QVERIFY(future.isFinished());
                auto result = future.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                QXmppTrustStorage::Authenticated,
                                authenticatedKeys),
                            std::pair(
                                QXmppTrustStorage::ManuallyDistrusted,
                                manuallyDistrustedKeys) }));
            }
        } else {
            if (isOwnMessage) {
                QMultiHash<QString, QString> trustedKeys;
                QMultiHash<QString, QString> distrustedKeys;

                for (const auto &keyOwner : keyOwners) {
                    for (const auto &trustedKey : keyOwner.trustedKeys()) {
                        trustedKeys.insert(keyOwner.jid(), trustedKey);
                    }

                    for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                        distrustedKeys.insert(keyOwner.jid(), distrustedKey);
                    }
                }

                auto future = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                             { senderKey });
                QVERIFY(future.isFinished());
                auto result = future.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                true,
                                trustedKeys),
                            std::pair(
                                false,
                                distrustedKeys) }));
            } else {
                QMultiHash<QString, QString> trustedKeys;
                QMultiHash<QString, QString> distrustedKeys;

                for (const auto &keyOwner : keyOwners) {
                    if (keyOwner.jid() == senderJid) {
                        for (const auto &trustedKey : keyOwner.trustedKeys()) {
                            trustedKeys.insert(keyOwner.jid(), trustedKey);
                        }

                        for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                            distrustedKeys.insert(keyOwner.jid(), distrustedKey);
                        }
                    }
                }

                auto futureHash = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo,
                                                                                 { senderKey });
                QVERIFY(futureHash.isFinished());
                auto result = futureHash.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                true,
                                trustedKeys),
                            std::pair(
                                false,
                                distrustedKeys) }));
            }
        }
    } else {
        auto futureHash = m_trustStorage->keys(ns_omemo);
        QVERIFY(futureHash.isFinished());
        auto resultHash = futureHash.result();
        QVERIFY(resultHash.isEmpty());

        auto futureHash2 = m_trustStorage->keysForPostponedTrustDecisions(ns_omemo);
        QVERIFY(futureHash2.isFinished());
        auto resultHash2 = futureHash2.result();
        QVERIFY(resultHash2.isEmpty());
    }
}

void tst_QXmppAtmManager::testMakeTrustDecisionsNoKeys()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of own endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") },
        QXmppTrustStorage::ManuallyDistrusted);

    const QObject context;

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &) {
        if (type == QXmppLogger::SentMessage) {
            emit unexpectedTrustMessageSent();
        }
    });

    auto futureVoid = m_manager->makeTrustDecisions(ns_omemo,
                                                    QStringLiteral("alice@example.org"),
                                                    {},
                                                    {});
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    QMultiHash<QString, QString> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
                                                       { QStringLiteral("alice@example.org"),
                                                         QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") } };

    QMultiHash<QString, QString> manuallyDistrustedKeys = { { QStringLiteral("bob@example.com"),
                                                              QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") } };

    auto future = m_trustStorage->keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    QXmppTrustStorage::Authenticated,
                    authenticatedKeys),
                std::pair(
                    QXmppTrustStorage::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeys()
{
    clearTrustStorage();

    // keys of own endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::Authenticated);
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") },
        QXmppTrustStorage::ManuallyDistrusted);

    // keys of contact's endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") },
        QXmppTrustStorage::ManuallyDistrusted);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own keys to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for own keys to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for all keys to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 3);

                for (const auto &keyOwner : keyOwners) {
                    const auto keyOwnerJid = keyOwner.jid();

                    if (keyOwnerJid == QStringLiteral("alice@example.org")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                         QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") }));
                        QCOMPARE(keyOwner.distrustedKeys(),
                                 QList({ QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") }));
                    } else if (keyOwnerJid == QStringLiteral("bob@example.com")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") }));
                        QCOMPARE(keyOwner.distrustedKeys(),
                                 QList({ QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") }));
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("alice@example.org"),
                                                { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                                  QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                                  QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                                                { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4"),
                                                  QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 3);

    testMakeTrustDecisionsOwnKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysNoOwnEndpoints()
{
    clearTrustStorage();

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own keys to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for own keys to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for contacts' keys to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 2);

                for (const auto &keyOwner : keyOwners) {
                    const auto keyOwnerJid = keyOwner.jid();

                    if (keyOwnerJid == QStringLiteral("bob@example.com")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("alice@example.org"),
                                                { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                                  QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                                                { QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 3);

    testMakeTrustDecisionsOwnKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysNoOwnEndpointsWithAuthenticatedKeys()
{
    clearTrustStorage();

    // key of own endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") },
        QXmppTrustStorage::ManuallyDistrusted);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own keys to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for own keys to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") }));
            }
        }
    });

    // trust message for all keys to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 3);

                for (const auto &keyOwner : keyOwners) {
                    const auto keyOwnerJid = keyOwner.jid();

                    if (keyOwnerJid == QStringLiteral("alice@example.org")) {
                        QVERIFY(keyOwner.trustedKeys().isEmpty());
                        QCOMPARE(keyOwner.distrustedKeys(),
                                 QList({ QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") }));
                    } else if (keyOwnerJid == QStringLiteral("bob@example.com")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("alice@example.org"),
                                                { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                                  QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                                                { QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 3);

    testMakeTrustDecisionsOwnKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysNoContactsWithAuthenticatedKeys()
{
    clearTrustStorage();

    // keys of own endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::Authenticated);

    // keys of contact's endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") },
        QXmppTrustStorage::AutomaticallyDistrusted);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own keys to own endpoints with authenticated keys
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));

                if (keyOwner.trustedKeys() == QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"), QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }) &&
                    keyOwner.distrustedKeys() == QList({ QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") })) {
                    sentMessagesCount++;
                }
            }
        }
    });

    // trust message for own keys to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));

                const auto trustedKeys = keyOwner.trustedKeys();
                if (trustedKeys == QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"), QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") })) {
                    sentMessagesCount++;

                    QVERIFY(keyOwner.distrustedKeys().isEmpty());
                }
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("alice@example.org"),
                                                { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                                  QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") },
                                                { QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 2);

    testMakeTrustDecisionsOwnKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsSoleOwnKeyDistrusted()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of own endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own key to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QVERIFY(keyOwner.trustedKeys().isEmpty());
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") }));
            }
        }
    });

    // trust message for own key to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QVERIFY(keyOwner.trustedKeys().isEmpty());
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") }));
            }
        }
    });

    // unexpected trust message for contacts' keys to own endpoint
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                emit unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("alice@example.org"),
                                                {},
                                                { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 2);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    auto futureTrustLevel = m_trustStorage->trustLevel(ns_omemo,
                                                       QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"));
    QVERIFY(futureTrustLevel.isFinished());
    auto result = futureTrustLevel.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeys()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // keys of own endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::Authenticated);
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") },
        QXmppTrustStorage::ManuallyDistrusted);

    // keys of contact's endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"),
          QStringLiteral("4fe76994007cb4649d6d805b4623a6fe3ad2fbc08fbb3187ac7f1999b8f2bcfa") },
        QXmppTrustStorage::Authenticated);
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") },
        QXmppTrustStorage::ManuallyDistrusted);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for Bob's keys to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("bob@example.com"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("9b30de293401566d5c4e6cc5f438c218a6b5e290c00d93952d3f4a87b08aec03"),
                                 QStringLiteral("187ce6ae2fb5539dde1518256d08685e053cbcea675d9d35d9583dd0788bbd6c") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("4fe76994007cb4649d6d805b4623a6fe3ad2fbc08fbb3187ac7f1999b8f2bcfa") }));
            }
        }
    });

    // trust message for own keys to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
                                 QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") }));
            }
        }
    });

    // unexpected trust message to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                emit unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("bob@example.com"),
                                                { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"),
                                                  QStringLiteral("9b30de293401566d5c4e6cc5f438c218a6b5e290c00d93952d3f4a87b08aec03"),
                                                  QStringLiteral("187ce6ae2fb5539dde1518256d08685e053cbcea675d9d35d9583dd0788bbd6c") },
                                                { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82"),
                                                  QStringLiteral("4fe76994007cb4649d6d805b4623a6fe3ad2fbc08fbb3187ac7f1999b8f2bcfa") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 2);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    testMakeTrustDecisionsContactKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeysNoOwnEndpoints()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    const QObject context;

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            emit unexpectedTrustMessageSent();
        }
    });

    m_manager->makeTrustDecisions(ns_omemo,
                                  QStringLiteral("bob@example.com"),
                                  { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"),
                                    QStringLiteral("9b30de293401566d5c4e6cc5f438c218a6b5e290c00d93952d3f4a87b08aec03") },
                                  { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") });

    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    testMakeTrustDecisionsContactKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeysNoOwnEndpointsWithAuthenticatedKeys()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of own endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") },
        QXmppTrustStorage::ManuallyDistrusted);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for own key to Bob
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QVERIFY(keyOwner.trustedKeys().isEmpty());
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("19a1f2b0d85c7c34b31b6aba3804e1446529b8507d13b882451ff598ad532fe4") }));
            }
        }
    });

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() != QStringLiteral("bob@example.com")) {
                emit unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("bob@example.com"),
                                                { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"),
                                                  QStringLiteral("9b30de293401566d5c4e6cc5f438c218a6b5e290c00d93952d3f4a87b08aec03") },
                                                { QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 1);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    testMakeTrustDecisionsContactKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsSoleContactKeyDistrusted()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of own endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") },
        QXmppTrustStorage::Authenticated);

    // key of contact's endpoint
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QStringLiteral("b55cb7ca00675b8aba5764d87bca788bdd38cc3fb1e2b34c45e8313e73c8edfc") },
        QXmppTrustStorage::Authenticated);

    int sentMessagesCount = 0;
    const QObject context;

    // trust message for Bob's key to own endpoints
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                sentMessagesCount++;

                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), QString(ns_atm));
                QCOMPARE(trustMessageElement->encryption(), QString(ns_omemo));

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("bob@example.com"));
                QVERIFY(keyOwner.trustedKeys().isEmpty());
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") }));
            }
        }
    });

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() != QStringLiteral("alice@example.org")) {
                emit unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager->makeTrustDecisions(ns_omemo,
                                                QStringLiteral("bob@example.com"),
                                                {},
                                                { QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58") });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 1);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    const auto futureTrustLevel = m_trustStorage->trustLevel(ns_omemo,
                                                             QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"));
    QVERIFY(futureTrustLevel.isFinished());
    const auto result = futureTrustLevel.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysDone()
{
    auto future = m_trustStorage->trustLevel(ns_omemo,
                                             QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage->trustLevel(ns_omemo,
                                        QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage->trustLevel(ns_omemo,
                                        QStringLiteral("e2206cc893d501f35633f3a0c80f5f6ac3af909f0ad7fd30b98a70546c384393"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeysDone()
{
    auto future = m_trustStorage->trustLevel(ns_omemo,
                                             QStringLiteral("fb5549bcc2c21af903aae67a99067e492fa04db438dfa04953014ea16d0c6b58"));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage->trustLevel(ns_omemo,
                                        QStringLiteral("9b30de293401566d5c4e6cc5f438c218a6b5e290c00d93952d3f4a87b08aec03"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::Authenticated);

    future = m_trustStorage->trustLevel(ns_omemo,
                                        QStringLiteral("f200530b57eca5890ee1a912e9028df97140f4d99ff4d10883b863b65a538c82"));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, QXmppTrustStorage::ManuallyDistrusted);
}

void tst_QXmppAtmManager::clearTrustStorage()
{
    m_trustStorage->removeKeys();
    m_trustStorage->removeKeysForPostponedTrustDecisions();
}

QTEST_MAIN(tst_QXmppAtmManager)
#include "tst_qxmppatmmanager.moc"
