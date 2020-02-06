/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppRegistrationManager.h"
#include "QXmppStreamFeatures.h"

#include "util.h"

template<class T>
QDomElement writePacketToDom(T packet)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);

    QDomDocument doc;
    doc.setContent(buffer.data(), true);

    return doc.documentElement();
}

class tst_QXmppRegistrationManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testDiscoFeatures();

    void testChangePassword_data();
    void testChangePassword();
    void testDeleteAccount();
    void testRequestRegistrationForm_data();
    void testRequestRegistrationForm();
    void testRegisterOnConnectGetSet();
    void testServiceDiscovery();
    void testSendCachedRegistrationForm_data();
    void testSendCachedRegistrationForm();
    void testStreamFeaturesCheck_data();
    void testStreamFeaturesCheck();
    void testRegistrationResult_data();
    void testRegistrationResult();
    void testChangePasswordResult_data();
    void testChangePasswordResult();
    void testRegistrationFormReceived();

    void sendStreamFeaturesToManager(bool registrationEnabled = true);
    void setManagerConfig(const QString &username, const QString &server = QStringLiteral("example.org"), const QString &password = {});

private:
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

    setManagerConfig(username, QStringLiteral("example.org"), password);

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
    setManagerConfig(QStringLiteral("bob"));

    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [=](QXmppLogger::MessageType type, const QString &text) {
        QCOMPARE(type, QXmppLogger::SentMessage);

        QXmppRegisterIq iq;
        parsePacket(iq, text.toUtf8());

        QVERIFY(!iq.id().isEmpty());
        // to address must be the server or empty
        QVERIFY(iq.to() == QStringLiteral("example.org") || iq.to().isEmpty());
        QCOMPARE(iq.type(), QXmppIq::Set);
        QVERIFY(iq.isRemove());

        delete context;  // disconnects lambda
    });

    const QString id = manager->deleteAccount();
    // we're not connnected, so the id should be null
    QVERIFY(id.isNull());
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

    setManagerConfig(QStringLiteral("bob"));

    manager->setRegistrationFormToSend(QXmppRegisterIq());
    manager->setRegisterOnConnectEnabled(true);

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            signalCalled = true;

            QVERIFY(text.contains(QStringLiteral("<query xmlns=\"jabber:iq:register\"/>")));

            QXmppRegisterIq iq;
            parsePacket(iq, text.toUtf8());

            QVERIFY(!iq.id().isEmpty());
            QCOMPARE(iq.type(), QXmppIq::Get);
        }
    });

    if (triggerManually)
        manager->requestRegistrationForm();
    else
        sendStreamFeaturesToManager(true);

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
    setManagerConfig(QStringLiteral("bob"));

    bool signalEmitted = false;
    QObject *context = new QObject(this);
    connect(manager, &QXmppRegistrationManager::supportedByServerChanged, context, [&]() {
        signalEmitted = true;
        QCOMPARE(manager->supportedByServer(), true);
    });

    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setFrom(QStringLiteral("example.org"));
    iq.setTo(QStringLiteral("bob@example.org"));
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);
    iq.setFeatures(QStringList() << QStringLiteral("jabber:iq:register"));

    client.findExtension<QXmppDiscoveryManager>()->handleStanza(writePacketToDom(iq));

    QVERIFY(signalEmitted);
    QVERIFY(manager->supportedByServer());
    delete context;

    // on disconnect, supportedByServer needs to be reset
    emit client.disconnected();
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

    setManagerConfig(QStringLiteral("bob"));

    QXmppRegisterIq iq;
    iq.setUsername(QStringLiteral("someone"));
    iq.setPassword(QStringLiteral("s3cr3t"));
    iq.setEmail(QStringLiteral("1234@example.org"));

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(&logger, &QXmppLogger::message, context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            signalCalled = true;

            QXmppRegisterIq parsedIq;
            parsePacket(parsedIq, text.toUtf8());

            QCOMPARE(parsedIq.id(), iq.id());
            QCOMPARE(parsedIq.type(), QXmppIq::Set);
            QCOMPARE(parsedIq.username(), QStringLiteral("someone"));
            QCOMPARE(parsedIq.password(), QStringLiteral("s3cr3t"));
            QCOMPARE(parsedIq.email(), QStringLiteral("1234@example.org"));
        }
    });

    manager->setRegistrationFormToSend(iq);
    if (triggerSendingManually)
        manager->sendCachedRegistrationForm();
    else
        sendStreamFeaturesToManager(true);

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
    registrationRequestForm.setUsername(QStringLiteral("someone"));
    registrationRequestForm.setPassword(QStringLiteral("s3cr3t"));
    registrationRequestForm.setEmail(QStringLiteral("1234@example.org"));
    registrationRequestForm.setId(QStringLiteral("register1"));

    bool signalCalled = false;
    QObject *context = new QObject(this);
    if (isSuccess) {
        connect(manager, &QXmppRegistrationManager::registrationSucceeded, context, [&]() {
            signalCalled = true;
        });
    } else {
        connect(manager, &QXmppRegistrationManager::registrationFailed, context, [&](const QXmppStanza::Error &) {
            signalCalled = true;
        });
    }

    manager->setRegistrationFormToSend(registrationRequestForm);
    manager->sendCachedRegistrationForm();

    QXmppIq serverResult(isSuccess ? QXmppIq::Result : QXmppIq::Error);
    serverResult.setId(registrationRequestForm.id());

    manager->handleStanza(writePacketToDom(serverResult));

    QVERIFY(signalCalled);
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

void tst_QXmppRegistrationManager::testRegistrationFormReceived()
{
    QXmppRegisterIq iq;
    iq.setUsername("");
    iq.setPassword("");

    bool signalCalled = false;
    QObject *context = new QObject(this);
    connect(manager, &QXmppRegistrationManager::registrationFormReceived, context, [&](const QXmppRegisterIq &) {
        signalCalled = true;
        QCOMPARE(iq.username(), QStringLiteral(""));
        QCOMPARE(iq.password(), QStringLiteral(""));
    });

    manager->handleStanza(writePacketToDom(iq));

    delete context;
}

void tst_QXmppRegistrationManager::sendStreamFeaturesToManager(bool registrationEnabled)
{
    QXmppStreamFeatures features;
    features.setBindMode(QXmppStreamFeatures::Enabled);
    if (registrationEnabled)
        features.setRegisterMode(QXmppStreamFeatures::Enabled);

    auto writeFeaturesToDom = [&]() -> QDomElement {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        QXmlStreamWriter writer(&buffer);
        features.toXml(&writer);

        // hacky hack to include stream namespace
        QByteArray manipulatedXml = buffer.data();
        manipulatedXml.replace("stream:", QByteArray());
        manipulatedXml.insert(9, QStringLiteral(" xmlns=\"%1\"").arg("http://etherx.jabber.org/streams"));

        QDomDocument doc;
        doc.setContent(manipulatedXml, true);
        return doc.documentElement();
    };

    manager->handleStanza(writeFeaturesToDom());
}

void tst_QXmppRegistrationManager::setManagerConfig(const QString &username, const QString &server, const QString &password)
{
    client.connectToServer(username + QStringLiteral("@") + server, password);
    client.disconnectFromServer();
}

QTEST_MAIN(tst_QXmppRegistrationManager)
#include "tst_qxmppregistrationmanager.moc"
