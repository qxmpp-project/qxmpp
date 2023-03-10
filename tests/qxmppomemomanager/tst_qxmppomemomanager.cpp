// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAtmManager.h"
#include "QXmppAtmTrustMemoryStorage.h"
#include "QXmppBitsOfBinaryContentId.h"
#include "QXmppBitsOfBinaryIq.h"
#include "QXmppCarbonManagerV2.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppMessage.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoManager.h"
#include "QXmppOmemoManager_p.h"
#include "QXmppOmemoMemoryStorage.h"
#include "QXmppPubSubManager.h"

#include "IntegrationTesting.h"
#include "util.h"
#include <QObject>

using namespace QXmpp;
using namespace QXmpp::Private;

struct OmemoUser
{
    QXmppClient client;
    QXmppLogger logger;
    QXmppOmemoManager *manager;
    QXmppCarbonManagerV2 *carbonManager;
    QXmppDiscoveryManager *discoveryManager;
    QXmppPubSubManager *pubSubManager;
    std::unique_ptr<QXmppOmemoMemoryStorage> omemoStorage;
    std::unique_ptr<QXmppAtmTrustStorage> trustStorage;
    QXmppAtmManager *trustManager;
};

class OmemoIqHandler : public QXmppClientExtension
{
public:
    OmemoIqHandler(const QXmppBitsOfBinaryIq &requestIq, const QXmppBitsOfBinaryIq &responseIq)
    {
        m_requestIq = requestIq;
        m_responseIq = responseIq;
    }

    bool handleStanza(const QDomElement &stanza, const std::optional<QXmppE2eeMetadata> &e2eeMetadata) override
    {
        if (stanza.tagName() == "iq" && QXmppBitsOfBinaryIq::isBitsOfBinaryIq(stanza)) {
            QXmppBitsOfBinaryIq iq;
            iq.parse(stanza);

            if (iq.cid().toContentId() != m_requestIq.cid().toContentId()) {
                return false;
            }

            m_responseIq.setId(iq.id());
            client()->reply(std::move(m_responseIq), e2eeMetadata);
            return true;
        }

        return false;
    };

private:
    QXmppBitsOfBinaryIq m_requestIq;
    QXmppBitsOfBinaryIq m_responseIq;
};

class tst_QXmppOmemoManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();
    Q_SLOT void testSecurityPolicies();
    Q_SLOT void testTrustLevels();
    Q_SLOT void initOmemoUser(OmemoUser &omemoUser);
    Q_SLOT void testInit();
    Q_SLOT void testSetUp();
    Q_SLOT void testLoad();
    Q_SLOT void testSendMessage();
    Q_SLOT void testSendIq();
    Q_SLOT void finish(OmemoUser &omemoUser);

private:
    OmemoUser m_alice1;
    OmemoUser m_alice2;
};

void tst_QXmppOmemoManager::initTestCase()
{
    initOmemoUser(m_alice1);
    initOmemoUser(m_alice2);
}

void tst_QXmppOmemoManager::testSecurityPolicies()
{
    auto futureSecurityPolicy = m_alice1.manager->securityPolicy();
    QVERIFY(futureSecurityPolicy.isFinished());
    auto resultSecurityPolicy = futureSecurityPolicy.result();
    QCOMPARE(resultSecurityPolicy, NoSecurityPolicy);

    m_alice1.manager->setSecurityPolicy(Toakafa);

    futureSecurityPolicy = m_alice1.manager->securityPolicy();
    QVERIFY(futureSecurityPolicy.isFinished());
    resultSecurityPolicy = futureSecurityPolicy.result();
    QCOMPARE(resultSecurityPolicy, Toakafa);
}

void tst_QXmppOmemoManager::testTrustLevels()
{
    auto futureTrustLevel = m_alice1.manager->trustLevel(QStringLiteral("alice@example.org"),
                                                         QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")));
    QVERIFY(futureTrustLevel.isFinished());
    auto resultTrustLevel = futureTrustLevel.result();
    QCOMPARE(resultTrustLevel, TrustLevel::Undecided);

    m_alice1.manager->setTrustLevel(
        { { QStringLiteral("alice@example.org"),
            QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")) },
          { QStringLiteral("bob@example.com"),
            QByteArray::fromBase64(QByteArrayLiteral("9E51lG3vVmUn8CM7/AIcmIlLP2HPl6Ao0/VSf4VT/oA=")) } },
        TrustLevel::Authenticated);

    futureTrustLevel = m_alice1.manager->trustLevel(QStringLiteral("alice@example.org"),
                                                    QByteArray::fromBase64(QByteArrayLiteral("AZ/cF4OrUOILKO1gQBf62pQevOhBJ2NyHnXLwM4FDZU=")));
    QVERIFY(futureTrustLevel.isFinished());
    resultTrustLevel = futureTrustLevel.result();
    QCOMPARE(resultTrustLevel, TrustLevel::Authenticated);
}

void tst_QXmppOmemoManager::initOmemoUser(OmemoUser &omemoUser)
{
    omemoUser.discoveryManager = new QXmppDiscoveryManager;
    omemoUser.client.addExtension(omemoUser.discoveryManager);

    omemoUser.pubSubManager = new QXmppPubSubManager;
    omemoUser.client.addExtension(omemoUser.pubSubManager);

    omemoUser.trustStorage = std::make_unique<QXmppAtmTrustMemoryStorage>();
    omemoUser.trustManager = new QXmppAtmManager(omemoUser.trustStorage.get());
    omemoUser.client.addExtension(omemoUser.trustManager);

    omemoUser.omemoStorage = std::make_unique<QXmppOmemoMemoryStorage>();
    omemoUser.manager = new QXmppOmemoManager(omemoUser.omemoStorage.get());
    omemoUser.client.addExtension(omemoUser.manager);

    omemoUser.carbonManager = new QXmppCarbonManagerV2;
    omemoUser.client.addExtension(omemoUser.carbonManager);

    omemoUser.logger.setLoggingType(QXmppLogger::SignalLogging);
    omemoUser.client.setLogger(&omemoUser.logger);
}

void tst_QXmppOmemoManager::testInit()
{
    auto omemoStorage = std::make_unique<QXmppOmemoMemoryStorage>();
    auto manager = std::make_unique<QXmppOmemoManager>(omemoStorage.get());
    QVERIFY(manager->d->initGlobalContext());
    QVERIFY(manager->d->initLocking());
    QVERIFY(manager->d->initCryptoProvider());
    // TODO: Test initStores()
}

void tst_QXmppOmemoManager::testSetUp()
{
    SKIP_IF_INTEGRATION_TESTS_DISABLED();

    auto isManagerSetUp = false;
    const QObject context;

    connect(&m_alice1.client, &QXmppClient::connected, &context, [=, &isManagerSetUp]() {
        auto future = m_alice1.manager->setUp();
        future.then(this, [=, &isManagerSetUp](bool isSetUp) {
            if (isSetUp) {
                isManagerSetUp = true;
            }
        });
    });

    connect(&m_alice1.logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            qDebug() << "SENT: " << text;
        } else {
            qDebug() << "RECEIVED: " << text;
        }
    });

    m_alice1.client.connectToServer(IntegrationTests::clientConfiguration());

    QTRY_VERIFY(isManagerSetUp);
    finish(m_alice1);
}

void tst_QXmppOmemoManager::testLoad()
{
    auto future = m_alice1.manager->load();
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }
    auto result = future.result();
    QVERIFY(!result);

    const QXmppOmemoStorage::OwnDevice ownDevice = { 1,
                                                     QStringLiteral("notebook"),
                                                     QByteArray::fromBase64(QByteArrayLiteral("OU5HM3loYnFjZVVaYmpSbHdab0FPTDhJVHRzUFVUcFMK")),
                                                     QByteArray::fromBase64(QByteArrayLiteral("TkhodEZ6cnFDeGtENWRuT1ZZdUsyaGIwQkRPdHFRSE8K")),
                                                     2,
                                                     3 };
    m_alice1.omemoStorage->setOwnDevice(ownDevice);
    m_alice1.omemoStorage->addSignedPreKeyPair(2,
                                               { QDateTime::currentDateTimeUtc(),
                                                 QByteArray::fromBase64(QByteArrayLiteral("VEZBOTZFRjNQSVRzVE1OcnIzYmV2ZFFuM0R3WmduUWwK")) });
    m_alice1.omemoStorage->addPreKeyPairs({ { 3,
                                              QByteArray::fromBase64(QByteArrayLiteral("RmVmQ0RTTzB0Z2R2T0ZjckQ4N29PN01VTGFFMVZjUmIK")) } });

    future = m_alice1.manager->load();
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }
    result = future.result();
    QVERIFY(result);

    const auto storedOwnDevice = m_alice1.manager->ownDevice();
    //    QCOMPARE(storedOwnDevice.keyId(), ownDevice.publicIdentityKey);
    QCOMPARE(storedOwnDevice.label(), ownDevice.label);

    m_alice1.omemoStorage->resetAll();
}

void tst_QXmppOmemoManager::testSendMessage()
{
    SKIP_IF_INTEGRATION_TESTS_DISABLED()

    QSignalSpy disconnectedAlice1Spy(&m_alice1.client, &QXmppClient::disconnected);

    auto isFirstMessageSentByAlice1 = false;
    auto isFirstMessageDecryptedByAlice2 = false;
    auto isEmptyOmemoMessageReceivedByAlice1 = false;
    auto isSecondMessageSentByAlice1 = false;
    auto isSecondMessageDecryptedByAlice2 = false;

    const auto config1 = IntegrationTests::clientConfiguration();
    auto config2 = config1;
    config2.setResource(config2.resource() % QStringLiteral("2"));

    const QObject context;
    QString recipient = "bob@" % config1.domain();

    QXmppMessage message1;
    message1.setTo(recipient);
    message1.setBody("Hello Bob!");

    QXmppMessage message2;
    message2.setTo(recipient);
    message2.setBody("Hello Bob again!");

    connect(&m_alice1.client, &QXmppClient::connected, &context, [=]() {
        auto future = m_alice1.manager->setUp();
        future.then(this, [=](bool isSetUp) {
            if (isSetUp) {
                auto future = m_alice1.manager->setSecurityPolicy(Toakafa);
                future.then(this, [=]() {
                    auto future = m_alice2.manager->setSecurityPolicy(Toakafa);
                    future.then(this, [=]() {
                        m_alice2.client.connectToServer(config2);
                    });
                });
            }
        });
    });

    connect(&m_alice2.client, &QXmppClient::connected, &context, [=]() {
        m_alice2.manager->setUp();
    });

    connect(&m_alice2.logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            qDebug() << "Alice 2 - SENT: " << text;
        } else {
            qDebug() << "Alice 2 - RECEIVED: " << text;
        }
    });

    connect(&m_alice2.client, &QXmppClient::messageReceived, &context, [=, &isFirstMessageDecryptedByAlice2, &isSecondMessageDecryptedByAlice2](const QXmppMessage &receivedMessage) {
        // Process only encrypted stanzas.
        if (receivedMessage.e2eeMetadata()) {
            qDebug() << "Decrypted message:" << receivedMessage.body();
            if (receivedMessage.body() == message1.body()) {
                isFirstMessageDecryptedByAlice2 = true;
            } else if (receivedMessage.body() == message2.body()) {
                isSecondMessageDecryptedByAlice2 = true;
            }
        }
    });

    connect(&m_alice1.logger, &QXmppLogger::message, &context, [=, &isEmptyOmemoMessageReceivedByAlice1, &isSecondMessageSentByAlice1](QXmppLogger::MessageType type, const QString &text) mutable {
        if (type == QXmppLogger::SentMessage) {
            qDebug() << "Alice - SENT: " << text;
        } else if (type == QXmppLogger::ReceivedMessage) {
            qDebug() << "Alice - RECEIVED: " << text;

            // Check if Alice 1 received an empty OMEMO message from Alice 2.
            // If that is the case, send a second message to Alice 2.
            // The empty OMEMO message is not emitted via QXmppClient::messageReceived().
            // Thus, it must be parsed manually here.
            const auto content = text.toUtf8();
            if (content.startsWith(QByteArrayLiteral("<message "))) {
                QXmppMessage message;
                parsePacket(message, text.toUtf8());

                if (const auto optionalOmemoElement = message.omemoElement(); optionalOmemoElement && optionalOmemoElement.value().payload().isEmpty()) {
                    isEmptyOmemoMessageReceivedByAlice1 = true;

                    auto future = m_alice1.client.sendSensitive(std::move(message2), QXmppSendStanzaParams());
                    future.then(this, [=, &isSecondMessageSentByAlice1](QXmpp::SendResult result) {
                        if (std::get_if<QXmpp::SendSuccess>(&result)) {
                            isSecondMessageSentByAlice1 = true;
                        }
                    });
                }
            }
        }
    });

    // Wait for receiving the device of Alice 2 in order to send a message to Bob and a message
    // carbon to Alice 2.
    connect(m_alice1.manager, &QXmppOmemoManager::deviceAdded, &context, [=, &isFirstMessageSentByAlice1](const QString &jid, uint32_t) mutable {
        if (jid == m_alice2.client.configuration().jidBare()) {
            if (!isFirstMessageSentByAlice1) {
                auto future = m_alice1.client.sendSensitive(std::move(message1), QXmppSendStanzaParams());
                future.then(this, [=, &isFirstMessageSentByAlice1](QXmpp::SendResult result) {
                    if (std::get_if<QXmpp::SendSuccess>(&result)) {
                        isFirstMessageSentByAlice1 = true;
                    }
                });
            }
        }
    });

    m_alice1.client.connectToServer(config1);

    QTRY_VERIFY_WITH_TIMEOUT(isFirstMessageSentByAlice1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(isFirstMessageDecryptedByAlice2, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(isEmptyOmemoMessageReceivedByAlice1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(isSecondMessageSentByAlice1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(isSecondMessageDecryptedByAlice2, 10000);

    m_alice1.client.disconnectFromServer();
    QVERIFY2(disconnectedAlice1Spy.wait(), "Could not disconnect from server!");
    finish(m_alice2);
}

void tst_QXmppOmemoManager::testSendIq()
{
    SKIP_IF_INTEGRATION_TESTS_DISABLED()

    QSignalSpy disconnectedAlice1Spy(&m_alice1.client, &QXmppClient::disconnected);

    auto isFirstRequestSent = false;
    auto isErrorResponseReceived = false;
    auto isSecondRequestSent = false;
    auto isResultResponseReceived = false;

    const auto config1 = IntegrationTests::clientConfiguration();
    auto config2 = config1;
    config2.setResource(config2.resource() % QStringLiteral("2"));

    const QObject context;

    QXmppBitsOfBinaryIq requestIq;
    requestIq.setTo(config2.jid());
    requestIq.setCid(QXmppBitsOfBinaryContentId::fromContentId(QStringLiteral("sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org")));

    QXmppBitsOfBinaryIq responseIq;
    responseIq.setType(QXmppIq::Result);
    responseIq.setTo(config1.jid());
    responseIq.setData(QByteArray::fromBase64(QByteArrayLiteral(
        "iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAMAAAC67D+PAAAAclBMVEUAAADYZArfaA9GIAoBAAGN"
        "QA3MXgniaAiEOgZMIATDXRXZZhHUZBHIXhDrbQ6sUQ7OYA2TRAubRwqMQQq7VQlKHgMAAAK5WRfJ"
        "YBOORBFoMBCwUQ/ycA6FPgvbZQpeKglNJQmrTQeOPgQyFwR6MwACAABRPE/oAAAAW0lEQVQI1xXI"
        "Rw6EMBTAUP8kJKENnaF37n9FQPLCekAgzklhgCwfrlNHEXhrvCsxaU/SwLGAFuIWZFpBERtKm9Xf"
        "JqH+vVWh4POqgHrsAtht095b+geYRSl57QHSPgP3+CwvAAAAAABJRU5ErkJggg==")));

    OmemoIqHandler iqHandler(requestIq, responseIq);

    connect(&m_alice1.client, &QXmppClient::connected, &context, [=]() {
        auto future = m_alice1.manager->setUp();
        future.then(this, [=](bool isSetUp) {
            if (isSetUp) {
                auto future = m_alice1.manager->setSecurityPolicy(Toakafa);
                future.then(this, [=]() {
                    auto future = m_alice2.manager->setSecurityPolicy(Toakafa);
                    future.then(this, [=]() {
                        m_alice2.client.connectToServer(config2);
                    });
                });
            }
        });
    });

    connect(&m_alice2.client, &QXmppClient::connected, &context, [=]() {
        m_alice2.manager->setUp();
    });

    connect(&m_alice1.logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            qDebug() << "Alice - SENT: " << text;
        } else if (type == QXmppLogger::ReceivedMessage) {
            qDebug() << "Alice - RECEIVED: " << text;
        }
    });

    connect(&m_alice2.logger, &QXmppLogger::message, &context, [=](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            qDebug() << "Alice 2 - SENT: " << text;
        } else {
            qDebug() << "Alice 2 - RECEIVED: " << text;
        }
    });

    // Wait for receiving the device of Alice 2 in order to send a request to it.
    connect(m_alice1.manager, &QXmppOmemoManager::deviceAdded, &context, [=, &isFirstRequestSent, &isErrorResponseReceived, &isSecondRequestSent, &isResultResponseReceived, &iqHandler](const QString &jid, uint32_t) {
        if (jid != m_alice2.client.configuration().jidBare()) {
            return;
        }
        if (!isFirstRequestSent && !isSecondRequestSent) {
            auto requestIqCopy = requestIq;
            auto future = m_alice1.client.sendSensitiveIq(std::move(requestIqCopy));
            future.then(this, [=, &isFirstRequestSent, &isErrorResponseReceived, &isSecondRequestSent, &isResultResponseReceived, &iqHandler](QXmppClient::IqResult result) {
                if (const auto response = std::get_if<QDomElement>(&result)) {
                    isFirstRequestSent = true;

                    QXmppIq iq;
                    iq.parse(*response);

                    QCOMPARE(iq.type(), QXmppIq::Error);
                    const auto error = iq.error();
                    QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
                    QCOMPARE(error.condition(), QXmppStanza::Error::FeatureNotImplemented);
                    isErrorResponseReceived = true;

                    m_alice2.client.addExtension(&iqHandler);

                    auto requestIqCopy = requestIq;
                    auto future = m_alice1.client.sendSensitiveIq(std::move(requestIqCopy));
                    future.then(this, [=, &isSecondRequestSent, &isResultResponseReceived](QXmppClient::IqResult result) {
                        if (const auto response = std::get_if<QDomElement>(&result)) {
                            isSecondRequestSent = true;

                            if (QXmppBitsOfBinaryIq::isBitsOfBinaryIq(*response)) {
                                QXmppBitsOfBinaryIq iq;
                                iq.parse(*response);
                                QCOMPARE(iq.data(), responseIq.data());
                                isResultResponseReceived = true;
                            }
                        }
                    });
                }
            });
        }
    });

    m_alice1.client.connectToServer(config1);

    QTRY_VERIFY_WITH_TIMEOUT(isFirstRequestSent, 20'000);
    QTRY_VERIFY(isErrorResponseReceived);
    QTRY_VERIFY(isSecondRequestSent);
    QTRY_VERIFY(isResultResponseReceived);

    m_alice1.client.disconnectFromServer();
    QVERIFY2(disconnectedAlice1Spy.wait(), "Could not disconnect from server!");
    finish(m_alice2);
}

void tst_QXmppOmemoManager::finish(OmemoUser &omemoUser)
{
    QSignalSpy disconnectedSpy(&omemoUser.client, &QXmppClient::disconnected);

    bool isManagerReset = false;

    auto future = omemoUser.manager->resetAll();
    future.then(this, [=, &isManagerReset, &omemoUser](bool isReset) {
        if (isReset) {
            isManagerReset = true;
        }

        omemoUser.client.disconnectFromServer();
    });

    QVERIFY2(disconnectedSpy.wait(), "Could not disconnect from server!");
    QTRY_VERIFY(isManagerReset);
}

QTEST_MAIN(tst_QXmppOmemoManager)
#include "tst_qxmppomemomanager.moc"
