// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppRegistrationManager.h"
#include "QXmppStreamFeatures.h"

#include "util.h"

class tst_QXmppRegistrationManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testDiscoFeatures();

    Q_SLOT void testChangePassword_data();
    Q_SLOT void testChangePassword();
    Q_SLOT void testDeleteAccount();
    Q_SLOT void testRequestRegistrationForm_data();
    Q_SLOT void testRequestRegistrationForm();
    Q_SLOT void testRegisterOnConnectGetSet();
    Q_SLOT void testServiceDiscovery();
    Q_SLOT void testSendCachedRegistrationForm_data();
    Q_SLOT void testSendCachedRegistrationForm();
    Q_SLOT void testStreamFeaturesCheck_data();
    Q_SLOT void testStreamFeaturesCheck();
    Q_SLOT void testRegistrationResult_data();
    Q_SLOT void testRegistrationResult();
    Q_SLOT void testChangePasswordResult_data();
    Q_SLOT void testChangePasswordResult();
    Q_SLOT void testDeleteAccountResult_data();
    Q_SLOT void testDeleteAccountResult();
    Q_SLOT void testRegistrationFormReceived();

    Q_SLOT void sendStreamFeaturesToManager(bool registrationEnabled = true);
    Q_SLOT void setManagerConfig(const QString &username, const QString &server = u"example.org"_s, const QString &password = {});

    QXmppClient client;
    QXmppLogger logger;
    QXmppRegistrationManager *manager;
    QString expectedXml;
};

void tst_QXmppRegistrationManager::initTestCase()
{
    manager = new QXmppRegistrationManager;
    client.addExtension(manager);

    logger.setLoggingType(QXmppLogger::SignalLogging);
    client.setLogger(&logger);
}

void tst_QXmppRegistrationManager::testDiscoFeatures()
{
    QCOMPARE(manager->discoveryFeatures(), QStringList() << "jabber:iq:register");
}

void tst_QXmppRegistrationManager::testChangePassword_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");

#define ROW(name, username, password) \
    QTest::newRow(name) << QStringLiteral(username) << QStringLiteral(password)

    ROW("user-bill", "bill", "m1cr0$0ft");
    ROW("user-alice", "alice", "bitten-apple");

#undef ROW
}

void tst_QXmppRegistrationManager::testChangePassword()
{
    QFETCH(QString, username);
    QFETCH(QString, password);

    setManagerConfig(username, u"example.org"_s, password);

    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [=](QXmppLogger::MessageType type, const QString &text) {
        QCOMPARE(type, QXmppLogger::SentMessage);

        QXmppRegisterIq iq;
        parsePacket(iq, text.toUtf8());

        QVERIFY(!iq.id().isEmpty());
        QCOMPARE(iq.type(), QXmppIq::Set);
        QCOMPARE(iq.username(), username);
        QCOMPARE(iq.password(), password);

        delete context;  // disconnects lambda
    });

    manager->changePassword(password);
}

void tst_QXmppRegistrationManager::testDeleteAccount()
{
    setManagerConfig(u"bob"_s);

    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [=](QXmppLogger::MessageType type, const QString &text) {
        QCOMPARE(type, QXmppLogger::SentMessage);

        QXmppRegisterIq iq;
        parsePacket(iq, text.toUtf8());

        QVERIFY(!iq.id().isEmpty());
        // to address must be the server or empty
        QVERIFY(iq.to() == u"example.org" || iq.to().isEmpty());
        QCOMPARE(iq.type(), QXmppIq::Set);
        QVERIFY(iq.isRemove());

        delete context;  // disconnects lambda
    });

    manager->deleteAccount();
}

void tst_QXmppRegistrationManager::testRequestRegistrationForm_data()
{
    QTest::addColumn<bool>("triggerManually");

#define ROW(name, enabled) \
    QTest::newRow(name) << enabled

    ROW("trigger-manually", true);
    ROW("request-form-upon-stream-features", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testRequestRegistrationForm()
{
    QFETCH(bool, triggerManually);

    setManagerConfig(u"bob"_s);

    manager->setRegistrationFormToSend(QXmppRegisterIq());
    manager->setRegisterOnConnectEnabled(true);

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            signalCalled = true;

            QVERIFY(text.contains(u"<query xmlns=\"jabber:iq:register\"/>"_s));

            QXmppRegisterIq iq;
            parsePacket(iq, text.toUtf8());

            QVERIFY(!iq.id().isEmpty());
            QCOMPARE(iq.type(), QXmppIq::Get);
        }
    });

    if (triggerManually) {
        manager->requestRegistrationForm();
    } else {
        sendStreamFeaturesToManager(true);
    }

    QVERIFY(signalCalled);
    delete context;
    manager->setRegisterOnConnectEnabled(false);
}

void tst_QXmppRegistrationManager::testRegisterOnConnectGetSet()
{
    manager->setRegisterOnConnectEnabled(true);
    QVERIFY(manager->registerOnConnectEnabled());

    manager->setRegisterOnConnectEnabled(false);
    QVERIFY(!manager->registerOnConnectEnabled());
}

void tst_QXmppRegistrationManager::testServiceDiscovery()
{
    setManagerConfig(u"bob"_s);

    bool signalEmitted = false;
    QObject *context = new QObject(this);
    connect(manager, &QXmppRegistrationManager::supportedByServerChanged, context, [&]() {
        signalEmitted = true;
        QCOMPARE(manager->supportedByServer(), true);
    });

    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setFrom(u"example.org"_s);
    iq.setTo(u"bob@example.org"_s);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);
    iq.setFeatures(QStringList() << u"jabber:iq:register"_s);

    client.findExtension<QXmppDiscoveryManager>()->handleStanza(writePacketToDom(iq));

    QVERIFY(signalEmitted);
    QVERIFY(manager->supportedByServer());
    delete context;

    // on disconnect, supportedByServer needs to be reset
    Q_EMIT client.disconnected();
    QVERIFY(!manager->supportedByServer());
}

void tst_QXmppRegistrationManager::testSendCachedRegistrationForm_data()
{
    QTest::addColumn<bool>("triggerSendingManually");

#define ROW(name, enabled) \
    QTest::newRow(name) << enabled

    ROW("manually-trigger-sending", true);
    ROW("sending-upon-correct-stream-features", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testSendCachedRegistrationForm()
{
    QFETCH(bool, triggerSendingManually);

    setManagerConfig(u"bob"_s);

    QXmppRegisterIq iq;
    iq.setUsername(u"someone"_s);
    iq.setPassword(u"s3cr3t"_s);
    iq.setEmail(u"1234@example.org"_s);

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            signalCalled = true;

            QXmppRegisterIq parsedIq;
            parsePacket(parsedIq, text.toUtf8());

            QCOMPARE(parsedIq.id(), iq.id());
            QCOMPARE(parsedIq.type(), QXmppIq::Set);
            QCOMPARE(parsedIq.username(), u"someone"_s);
            QCOMPARE(parsedIq.password(), u"s3cr3t"_s);
            QCOMPARE(parsedIq.email(), u"1234@example.org"_s);
        }
    });

    manager->setRegistrationFormToSend(iq);
    if (triggerSendingManually) {
        manager->sendCachedRegistrationForm();
    } else {
        sendStreamFeaturesToManager(true);
    }

    delete context;
}

void tst_QXmppRegistrationManager::testStreamFeaturesCheck_data()
{
    QTest::addColumn<bool>("registrationEnabled");

#define ROW(name, enabled) \
    QTest::newRow(name) << enabled

    ROW("registration-enabled", true);
    ROW("registration-disabled", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testStreamFeaturesCheck()
{
    QFETCH(bool, registrationEnabled);

    bool signalEmitted = false;
    QObject *context = new QObject(this);
    connect(manager, &QXmppRegistrationManager::registrationFailed, context, [&](const QXmppStanza::Error &error) {
        signalEmitted = true;

        QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
        QCOMPARE(error.condition(), QXmppStanza::Error::FeatureNotImplemented);
    });

    manager->setRegisterOnConnectEnabled(true);
    sendStreamFeaturesToManager(registrationEnabled);

    QCOMPARE(signalEmitted, !registrationEnabled);
    delete context;
    manager->setRegisterOnConnectEnabled(false);
}

void tst_QXmppRegistrationManager::testRegistrationResult_data()
{
    QTest::addColumn<bool>("isSuccess");

#define ROW(name, isSuccess) \
    QTest::newRow(name) << isSuccess

    ROW("success", true);
    ROW("error", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testRegistrationResult()
{
    QFETCH(bool, isSuccess);

    QXmppRegisterIq registrationRequestForm;
    registrationRequestForm.setUsername(u"someone"_s);
    registrationRequestForm.setPassword(u"s3cr3t"_s);
    registrationRequestForm.setEmail(u"1234@example.org"_s);
    registrationRequestForm.setId(u"register1"_s);

    bool succeededSignalCalled = false;
    bool failedSignalCalled = false;

    QObject *context = new QObject(this);

    connect(manager, &QXmppRegistrationManager::registrationSucceeded, context, [&]() {
        succeededSignalCalled = true;
    });
    connect(manager, &QXmppRegistrationManager::registrationFailed, context, [&](const QXmppStanza::Error &) {
        failedSignalCalled = true;
    });

    manager->setRegistrationFormToSend(registrationRequestForm);
    manager->sendCachedRegistrationForm();

    QXmppIq serverResult(isSuccess ? QXmppIq::Result : QXmppIq::Error);
    serverResult.setId(registrationRequestForm.id());

    manager->handleStanza(writePacketToDom(serverResult));

    QCOMPARE(succeededSignalCalled, isSuccess);
    QCOMPARE(failedSignalCalled, !isSuccess);

    delete context;
}

void tst_QXmppRegistrationManager::testChangePasswordResult_data()
{
    QTest::addColumn<bool>("isSuccess");

#define ROW(name, isSuccess) \
    QTest::newRow(name) << isSuccess

    ROW("success", true);
    ROW("error", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testChangePasswordResult()
{
    QFETCH(bool, isSuccess);

    QString changePasswordRequestIqId;

    bool requestSentSignalCalled = false;
    QObject *requestSentSignalContext = new QObject(this);
    connect(&logger, &QXmppLogger::message, requestSentSignalContext, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            requestSentSignalCalled = true;

            QXmppIq parsedIq;
            parsePacket(parsedIq, text.toUtf8());
            changePasswordRequestIqId = parsedIq.id();
        }
    });

    manager->changePassword({});
    QVERIFY(requestSentSignalCalled);
    QVERIFY(!changePasswordRequestIqId.isEmpty());
    delete requestSentSignalContext;

    bool resultSignalCalled = false;
    QObject *resultContext = new QObject(this);
    if (isSuccess) {
        connect(manager, &QXmppRegistrationManager::passwordChanged, resultContext, [&](const QString &) {
            resultSignalCalled = true;
        });
    } else {
        connect(manager, &QXmppRegistrationManager::passwordChangeFailed, resultContext, [&](QXmppStanza::Error) {
            resultSignalCalled = true;
        });
    }

    QXmppIq serverResult(isSuccess ? QXmppIq::Result : QXmppIq::Error);
    serverResult.setId(changePasswordRequestIqId);

    manager->handleStanza(writePacketToDom(serverResult));

    QVERIFY(resultSignalCalled);
    delete resultContext;
}

void tst_QXmppRegistrationManager::testDeleteAccountResult_data()
{
    QTest::addColumn<bool>("isSuccess");

#define ROW(name, isSuccess) \
    QTest::newRow(name) << isSuccess

    ROW("success", true);
    ROW("error", false);

#undef ROW
}

void tst_QXmppRegistrationManager::testDeleteAccountResult()
{
    QFETCH(bool, isSuccess);

    QString deleteAccountRequestIqId;

    bool requestSentSignalCalled = false;
    QObject *requestSentSignalContext = new QObject(this);
    connect(&logger, &QXmppLogger::message, requestSentSignalContext, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            requestSentSignalCalled = true;

            QXmppIq parsedIq;
            parsePacket(parsedIq, text.toUtf8());
            deleteAccountRequestIqId = parsedIq.id();
        }
    });

    manager->deleteAccount();
    QVERIFY(requestSentSignalCalled);
    QVERIFY(!deleteAccountRequestIqId.isEmpty());
    delete requestSentSignalContext;

    bool resultSignalCalled = false;
    QObject *resultContext = new QObject(this);
    if (isSuccess) {
        connect(manager, &QXmppRegistrationManager::accountDeleted, resultContext, [&]() {
            resultSignalCalled = true;
        });
    } else {
        connect(manager, &QXmppRegistrationManager::accountDeletionFailed, resultContext, [&](QXmppStanza::Error) {
            resultSignalCalled = true;
        });
    }

    QXmppIq serverResult(isSuccess ? QXmppIq::Result : QXmppIq::Error);
    serverResult.setId(deleteAccountRequestIqId);

    manager->handleStanza(writePacketToDom(serverResult));

    QVERIFY(resultSignalCalled);
    delete resultContext;
}

void tst_QXmppRegistrationManager::testRegistrationFormReceived()
{
    QXmppRegisterIq iq;
    iq.setUsername("");
    iq.setPassword("");

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(manager, &QXmppRegistrationManager::registrationFormReceived, context, [&](const QXmppRegisterIq &) {
        signalCalled = true;
        QCOMPARE(iq.username(), u""_s);
        QCOMPARE(iq.password(), u""_s);
    });

    manager->handleStanza(writePacketToDom(iq));

    delete context;
}

void tst_QXmppRegistrationManager::sendStreamFeaturesToManager(bool registrationEnabled)
{
    QXmppStreamFeatures features;
    features.setBindMode(QXmppStreamFeatures::Enabled);
    if (registrationEnabled) {
        features.setRegisterMode(QXmppStreamFeatures::Enabled);
    }

    auto writeFeaturesToDom = [&]() -> QDomElement {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        QXmlStreamWriter writer(&buffer);
        features.toXml(&writer);

        // hacky hack to include stream namespace
        QByteArray manipulatedXml = buffer.data();
        manipulatedXml.replace("stream:", QByteArray());
        manipulatedXml.insert(9, QByteArrayLiteral(" xmlns=\"http://etherx.jabber.org/streams\""));

        QDomDocument doc;
        doc.setContent(manipulatedXml, true);
        return doc.documentElement();
    };

    manager->handleStanza(writeFeaturesToDom());
}

void tst_QXmppRegistrationManager::setManagerConfig(const QString &username, const QString &server, const QString &password)
{
    client.connectToServer(username + u'@' + server, password);
    client.disconnectFromServer();
}

QTEST_MAIN(tst_QXmppRegistrationManager)
#include "tst_qxmppregistrationmanager.moc"
