// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppE2eeExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"

#include "util.h"
#include <QObject>

using namespace QXmpp::Private;

class tst_QXmppClient : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void handleMessageSent(QXmppLogger::MessageType type, const QString &text) const;
    void testSendMessage();

    void testIndexOfExtension();

    void testE2eeEncryption();

private:
    QXmppClient *client;
};

void tst_QXmppClient::handleMessageSent(QXmppLogger::MessageType type, const QString &text) const
{
    QCOMPARE(type, QXmppLogger::MessageType::SentMessage);

    QXmppMessage msg;
    parsePacket(msg, text.toUtf8());

    QCOMPARE(msg.from(), QString());
    QCOMPARE(msg.to(), QStringLiteral("support@qxmpp.org"));
    QCOMPARE(msg.body(), QStringLiteral("implement XEP-* plz"));
}

void tst_QXmppClient::initTestCase()
{
    client = new QXmppClient(this);
}

void tst_QXmppClient::testSendMessage()
{
    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::SignalLogging);
    client->setLogger(&logger);

    connect(&logger, &QXmppLogger::message, this, &tst_QXmppClient::handleMessageSent);

    client->sendMessage(
        QStringLiteral("support@qxmpp.org"),
        QStringLiteral("implement XEP-* plz"));

    // see handleMessageSent()

    client->setLogger(nullptr);
}

void tst_QXmppClient::testIndexOfExtension()
{
    auto client = new QXmppClient;

    for (auto *ext : client->extensions()) {
        client->removeExtension(ext);
    }

    auto rosterManager = new QXmppRosterManager(client);
    auto vCardManager = new QXmppVCardManager;

    client->addExtension(rosterManager);
    client->addExtension(vCardManager);

    // This extension is not in the list.
    QCOMPARE(client->indexOfExtension<QXmppVersionManager>(), -1);

    // These extensions are in the list.
    QCOMPARE(client->indexOfExtension<QXmppRosterManager>(), 0);
    QCOMPARE(client->indexOfExtension<QXmppVCardManager>(), 1);
}

class EncryptionExtension : public QXmppE2eeExtension
{
public:
    bool messageCalled = false;
    bool iqCalled = false;

    QFuture<MessageEncryptResult> encryptMessage(QXmppMessage &&) override
    {
        messageCalled = true;
        return makeReadyFuture<MessageEncryptResult>(QXmpp::SendError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    QFuture<IqEncryptResult> encryptIq(QXmppIq &&) override
    {
        iqCalled = true;
        return makeReadyFuture<IqEncryptResult>(QXmpp::SendError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    QFuture<IqDecryptResult> decryptIq(const QDomElement &) override
    {
        return makeReadyFuture<IqDecryptResult>(QXmpp::SendError { "it's only a test", QXmpp::SendError::EncryptionError });
    }
};

void tst_QXmppClient::testE2eeEncryption()
{
    QXmppClient client;
    EncryptionExtension encrypter;
    client.setEncryptionExtension(&encrypter);

    auto result = client.send(QXmppMessage("me@qxmpp.org", "somebody@qxmpp.org", "Hello"));
    QVERIFY(encrypter.messageCalled);
    QVERIFY(!encrypter.iqCalled);
    QCoreApplication::processEvents();
    expectFutureVariant<QXmpp::SendError>(result);

    encrypter.messageCalled = false;
    result = client.send(QXmppPresence(QXmppPresence::Available));
    QVERIFY(!encrypter.messageCalled);
    QVERIFY(!encrypter.iqCalled);

    auto createRequest = []() {
        QXmppDiscoveryIq request;
        request.setType(QXmppIq::Get);
        request.setQueryType(QXmppDiscoveryIq::InfoQuery);
        request.setTo("component.qxmpp.org");
        return request;
    };

    client.send(createRequest());
    QVERIFY(encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.sendUnencrypted(createRequest());
    QVERIFY(!encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.sendIq(createRequest());
    QVERIFY(!encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.sendSensitiveIq(createRequest());
    QVERIFY(encrypter.iqCalled);
    encrypter.iqCalled = false;
}

QTEST_MAIN(tst_QXmppClient)
#include "tst_qxmppclient.moc"
