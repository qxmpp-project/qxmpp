// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAtmManager.h"
#include "QXmppAtmTrustMemoryStorage.h"
#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppMessage.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppTrustMessageKeyOwner.h"
#include "QXmppUtils.h"

#include "util.h"
#include <QObject>
#include <QSet>

using namespace QXmpp;

Q_DECLARE_METATYPE(QList<QXmppTrustMessageKeyOwner>)

// time period (in ms) to wait for a trust message that should not be sent.
constexpr int UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT = 1000;

static const char *ns_atm = "urn:xmpp:atm:1";
static const char *ns_omemo = "eu.siacs.conversations.axolotl";
static const char *ns_ox = "urn:xmpp:openpgp:0";

class tst_QXmppAtmManager : public QObject
{
    Q_OBJECT

public:
    Q_SIGNAL void unexpectedTrustMessageSent();

private:
    Q_SLOT void initTestCase();
    Q_SLOT void testSendTrustMessage();
    Q_SLOT void testMakePostponedTrustDecisions();
    Q_SLOT void testDistrustAutomaticallyTrustedKeys();
    Q_SLOT void testDistrust();
    Q_SLOT void testAuthenticate_data();
    Q_SLOT void testAuthenticate();
    Q_SLOT void testMakeTrustDecisions();
    Q_SLOT void testHandleMessage_data();
    Q_SLOT void testHandleMessage();
    Q_SLOT void testMakeTrustDecisionsNoKeys();
    Q_SLOT void testMakeTrustDecisionsOwnKeys();
    Q_SLOT void testMakeTrustDecisionsOwnKeysNoOwnEndpoints();
    Q_SLOT void testMakeTrustDecisionsOwnKeysNoOwnEndpointsWithAuthenticatedKeys();
    Q_SLOT void testMakeTrustDecisionsOwnKeysNoContactsWithAuthenticatedKeys();
    Q_SLOT void testMakeTrustDecisionsSoleOwnKeyDistrusted();
    Q_SLOT void testMakeTrustDecisionsContactKeys();
    Q_SLOT void testMakeTrustDecisionsContactKeysNoOwnEndpoints();
    Q_SLOT void testMakeTrustDecisionsContactKeysNoOwnEndpointsWithAuthenticatedKeys();
    Q_SLOT void testMakeTrustDecisionsSoleContactKeyDistrusted();

    void testMakeTrustDecisionsOwnKeysDone();
    void testMakeTrustDecisionsContactKeysDone();
    void clearTrustStorage();

    QXmppClient m_client;
    QXmppLogger m_logger;
    QXmppAtmTrustMemoryStorage m_trustStorage;
    QXmppAtmManager m_manager { &m_trustStorage };
    QXmppCarbonManager *m_carbonManager;
};

void tst_QXmppAtmManager::initTestCase()
{
    m_client.addExtension(&m_manager);
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
    keyOwnerAlice.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) });
    keyOwnerAlice.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) });
    keyOwnerBob.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")),
                                    QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) });

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

    m_manager.sendTrustMessage(ns_omemo, { keyOwnerAlice, keyOwnerBob }, QStringLiteral("alice@example.org"));

    QVERIFY(isMessageSent);
}

void tst_QXmppAtmManager::testMakePostponedTrustDecisions()
{
    clearTrustStorage();

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) });
    keyOwnerAlice.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("wzsLdCDtOGUIoLkHAQN3Fdt86GLjE0716F0mnci/pVY=")),
                                                     { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("3bqdCfhQalsOp3LcrFVucCQB4pRRWCyoBTV8KM/oOhY=")) });
    keyOwnerBob.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("baIfLxQhTrtY5JmZvsLaU1MenAU1wwZcI1B7MyWa0Is=")),
                                    QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("cF3Li3ddEJzt9rw/1eAmMS31/G/G4ZTpf+9wbEs51HA=")),
                                                     { keyOwnerBob });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("OxRakAGKtXyuB9sdH3gJDa1XzsV18BAMcVf/m1vD3Xg=")) });
    keyOwnerCarol.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("TKZIGhEMc+gyCgrJEyCnf7OtuoBFhOupOWhdwFhfZBk=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("Zgk0SxGFbeSgDw/Zanza/jzNrr6t1LU0jYX2d7RReKY=")),
                                                     { keyOwnerCarol });

    auto futureVoid = m_manager.makePostponedTrustDecisions(ns_omemo,
                                                            { QByteArray::fromBase64(QByteArrayLiteral("wzsLdCDtOGUIoLkHAQN3Fdt86GLjE0716F0mnci/pVY=")),
                                                              QByteArray::fromBase64(QByteArrayLiteral("cF3Li3ddEJzt9rw/1eAmMS31/G/G4ZTpf+9wbEs51HA=")) });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    auto futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                         { QByteArray::fromBase64(QByteArrayLiteral("wzsLdCDtOGUIoLkHAQN3Fdt86GLjE0716F0mnci/pVY=")),
                                                                           QByteArray::fromBase64(QByteArrayLiteral("cF3Li3ddEJzt9rw/1eAmMS31/G/G4ZTpf+9wbEs51HA=")) });
    QVERIFY(futurePostponed.isFinished());
    auto resultPostponed = futurePostponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QByteArray> trustedKeys = { { QStringLiteral("carol@example.net"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("OxRakAGKtXyuB9sdH3gJDa1XzsV18BAMcVf/m1vD3Xg=")) } };
    QMultiHash<QString, QByteArray> distrustedKeys = { { QStringLiteral("carol@example.net"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("TKZIGhEMc+gyCgrJEyCnf7OtuoBFhOupOWhdwFhfZBk=")) } };

    futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                    { QByteArray::fromBase64(QByteArrayLiteral("Zgk0SxGFbeSgDw/Zanza/jzNrr6t1LU0jYX2d7RReKY=")) });
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

    QMultiHash<QString, QByteArray> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")) },
                                                          { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                                                          { QStringLiteral("bob@example.com"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")) },
                                                          { QStringLiteral("bob@example.com"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("3bqdCfhQalsOp3LcrFVucCQB4pRRWCyoBTV8KM/oOhY=")) } };

    auto future = m_manager.keys(ns_omemo,
                                 TrustLevel::Authenticated);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            TrustLevel::Authenticated,
            authenticatedKeys) }));

    QMultiHash<QString, QByteArray> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")) },
                                                               { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) },
                                                               { QStringLiteral("bob@example.com"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("baIfLxQhTrtY5JmZvsLaU1MenAU1wwZcI1B7MyWa0Is=")) },
                                                               { QStringLiteral("bob@example.com"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) } };

    future = m_manager.keys(ns_omemo,
                            TrustLevel::ManuallyDistrusted);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            TrustLevel::ManuallyDistrusted,
            manuallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testDistrustAutomaticallyTrustedKeys()
{
    clearTrustStorage();

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
        TrustLevel::AutomaticallyTrusted);

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) },
        TrustLevel::Authenticated);

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("dZVdoBINK2n8BkWeTzVg0lVOah4n/9IA/IvQpzUuo1w=")) },
        TrustLevel::AutomaticallyTrusted);

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("We+r1A/kixDad8e383oTmhPDy8g+F5/ircMJmEET8MA=")) },
        TrustLevel::ManuallyTrusted);

    m_manager.distrustAutomaticallyTrustedKeys(ns_omemo,
                                               { QStringLiteral("alice@example.org"),
                                                 QStringLiteral("bob@example.com") });

    QMultiHash<QString, QByteArray> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                                                                    { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
                                                                    { QStringLiteral("bob@example.com"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("dZVdoBINK2n8BkWeTzVg0lVOah4n/9IA/IvQpzUuo1w=")) } };

    auto future = m_manager.keys(ns_omemo,
                                 TrustLevel::AutomaticallyDistrusted);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
            TrustLevel::AutomaticallyDistrusted,
            automaticallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testDistrust()
{
    clearTrustStorage();

    QMultiHash<QString, QByteArray> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                                                          { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        authenticatedKeys.values(),
        TrustLevel::Authenticated);

    QMultiHash<QString, QByteArray> automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                                   QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        automaticallyTrustedKeys.values(),
        TrustLevel::AutomaticallyTrusted);

    QMultiHash<QString, QByteArray> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("6FjJDKcwUxncGka8RvrTGSho+LVDX/7E0+pi5ueqOBQ=")) },
                                                               { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("QfXYzw7lmiD3Qoto6l2kx+HuM1tmKQYW2wCR+u78q8A=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        manuallyDistrustedKeys.values(),
        TrustLevel::ManuallyDistrusted);

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) });
    keyOwnerAlice.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                                     { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")) });
    keyOwnerBob.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")),
                                                     { keyOwnerAlice, keyOwnerBob });

    // The entries for the sender key
    // tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=
    // and the keys of keyOwnerBob remain in the storage.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")),
                                                     { keyOwnerBob });

    auto futureVoid = m_manager.distrust(ns_omemo, {});
    QVERIFY(futureVoid.isFinished());

    auto future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    TrustLevel::Authenticated,
                    authenticatedKeys),
                std::pair(
                    TrustLevel::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    TrustLevel::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));

    futureVoid = m_manager.distrust(ns_omemo,
                                    { std::pair(
                                          QStringLiteral("alice@example.org"),
                                          QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI="))),
                                      std::pair(
                                          QStringLiteral("bob@example.com"),
                                          QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY="))) });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    authenticatedKeys = { { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) } };

    manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("6FjJDKcwUxncGka8RvrTGSho+LVDX/7E0+pi5ueqOBQ=")) },
                               { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("QfXYzw7lmiD3Qoto6l2kx+HuM1tmKQYW2wCR+u78q8A=")) },
                               { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                               { QStringLiteral("bob@example.com"),
                                 QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")) } };

    future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    TrustLevel::Authenticated,
                    authenticatedKeys),
                std::pair(
                    TrustLevel::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));

    auto futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                         { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                                                           QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")) });
    QVERIFY(futurePostponed.isFinished());
    auto resultPostponed = futurePostponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QByteArray> trustedKeys = { { QStringLiteral("bob@example.com"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")) } };
    QMultiHash<QString, QByteArray> distrustedKeys = { { QStringLiteral("bob@example.com"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) } };

    futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                    { QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) });
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
    QTest::addColumn<TrustSecurityPolicy>("securityPolicy");

    QTest::newRow("noSecurityPolicy")
        << NoSecurityPolicy;

    QTest::newRow("toakafa")
        << Toakafa;
}

void tst_QXmppAtmManager::testAuthenticate()
{
    clearTrustStorage();

    QFETCH(TrustSecurityPolicy, securityPolicy);
    m_manager.setSecurityPolicy(ns_omemo, securityPolicy);

    QMultiHash<QString, QByteArray> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("rQIL2albuSR1i06EZAp1uZ838zUeEgGIq2whwu3s+Zg=")) },
                                                          { QStringLiteral("carol@example.net"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("+CQZlFyxdeTGgbPby7YvvZT3YIVcIi+1E8N5nSc6QTA=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        authenticatedKeys.values(QStringLiteral("alice@example.org")),
        TrustLevel::Authenticated);

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        authenticatedKeys.values(QStringLiteral("carol@example.net")),
        TrustLevel::Authenticated);

    QMultiHash<QString, QByteArray> automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                                                   QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")) },
                                                                 { QStringLiteral("bob@example.com"),
                                                                   QByteArray::fromBase64(QByteArrayLiteral("/dqv0+RNyFIPdMQiJ7mSEJWKVExFeUBEvTXxOtqIMDg=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        automaticallyTrustedKeys.values(),
        TrustLevel::AutomaticallyTrusted);

    QMultiHash<QString, QByteArray> manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("6FjJDKcwUxncGka8RvrTGSho+LVDX/7E0+pi5ueqOBQ=")) },
                                                               { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("QfXYzw7lmiD3Qoto6l2kx+HuM1tmKQYW2wCR+u78q8A=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        manuallyDistrustedKeys.values(),
        TrustLevel::ManuallyDistrusted);

    QMultiHash<QString, QByteArray> automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                                                                    { QStringLiteral("alice@example.org"),
                                                                      QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) } };

    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        automaticallyDistrustedKeys.values(),
        TrustLevel::AutomaticallyDistrusted);

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) });
    keyOwnerAlice.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")),
                                      QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                                     { keyOwnerAlice });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")) });
    keyOwnerBob.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) });

    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")),
                                                     { keyOwnerAlice, keyOwnerBob });

    QXmppTrustMessageKeyOwner keyOwnerCarol;
    keyOwnerCarol.setJid(QStringLiteral("carol@example.net"));
    keyOwnerCarol.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("ikwzympBsVXz3AxqofZKWSPswNJIGiLGD1ItfGBQmHE=")) });
    keyOwnerCarol.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("+CQZlFyxdeTGgbPby7YvvZT3YIVcIi+1E8N5nSc6QTA=")) });

    // The keys of keyOwnerCarol are used for trust decisions once Bob's key
    // z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw= is
    // authenticated by the authentication of key
    // mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")),
                                                     { keyOwnerCarol });

    // The entries for the sender key
    // LpzzOVOECo4N3P4B7CxYl7DBhCHBbtOBNa4FHOK+pD4=
    // and the keys of keyOwnerCarol are removed from the storage
    // because they are already used for trust decisions once Bob's key
    // z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw= is
    // authenticated.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("LpzzOVOECo4N3P4B7CxYl7DBhCHBbtOBNa4FHOK+pD4=")),
                                                     { keyOwnerCarol });

    keyOwnerCarol.setTrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("s/fRdN1iurUbZUHGdnIC7l7nllzv6ArLuwsK1GcgI58=")) });
    keyOwnerCarol.setDistrustedKeys({ QByteArray::fromBase64(QByteArrayLiteral("9D5EokNlchfgWRkfd7L+cpvkcTCCqwf5sKwcx0HfHbs=")) });

    // The entries for the sender key
    // KXVnPIqbak7+7XZ+58dkPoe6w3cN/GyjKj8IdJtcbt8=
    // and the keys of keyOwnerCarol remain in the storage.
    m_trustStorage.addKeysForPostponedTrustDecisions(ns_omemo,
                                                     QByteArray::fromBase64(QByteArrayLiteral("KXVnPIqbak7+7XZ+58dkPoe6w3cN/GyjKj8IdJtcbt8=")),
                                                     { keyOwnerCarol });

    auto futureVoid = m_manager.authenticate(ns_omemo, {});
    QVERIFY(futureVoid.isFinished());

    auto future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    TrustLevel::Authenticated,
                    authenticatedKeys),
                std::pair(
                    TrustLevel::AutomaticallyTrusted,
                    automaticallyTrustedKeys),
                std::pair(
                    TrustLevel::ManuallyDistrusted,
                    manuallyDistrustedKeys),
                std::pair(
                    TrustLevel::AutomaticallyDistrusted,
                    automaticallyDistrustedKeys) }));

    futureVoid = m_manager.authenticate(ns_omemo,
                                        { std::pair(
                                              QStringLiteral("alice@example.org"),
                                              QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI="))),
                                          std::pair(
                                              QStringLiteral("bob@example.com"),
                                              QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY="))) });
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    authenticatedKeys = { { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("rQIL2albuSR1i06EZAp1uZ838zUeEgGIq2whwu3s+Zg=")) },
                          { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                          { QStringLiteral("bob@example.com"),
                            QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")) },
                          { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")) },
                          { QStringLiteral("alice@example.org"),
                            QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                          { QStringLiteral("bob@example.com"),
                            QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")) },
                          { QStringLiteral("carol@example.net"),
                            QByteArray::fromBase64(QByteArrayLiteral("ikwzympBsVXz3AxqofZKWSPswNJIGiLGD1ItfGBQmHE=")) } };

    manuallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("6FjJDKcwUxncGka8RvrTGSho+LVDX/7E0+pi5ueqOBQ=")) },
                               { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("QfXYzw7lmiD3Qoto6l2kx+HuM1tmKQYW2wCR+u78q8A=")) },
                               { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("eIpA0OrlpAQJ1Gh6NtMQa742GXGuwCRVmFcee2Ke3Gs=")) },
                               { QStringLiteral("alice@example.org"),
                                 QByteArray::fromBase64(QByteArrayLiteral("tsIeERvU+e0G7gSFyzAr8SOOkLiZhqBAYeSNSd2+lcs=")) },
                               { QStringLiteral("bob@example.com"),
                                 QByteArray::fromBase64(QByteArrayLiteral("U3+UnkTp12gusKbzWwN0lqDLEPb2CdMxP4bY85q9pxA=")) },
                               { QStringLiteral("carol@example.net"),
                                 QByteArray::fromBase64(QByteArrayLiteral("+CQZlFyxdeTGgbPby7YvvZT3YIVcIi+1E8N5nSc6QTA=")) } };

    if (securityPolicy == NoSecurityPolicy) {
        automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) } };

        automaticallyTrustedKeys = { { QStringLiteral("bob@example.com"),
                                       QByteArray::fromBase64(QByteArrayLiteral("/dqv0+RNyFIPdMQiJ7mSEJWKVExFeUBEvTXxOtqIMDg=")) } };
    } else if (securityPolicy == Toakafa) {
        automaticallyDistrustedKeys = { { QStringLiteral("alice@example.org"),
                                          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
                                        { QStringLiteral("bob@example.com"),
                                          QByteArray::fromBase64(QByteArrayLiteral("/dqv0+RNyFIPdMQiJ7mSEJWKVExFeUBEvTXxOtqIMDg=")) } };
    }

    future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    result = future.result();
    switch (securityPolicy) {
    case NoSecurityPolicy:
        QCOMPARE(
            result,
            QHash({ std::pair(
                        TrustLevel::Authenticated,
                        authenticatedKeys),
                    std::pair(
                        TrustLevel::AutomaticallyTrusted,
                        automaticallyTrustedKeys),
                    std::pair(
                        TrustLevel::ManuallyDistrusted,
                        manuallyDistrustedKeys),
                    std::pair(
                        TrustLevel::AutomaticallyDistrusted,
                        automaticallyDistrustedKeys) }));
        break;
    case Toakafa:
        QCOMPARE(
            result,
            QHash({ std::pair(
                        TrustLevel::Authenticated,
                        authenticatedKeys),
                    std::pair(
                        TrustLevel::ManuallyDistrusted,
                        manuallyDistrustedKeys),
                    std::pair(
                        TrustLevel::AutomaticallyDistrusted,
                        automaticallyDistrustedKeys) }));
        break;
    }

    auto futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                         { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                                                           QByteArray::fromBase64(QByteArrayLiteral("mwT0Hwr7aG1p+x0q60H0UDSEnr8cr7hxvxDEhFGrLmY=")),
                                                                           QByteArray::fromBase64(QByteArrayLiteral("z6MVV3MHGCZkKgapng8hQHCh57iZmlcQogmTmsy3/Kw=")),
                                                                           QByteArray::fromBase64(QByteArrayLiteral("LpzzOVOECo4N3P4B7CxYl7DBhCHBbtOBNa4FHOK+pD4=")) });
    QVERIFY(futurePostponed.isFinished());
    auto resultPostponed = futurePostponed.result();
    QVERIFY(resultPostponed.isEmpty());

    QMultiHash<QString, QByteArray> trustedKeys = { { QStringLiteral("carol@example.net"),
                                                      QByteArray::fromBase64(QByteArrayLiteral("s/fRdN1iurUbZUHGdnIC7l7nllzv6ArLuwsK1GcgI58=")) } };
    QMultiHash<QString, QByteArray> distrustedKeys = { { QStringLiteral("carol@example.net"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("9D5EokNlchfgWRkfd7L+cpvkcTCCqwf5sKwcx0HfHbs=")) } };

    futurePostponed = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
                                                                    { QByteArray::fromBase64(QByteArrayLiteral("KXVnPIqbak7+7XZ+58dkPoe6w3cN/GyjKj8IdJtcbt8=")) });
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

    QMultiHash<QString, QByteArray> keysBeingAuthenticated = { { QStringLiteral("alice@example.org"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("b4XbD7VaiMNyHfb2cq7PLGTaW3iAM75iXQpLkcr3r0M=")) },
                                                               { QStringLiteral("bob@example.com"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("PJz644fYarsYEO1ECZhpqmrtboAB4lqNgSjhQik0jSM=")) } };
    QMultiHash<QString, QByteArray> keysBeingDistrusted = { { QStringLiteral("alice@example.org"),
                                                              QByteArray::fromBase64(QByteArrayLiteral("Pw4KZ2uLdEVuGTWaeSbwZsSstBzN2+prK0GDeD8HyKA=")) },
                                                            { QStringLiteral("bob@example.com"),
                                                              QByteArray::fromBase64(QByteArrayLiteral("Pw4KZ2uLdEVuGTWaeSbwZsSstBzN2+prK0GDeD8HyKA=")) } };

    auto futureVoid = m_manager.makeTrustDecisions(ns_omemo,
                                                   keysBeingAuthenticated,
                                                   keysBeingDistrusted);
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    auto future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    TrustLevel::Authenticated,
                    keysBeingAuthenticated),
                std::pair(
                    TrustLevel::ManuallyDistrusted,
                    keysBeingDistrusted) }));
}

void tst_QXmppAtmManager::testHandleMessage_data()
{
    QTest::addColumn<QXmppMessage>("message");
    QTest::addColumn<bool>("areTrustDecisionsValid");
    QTest::addColumn<bool>("isSenderKeyAuthenticated");

    QXmppTrustMessageKeyOwner keyOwnerAlice;
    keyOwnerAlice.setJid(QStringLiteral("alice@example.org"));
    keyOwnerAlice.setTrustedKeys({ { QByteArray::fromBase64(QByteArrayLiteral("YHiLgLpE3dvoy4MayxycR+BABFY9w6D/rKZjUnu2jSY=")) },
                                   { QByteArray::fromBase64(QByteArrayLiteral("Ocp5ah/API6Ph83N3fFJZqObX7Rywg++D4EowImgFrw=")) } });
    keyOwnerAlice.setDistrustedKeys({ { QByteArray::fromBase64(QByteArrayLiteral("0PO+OhpTQkuM3Fd/CuhdWVuRZzYoUfQzOUvpcCIvKZQ=")) },
                                      { QByteArray::fromBase64(QByteArrayLiteral("fkcPYIctqF+bzuvkd6dVMv8z0EpFoA7sEuUNe/lvEx4=")) } });

    QXmppTrustMessageKeyOwner keyOwnerBob;
    keyOwnerBob.setJid(QStringLiteral("bob@example.com"));
    keyOwnerBob.setTrustedKeys({ { QByteArray::fromBase64(QByteArrayLiteral("nKT6zqFRNDq6GpWQIV/CwbA65fqN9Bo4qVxMfFjwl1w=")) },
                                 { QByteArray::fromBase64(QByteArrayLiteral("E4z5Qz9cWDt49j8JXxjSHGlQ9Xx6YESBX7ukfet2LhY=")) } });
    keyOwnerBob.setDistrustedKeys({ { QByteArray::fromBase64(QByteArrayLiteral("b3EsvoNBgUpiQD9KRHmosP/rR7T+3BA84MQw4N6eZmU=")) },
                                    { QByteArray::fromBase64(QByteArrayLiteral("guRlZo0QVxX3TbzdhyOwzdlorG0Znndo/P9NsWtMkk4=")) } });

    QXmppE2eeMetadata e2eeMetadata;
    e2eeMetadata.setSenderKey(QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")));

    QList<QXmppTrustMessageKeyOwner> keyOwners;
    keyOwners << keyOwnerAlice << keyOwnerBob;

    QXmppTrustMessageElement trustMessageElement;
    trustMessageElement.setUsage(ns_atm);
    trustMessageElement.setEncryption(ns_omemo);
    trustMessageElement.setKeyOwners(keyOwners);

    QXmppMessage message;
    message.setFrom(m_client.configuration().jid());
    message.setE2eeMetadata(e2eeMetadata);
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

    e2eeMetadata.setSenderKey(QByteArray::fromBase64(QByteArrayLiteral("qfNJsEMZ8jru0dS76DtYaTxZjiVQ5lpJWBiyaUj9UGU=")));
    message.setFrom(QStringLiteral("bob@example.com/notebook"));
    message.setE2eeMetadata(e2eeMetadata);

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
    const auto senderKey = message.e2eeMetadata()->senderKey();

    // Add the sender key in preparation for the test.
    if (areTrustDecisionsValid) {
        if (isSenderKeyAuthenticated) {
            m_manager.addKeys(ns_omemo,
                              senderJid,
                              { senderKey },
                              TrustLevel::Authenticated);
        } else {
            m_manager.addKeys(ns_omemo,
                              senderJid,
                              { senderKey });
        }
    }

    auto future = m_manager.handleMessage(message);
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    // Remove the sender key as soon as the method being tested is executed.
    if (areTrustDecisionsValid) {
        m_manager.removeKeys(ns_omemo, QList { senderKey });
    }

    if (areTrustDecisionsValid) {
        const auto isOwnMessage = senderJid == m_client.configuration().jidBare();
        const auto keyOwners = message.trustMessageElement()->keyOwners();

        if (isSenderKeyAuthenticated) {
            QMultiHash<QString, QByteArray> authenticatedKeys;
            QMultiHash<QString, QByteArray> manuallyDistrustedKeys;

            if (isOwnMessage) {
                for (const auto &keyOwner : keyOwners) {
                    for (const auto &trustedKey : keyOwner.trustedKeys()) {
                        authenticatedKeys.insert(keyOwner.jid(), trustedKey);
                    }

                    for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                        manuallyDistrustedKeys.insert(keyOwner.jid(), distrustedKey);
                    }
                }

                auto future = m_manager.keys(ns_omemo);
                QVERIFY(future.isFinished());
                auto result = future.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                TrustLevel::Authenticated,
                                authenticatedKeys),
                            std::pair(
                                TrustLevel::ManuallyDistrusted,
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

                auto future = m_manager.keys(ns_omemo);
                QVERIFY(future.isFinished());
                auto result = future.result();
                QCOMPARE(
                    result,
                    QHash({ std::pair(
                                TrustLevel::Authenticated,
                                authenticatedKeys),
                            std::pair(
                                TrustLevel::ManuallyDistrusted,
                                manuallyDistrustedKeys) }));
            }
        } else {
            if (isOwnMessage) {
                QMultiHash<QString, QByteArray> trustedKeys;
                QMultiHash<QString, QByteArray> distrustedKeys;

                for (const auto &keyOwner : keyOwners) {
                    for (const auto &trustedKey : keyOwner.trustedKeys()) {
                        trustedKeys.insert(keyOwner.jid(), trustedKey);
                    }

                    for (const auto &distrustedKey : keyOwner.distrustedKeys()) {
                        distrustedKeys.insert(keyOwner.jid(), distrustedKey);
                    }
                }

                auto future = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
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
                QMultiHash<QString, QByteArray> trustedKeys;
                QMultiHash<QString, QByteArray> distrustedKeys;

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

                auto futureHash = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo,
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
        auto futureHash = m_manager.keys(ns_omemo);
        QVERIFY(futureHash.isFinished());
        auto resultHash = futureHash.result();
        QVERIFY(resultHash.isEmpty());

        auto futureHash2 = m_trustStorage.keysForPostponedTrustDecisions(ns_omemo);
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) },
        TrustLevel::ManuallyDistrusted);

    const QObject context;

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &) {
        if (type == QXmppLogger::SentMessage) {
            Q_EMIT unexpectedTrustMessageSent();
        }
    });

    auto futureVoid = m_manager.makeTrustDecisions(ns_omemo,
                                                   QStringLiteral("alice@example.org"),
                                                   {},
                                                   {});
    while (!futureVoid.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    QMultiHash<QString, QByteArray> authenticatedKeys = { { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
                                                          { QStringLiteral("alice@example.org"),
                                                            QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) } };

    QMultiHash<QString, QByteArray> manuallyDistrustedKeys = { { QStringLiteral("bob@example.com"),
                                                                 QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) } };

    auto future = m_manager.keys(ns_omemo);
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(
        result,
        QHash({ std::pair(
                    TrustLevel::Authenticated,
                    authenticatedKeys),
                std::pair(
                    TrustLevel::ManuallyDistrusted,
                    manuallyDistrustedKeys) }));
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeys()
{
    clearTrustStorage();

    // keys of own endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
        TrustLevel::Authenticated);
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) },
        TrustLevel::ManuallyDistrusted);

    // keys of contact's endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) },
        TrustLevel::ManuallyDistrusted);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                         QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) }));
                        QCOMPARE(keyOwner.distrustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) }));
                    } else if (keyOwnerJid == QStringLiteral("bob@example.com")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) }));
                        QCOMPARE(keyOwner.distrustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) }));
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("alice@example.org"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("alice@example.org"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) },
        TrustLevel::ManuallyDistrusted);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) }));
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
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) }));
                    } else if (keyOwnerJid == QStringLiteral("bob@example.com")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else if (keyOwnerJid == QStringLiteral("carol@example.net")) {
                        QCOMPARE(keyOwner.trustedKeys(),
                                 QList({ QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) }));
                        QVERIFY(keyOwner.distrustedKeys().isEmpty());
                    } else {
                        QFAIL("Unexpected key owner sent!");
                    }
                }
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("alice@example.org"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
        TrustLevel::Authenticated);

    // keys of contact's endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) },
        TrustLevel::AutomaticallyDistrusted);

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

                if (keyOwner.trustedKeys() == QList({ QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")), QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) }) &&
                    keyOwner.distrustedKeys() == QList({ QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) })) {
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
                if (trustedKeys == QList({ QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")), QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) })) {
                    sentMessagesCount++;

                    QVERIFY(keyOwner.distrustedKeys().isEmpty());
                }
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("alice@example.org"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) }));
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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) }));
            }
        }
    });

    // unexpected trust message for contacts' keys to own endpoint
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("alice@example.org")) {
                Q_EMIT unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("alice@example.org"),
                                               {},
                                               { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 2);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    auto futureTrustLevel = m_manager.trustLevel(ns_omemo,
                                                 QStringLiteral("alice@example.org"),
                                                 QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")));
    QVERIFY(futureTrustLevel.isFinished());
    auto result = futureTrustLevel.result();
    QCOMPARE(result, TrustLevel::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeys()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // keys of own endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
          QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) },
        TrustLevel::Authenticated);
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) },
        TrustLevel::ManuallyDistrusted);

    // keys of contact's endpoints
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")),
          QByteArray::fromBase64(QByteArrayLiteral("T+dplAB8tGSdbYBbRiOm/jrS+8CPuzGHrH8ZmbjyvPo=")) },
        TrustLevel::Authenticated);
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) },
        TrustLevel::ManuallyDistrusted);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("mzDeKTQBVm1cTmzF9DjCGKa14pDADZOVLT9Kh7CK7AM=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("GHzmri+1U53eFRglbQhoXgU8vOpnXZ012Vg90HiLvWw=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("T+dplAB8tGSdbYBbRiOm/jrS+8CPuzGHrH8ZmbjyvPo=")) }));
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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")),
                                 QByteArray::fromBase64(QByteArrayLiteral("tfskruc1xcfC+VKzuqvLZUJVZccZX/Pg5j88ukpuY2M=")) }));
                QCOMPARE(keyOwner.distrustedKeys(),
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) }));
            }
        }
    });

    // unexpected trust message to Carol
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == QStringLiteral("carol@example.net")) {
                Q_EMIT unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("bob@example.com"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("mzDeKTQBVm1cTmzF9DjCGKa14pDADZOVLT9Kh7CK7AM=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("GHzmri+1U53eFRglbQhoXgU8vOpnXZ012Vg90HiLvWw=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("T+dplAB8tGSdbYBbRiOm/jrS+8CPuzGHrH8ZmbjyvPo=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

    const QObject context;

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            Q_EMIT unexpectedTrustMessageSent();
        }
    });

    m_manager.makeTrustDecisions(ns_omemo,
                                 QStringLiteral("bob@example.com"),
                                 { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")),
                                   QByteArray::fromBase64(QByteArrayLiteral("mzDeKTQBVm1cTmzF9DjCGKa14pDADZOVLT9Kh7CK7AM=")) },
                                 { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) });

    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    testMakeTrustDecisionsContactKeysDone();
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeysNoOwnEndpointsWithAuthenticatedKeys()
{
    clearTrustStorage();

    QSignalSpy unexpectedTrustMessageSentSpy(this, &tst_QXmppAtmManager::unexpectedTrustMessageSent);

    // key of own endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) },
        TrustLevel::ManuallyDistrusted);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("GaHysNhcfDSzG2q6OAThRGUpuFB9E7iCRR/1mK1TL+Q=")) }));
            }
        }
    });

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() != QStringLiteral("bob@example.com")) {
                Q_EMIT unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("bob@example.com"),
                                               { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")),
                                                 QByteArray::fromBase64(QByteArrayLiteral("mzDeKTQBVm1cTmzF9DjCGKa14pDADZOVLT9Kh7CK7AM=")) },
                                               { QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")) });
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
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("alice@example.org"),
        { QByteArray::fromBase64(QByteArrayLiteral("RwyI/3m9l4wgju9JduFxb5MEJvBNRDfPfo1Ewhl1DEI=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("bob@example.com"),
        { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) },
        TrustLevel::Authenticated);

    // key of contact's endpoint
    m_manager.addKeys(
        ns_omemo,
        QStringLiteral("carol@example.net"),
        { QByteArray::fromBase64(QByteArrayLiteral("tVy3ygBnW4q6V2TYe8p4i904zD+x4rNMRegxPnPI7fw=")) },
        TrustLevel::Authenticated);

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
                         QList({ QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) }));
            }
        }
    });

    // unexpected trust message
    connect(&m_logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() != QStringLiteral("alice@example.org")) {
                Q_EMIT unexpectedTrustMessageSent();
            }
        }
    });

    auto future = m_manager.makeTrustDecisions(ns_omemo,
                                               QStringLiteral("bob@example.com"),
                                               {},
                                               { QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")) });
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QCOMPARE(sentMessagesCount, 1);
    QVERIFY2(!unexpectedTrustMessageSentSpy.wait(UNEXPECTED_TRUST_MESSAGE_WAITING_TIMEOUT), "Unexpected trust message sent!");

    const auto futureTrustLevel = m_manager.trustLevel(ns_omemo,
                                                       QStringLiteral("bob@example.com"),
                                                       QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")));
    QVERIFY(futureTrustLevel.isFinished());
    const auto result = futureTrustLevel.result();
    QCOMPARE(result, TrustLevel::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsOwnKeysDone()
{
    auto future = m_manager.trustLevel(ns_omemo,
                                       QStringLiteral("alice@example.org"),
                                       QByteArray::fromBase64(QByteArrayLiteral("0RcVsGk3LnpEFsqqztTzAgCDgVXlfa03paSqJFOOWOU=")));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, TrustLevel::Authenticated);

    future = m_manager.trustLevel(ns_omemo,
                                  QStringLiteral("alice@example.org"),
                                  QByteArray::fromBase64(QByteArrayLiteral("tYn/wcIOxBSoW4W1UfPr/zgbLipBK2KsFfC7F1bzut0=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, TrustLevel::Authenticated);

    future = m_manager.trustLevel(ns_omemo,
                                  QStringLiteral("alice@example.org"),
                                  QByteArray::fromBase64(QByteArrayLiteral("4iBsyJPVAfNWM/OgyA9fasOvkJ8K1/0wuYpwVGw4Q5M=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, TrustLevel::ManuallyDistrusted);
}

void tst_QXmppAtmManager::testMakeTrustDecisionsContactKeysDone()
{
    auto future = m_manager.trustLevel(ns_omemo,
                                       QStringLiteral("bob@example.com"),
                                       QByteArray::fromBase64(QByteArrayLiteral("+1VJvMLCGvkDquZ6mQZ+SS+gTbQ436BJUwFOoW0Ma1g=")));
    QVERIFY(future.isFinished());
    auto result = future.result();
    QCOMPARE(result, TrustLevel::Authenticated);

    future = m_manager.trustLevel(ns_omemo,
                                  QStringLiteral("bob@example.com"),
                                  QByteArray::fromBase64(QByteArrayLiteral("mzDeKTQBVm1cTmzF9DjCGKa14pDADZOVLT9Kh7CK7AM=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, TrustLevel::Authenticated);

    future = m_manager.trustLevel(ns_omemo,
                                  QStringLiteral("bob@example.com"),
                                  QByteArray::fromBase64(QByteArrayLiteral("8gBTC1fspYkO4akS6QKN+XFA9Nmf9NEIg7hjtlpTjII=")));
    QVERIFY(future.isFinished());
    result = future.result();
    QCOMPARE(result, TrustLevel::ManuallyDistrusted);
}

void tst_QXmppAtmManager::clearTrustStorage()
{
    m_manager.removeKeys(ns_omemo);
    m_trustStorage.removeKeysForPostponedTrustDecisions(ns_omemo);
}

QTEST_MAIN(tst_QXmppAtmManager)
#include "tst_qxmppatmmanager.moc"
