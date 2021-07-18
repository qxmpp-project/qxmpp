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
#include "QXmppFutureUtils_p.h"
#include "QXmppTrustMemoryStorage.h"
#include "QXmppTrustMessageElement.h"

#include "util.h"
#include <QObject>
#include <QSet>

class tst_QXmppAtmManager : public QObject
{
    Q_OBJECT

signals:
    void resultProcessed();
    void resultProcessed2();

private slots:
    void initTestCase();
    void testSendTrustMessage();
    void testMakeTrustDecisionsOwnKeys();
    void testMakeTrustDecisionsOwnKeysNoOwnEndpoints();
    void testMakeTrustDecisionsOwnKeysNoContacts();
    void testMakeTrustDecisionsContactKeys();
    void testAuthenticateManually();
    void testHandleMessageReceived();

private:
    void testMakeTrustDecisionsOwnKeysDone();

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
    m_client.configuration().setJid("alice@example.org");

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

    const auto keyOwners = { keyOwnerAlice, keyOwnerBob };

    const auto *context = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, context, [=, &keyOwners, &keyOwnerAlice, &keyOwnerBob](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            const std::optional<QXmppTrustMessageElement> trustMessageElement = message.trustMessageElement();

            QVERIFY(trustMessageElement);
            QCOMPARE(trustMessageElement->usage(), ns_atm);
            QCOMPARE(trustMessageElement->encryption(), ns_omemo);

            const auto sentKeyOwners = trustMessageElement->keyOwners();
            const auto sentKeyOwnerAlice = sentKeyOwners.at(0);
            const auto sentKeyOwnerBob = sentKeyOwners.at(1);

            QCOMPARE(sentKeyOwners.size(), keyOwners.size());

            QCOMPARE(sentKeyOwnerAlice.jid(), keyOwnerAlice.jid());
            QCOMPARE(sentKeyOwnerAlice.trustedKeys(), keyOwnerAlice.trustedKeys());
            QCOMPARE(sentKeyOwnerAlice.distrustedKeys(), keyOwnerAlice.distrustedKeys());

            QCOMPARE(sentKeyOwnerBob.jid(), keyOwnerBob.jid());
            QCOMPARE(sentKeyOwnerBob.trustedKeys(), keyOwnerBob.trustedKeys());
            QCOMPARE(sentKeyOwnerBob.distrustedKeys(), keyOwnerBob.distrustedKeys());

            // Disconnect the lambda expression.
            delete context;
        }
    });

    m_manager->sendTrustMessage(ns_omemo, keyOwners, QStringLiteral("alice@example.org"));
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeys()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

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

    const auto *trustMessageForOwnKeysToBob = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToBob, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));

                // Disconnect the lambda expression.
                delete trustMessageForOwnKeysToBob;

                emit resultProcessed();
            }
        }
    });

    const auto *trustMessageForOwnKeysToCarol = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToCarol, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));

                // Disconnect the lambda expression.
                delete trustMessageForOwnKeysToCarol;

                emit resultProcessed();
            }
        }
    });

    const auto *trustMessageForAllKeysToOwnEndpoints = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForAllKeysToOwnEndpoints, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

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
                        QVERIFY(false);
                    }
                }

                // Disconnect the lambda expression.
                delete trustMessageForAllKeysToOwnEndpoints;

                emit resultProcessed();
            }
        }
    });

    m_manager->makeTrustDecisions(ns_omemo,
                                  QStringLiteral("alice@example.org"),
                                  { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                    QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });

    while (resultProcessedSpy.count() < 3) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }

    testMakeTrustDecisionsOwnKeysDone();

    m_trustStorage->removeKeys();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysNoOwnEndpoints()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

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

    const auto *trustMessageForOwnKeysToBob = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToBob, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("bob@example.com")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QVERIFY(keyOwner.distrustedKeys().isEmpty());

                // Disconnect the lambda expression.
                delete trustMessageForOwnKeysToBob;

                emit resultProcessed();
            }
        }
    });

    const auto *trustMessageForOwnKeysToCarol = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToCarol, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
                QCOMPARE(keyOwner.trustedKeys(),
                         QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                 QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") }));
                QVERIFY(keyOwner.distrustedKeys().isEmpty());

                // Disconnect the lambda expression.
                delete trustMessageForOwnKeysToCarol;

                emit resultProcessed();
            }
        }
    });

    const auto *trustMessageForContactsKeysToOwnEndpoints = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForContactsKeysToOwnEndpoints, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

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
                        QVERIFY(false);
                    }
                }

                // Disconnect the lambda expression.
                delete trustMessageForContactsKeysToOwnEndpoints;

                emit resultProcessed();
            }
        }
    });

    m_manager->makeTrustDecisions(ns_omemo,
                                  QStringLiteral("alice@example.org"),
                                  { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                    QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });

    while (resultProcessedSpy.count() < 3) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }

    testMakeTrustDecisionsOwnKeysDone();

    m_trustStorage->removeKeys();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysNoContacts()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

    // keys of own endpoints
    m_trustStorage->addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"),
          QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") },
        QXmppTrustStorage::Authenticated);

    const auto *trustMessageForOwnKeysToOwnEndpointsWithAuthenticatedKeys = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToOwnEndpointsWithAuthenticatedKeys, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));

                const auto trustedKeys = keyOwner.trustedKeys();
                if (trustedKeys == QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"), QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") })) {
                    QVERIFY(keyOwner.distrustedKeys().isEmpty());

                    // Disconnect the lambda expression.
                    delete trustMessageForOwnKeysToOwnEndpointsWithAuthenticatedKeys;

                    emit resultProcessed();
                }
            }
        }
    });

    const auto *trustMessageForOwnKeysToOwnEndpoints = new QObject(this);
    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToOwnEndpoints, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                const auto trustMessageElement = message.trustMessageElement();

                QVERIFY(trustMessageElement);
                QCOMPARE(trustMessageElement->usage(), ns_atm);
                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

                const auto keyOwners = trustMessageElement->keyOwners();
                QCOMPARE(keyOwners.size(), 1);

                const auto keyOwner = keyOwners.at(0);
                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));

                const auto trustedKeys = keyOwner.trustedKeys();
                if (trustedKeys == QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42"), QStringLiteral("b5fb24aee735c5c7c2f952b3baabcb65425565c7195ff3e0e63f3cba4a6e6363") })) {
                    QVERIFY(keyOwner.distrustedKeys().isEmpty());

                    // Disconnect the lambda expression.
                    delete trustMessageForOwnKeysToOwnEndpoints;

                    emit resultProcessed();
                }
            }
        }
    });

    m_manager->makeTrustDecisions(ns_omemo,
                                  QStringLiteral("alice@example.org"),
                                  { QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
                                    QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });

    QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");

    testMakeTrustDecisionsOwnKeysDone();

    m_trustStorage->removeKeys();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeys()
{
    //    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

    //    // key of current own endpoint
    //    m_trustStorage->addKeys(
    //        ns_omemo,
    //        QStringLiteral("alice@example.org"),
    //        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
    //        QXmppTrustStorage::Authenticated
    //    );

    //    // key of contact's endpoint
    //    m_trustStorage->addKeys(
    //        ns_omemo,
    //        QStringLiteral("bob@example.com"),
    //        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
    //        QXmppTrustStorage::Authenticated
    //    );

    //    auto *trustMessageForBobsAuthenticatedKeysToOneself = new QObject(this);
    //    connect(&m_logger, &QXmppLogger::message, trustMessageForBobsAuthenticatedKeysToOneself, [=](QXmppLogger::MessageType type, const QString &text) {
    //        if (type == QXmppLogger::SentMessage) {
    //            QXmppMessage message;
    //            parsePacket(message, text.toUtf8());

    //            if (message.to() == QStringLiteral("alice@example.org")) {
    //                const std::optional<QXmppTrustMessageElement> trustMessageElement = message.trustMessageElement();

    //                QVERIFY(trustMessageElement);
    //                QCOMPARE(trustMessageElement->usage(), ns_atm);
    //                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

    //                auto keyOwners = trustMessageElement->keyOwners();
    //                QCOMPARE(keyOwners.size(), 1);

    //                auto keyOwner = keyOwners.at(0);
    //                QCOMPARE(keyOwner.jid(), QStringLiteral("bob@example.com"));
    //                QCOMPARE(keyOwner.trustedKeys(),
    //                         QList({ QStringLiteral("850b27ba48963a8475f926f41210cfa0408c164be3d620888f456bd920a4e36e"),
    //                                 QStringLiteral("f758f0d539774a2de5bd8bb6b46cb6f8804593a87f7cc469b9e6ead9ebe1f2b4") }));
    //                QVERIFY(keyOwner.distrustedKeys().isEmpty());

    //                // Disconnect the lambda expression.
    //                delete trustMessageForBobsAuthenticatedKeysToOneself;

    //                emit resultProcessed();
    //            }
    //        }
    //    });

    //    auto *trustMessageForOwnKeysToBob = new QObject(this);
    //    connect(&m_logger, &QXmppLogger::message, trustMessageForOwnKeysToBob, [=](QXmppLogger::MessageType type, const QString &text) {
    //        if (type == QXmppLogger::SentMessage) {
    //            QXmppMessage message;
    //            parsePacket(message, text.toUtf8());

    //            if (message.to() == QStringLiteral("bob@example.com")) {
    //                const std::optional<QXmppTrustMessageElement> trustMessageElement = message.trustMessageElement();

    //                QVERIFY(trustMessageElement);
    //                QCOMPARE(trustMessageElement->usage(), ns_atm);
    //                QCOMPARE(trustMessageElement->encryption(), ns_omemo);

    //                auto keyOwners = trustMessageElement->keyOwners();
    //                QCOMPARE(keyOwners.size(), 1);

    //                auto keyOwner = keyOwners.at(0);
    //                QCOMPARE(keyOwner.jid(), QStringLiteral("alice@example.org"));
    //                QCOMPARE(keyOwner.trustedKeys(),
    //                         QList({ QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") }));
    //                QVERIFY(keyOwner.distrustedKeys().isEmpty());

    //                // Disconnect the lambda expression.
    //                delete trustMessageForOwnKeysToBob;

    //                emit resultProcessed();
    //            }
    //        }
    //    });

    //    m_manager->makeTrustDecisions(ns_omemo,
    //                                 QStringLiteral("bob@example.com"),
    //                                 { QStringLiteral("850b27ba48963a8475f926f41210cfa0408c164be3d620888f456bd920a4e36e"),
    //                                   QStringLiteral("f758f0d539774a2de5bd8bb6b46cb6f8804593a87f7cc469b9e6ead9ebe1f2b4") });

    //    while (resultProcessedSpy.count() < 2) {
    //        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    //    }

    //    m_trustStorage->removeKeys();
}

void tst_QXmppAtmManager::testAuthenticateManually()
{
    //    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

    //    QObject *context = new QObject(this);
    //    connect(&m_logger, &QXmppLogger::message, context, [=](QXmppLogger::MessageType type, const QString &text) {
    //        QCOMPARE(type, QXmppLogger::SentMessage);

    //        QXmppMessage message;
    //        parsePacket(message, text.toUtf8());

    //        const std::optional<QXmppTrustMessageElement> trustMessageElement = message.trustMessageElement();

    //        QVERIFY(trustMessageElement);
    //        QCOMPARE(trustMessageElement->keyOwners(), keyOwners);

    //        if (type == QXmppLogger::SentMessage) {
    //            emit resultProcessed();
    //        }

    // Disconnect the lambda expression.
    //        delete context;
    //    });

    //    // key of current own endpoint added by encryption implementation
    //    m_trustStorage->addKeys(
    //        ns_omemo,
    //        QStringLiteral("alice@example.org"),
    //        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
    //        QXmppTrustStorage::AutomaticallyAuthenticated
    //    );

    //    // key of contact's endpoint added by encryption implementation
    //    m_trustStorage->addKeys(
    //        ns_omemo,
    //        QStringLiteral("bob@example.com"),
    //        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") },
    //        QXmppTrustStorage::AutomaticallyTrusted
    //    );

    //    // manually authenticated key of other own endpoint
    //    m_manager->authenticateManually(
    //        ns_omemo,
    //        QStringLiteral("alice@example.org"),
    //        { QStringLiteral("09bedfe9cc9b1fe388353c1a9681260cb44d52ade4714c7d844d88b03bbfbb1f") }
    //    );

    //    // manually authenticated stored key of contact's endpoint
    //    m_manager->authenticateManually(
    //        ns_omemo,
    //        QStringLiteral("bob@example.org"),
    //        { QStringLiteral("470c88ff79bd978c208eef4976e1716f930426f04d4437cf7e8d44c219750c42") }
    //    );

    //    // manually authenticated unknown key of contact's endpoint
    //    m_manager->authenticateManually(
    //        ns_omemo,
    //        QStringLiteral("bob@example.org"),
    //        { QStringLiteral("7ea04d96f97542918adab7a644ddb09a2a62d5156b8c372a7680b7de4f15b7d8") }
    //    );

    //    QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
}

void tst_QXmppAtmManager::testHandleMessageReceived()
{
    //    QXmppTrustMessageElement trustMessageElement;
    //    trustMessageElement.setUsage(ns_atm);
    //    trustMessageElement.setEncryption(ns_omemo);

    //    QXmppTrustMessageKeyOwner keyOwnerAlice;
    //    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    //    keyOwnerAlice.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
    //                               QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    //    keyOwnerAlice.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
    //                               QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    //    QXmppTrustMessageKeyOwner keyOwnerBob;
    //    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    //    keyOwnerBob.setTrustedKeys({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
    //                               QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") });
    //    keyOwnerBob.setDistrustedKeys({ QStringLiteral("788a40d0eae5a40409d4687a36d3106bbe361971aec0245598571e7b629edc6b"),
    //                               QStringLiteral("b6c21e111bd4f9ed06ee0485cb302bf1238e90b89986a04061e48d49ddbe95cb") });

    //    auto keyOwners = { keyOwnerAlice, keyOwnerBob };
    //    trustMessageElement.setKeyOwners(keyOwners);

    //    QXmppMessage message;
    //    message.setTo(QStringLiteral("alice@example.org"));
    //    message.setSenderKey(QStringLiteral("69c0087d87ac854f85d80b18e49cd03f7e18ffef381aff03bf204a18d6d9b1d5"));
    //    message.setTrustMessageElement(trustMessageElement);

    //    QObject *context = new QObject(this);
    //    connect(m_manager, &QXmppAtmManager::keysAuthenticated, context, [=](const QList<QString> &keyIds) {

    //        QCOMPARE(
    //            keyIds,
    //            QList({ QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"),
    //                    QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd") })
    //        );

    // Disconnect the lambda expression.
    //        delete context;
    //    });

    //    emit m_client.messageReceived(message);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysDone()
{
    QSignalSpy resultProcessedSpy(this, &tst_QXmppAtmManager::resultProcessed);

    auto future = m_trustStorage->trustLevel(ns_omemo,
                                             QStringLiteral("alice@example.org"),
                                             QStringLiteral("d11715b069372e7a4416caaaced4f30200838155e57dad37a5a4aa24538e58e5"));
    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::Authenticated);
        emit resultProcessed();
    });

    future = m_trustStorage->trustLevel(ns_omemo,
                                        QStringLiteral("alice@example.org"),
                                        QStringLiteral("b589ffc1c20ec414a85b85b551f3ebff381b2e2a412b62ac15f0bb1756f3badd"));
    QXmpp::Private::await(future, [=](const auto &&result) {
        QCOMPARE(result, QXmppTrustStorage::Authenticated);
        emit resultProcessed();
    });

    while (resultProcessedSpy.count() < 2) {
        QVERIFY2(resultProcessedSpy.wait(), "Request timed out!");
    }
}

QTEST_MAIN(tst_QXmppAtmManager)
#include "tst_qxmppatmmanager.moc"
