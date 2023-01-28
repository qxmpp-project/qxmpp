// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppE2eeExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppPromise.h"
#include "QXmppRegisterIq.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"

#include "util.h"
#include <QObject>

using namespace QXmpp::Private;

class tst_QXmppClient : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void handleMessageSent(QXmppLogger::MessageType type, const QString &text) const;
    Q_SLOT void testSendMessage();
    Q_SLOT void testIndexOfExtension();
    Q_SLOT void testE2eeExtension();
    Q_SLOT void testTaskDirect();
    Q_SLOT void testTaskStore();

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
    auto client = std::make_unique<QXmppClient>();

    for (auto *ext : client->extensions()) {
        client->removeExtension(ext);
    }

    auto rosterManager = new QXmppRosterManager(client.get());
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

    QXmppTask<MessageEncryptResult> encryptMessage(QXmppMessage &&, const std::optional<QXmppSendStanzaParams> &) override
    {
        messageCalled = true;
        return makeReadyTask<MessageEncryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }
    QXmppTask<MessageDecryptResult> decryptMessage(QXmppMessage &&) override
    {
        return makeReadyTask<MessageDecryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    QXmppTask<IqEncryptResult> encryptIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> &) override
    {
        iqCalled = true;
        return makeReadyTask<IqEncryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    QXmppTask<IqDecryptResult> decryptIq(const QDomElement &) override
    {
        return makeReadyTask<IqDecryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    bool isEncrypted(const QDomElement &) override { return false; };
    bool isEncrypted(const QXmppMessage &) override { return false; };
};

void tst_QXmppClient::testE2eeExtension()
{
    QXmppClient client;
    EncryptionExtension encrypter;
    client.setEncryptionExtension(&encrypter);

    auto result = client.sendSensitive(QXmppMessage("me@qxmpp.org", "somebody@qxmpp.org", "Hello"));
    QVERIFY(encrypter.messageCalled);
    QVERIFY(!encrypter.iqCalled);
    QCoreApplication::processEvents();
    expectFutureVariant<QXmppError>(result.toFuture(this));

    encrypter.messageCalled = false;
    result = client.sendSensitive(QXmppPresence(QXmppPresence::Available));
    QVERIFY(!encrypter.messageCalled);
    QVERIFY(!encrypter.iqCalled);

    auto createRequest = []() {
        QXmppDiscoveryIq request;
        request.setType(QXmppIq::Get);
        request.setQueryType(QXmppDiscoveryIq::InfoQuery);
        request.setTo("component.qxmpp.org");
        return request;
    };

    client.sendSensitive(createRequest());
    QVERIFY(encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.send(createRequest());
    QVERIFY(!encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.sendIq(createRequest());
    QVERIFY(!encrypter.iqCalled);
    encrypter.iqCalled = false;

    client.sendSensitiveIq(createRequest());
    QVERIFY(encrypter.iqCalled);
    encrypter.iqCalled = false;
}

void tst_QXmppClient::testTaskDirect()
{
    QXmppPromise<QXmppIq> p;
    QXmppRegisterIq iq;
    iq.setUsername("username");

    bool thenCalled = false;
    p.task().then(this, [&thenCalled](QXmppIq &&iq) {
        thenCalled = true;
        // casting not supported
        QVERIFY(!dynamic_cast<QXmppRegisterIq *>(&iq));
    });
    p.finish(std::move(iq));

    QVERIFY(thenCalled);
    QVERIFY(p.task().isFinished());
    QVERIFY(!p.task().hasResult());
}

static QXmppTask<QXmppIq> generateRegisterIq()
{
    QXmppPromise<QXmppIq> p;
    QXmppRegisterIq iq;
    iq.setFrom("juliet");
    iq.setUsername("username");
    p.finish(std::move(iq));
    return p.task();
}

void tst_QXmppClient::testTaskStore()
{
    auto task = generateRegisterIq();

    bool thenCalled = false;
    task.then(this, [&thenCalled](QXmppIq &&iq) {
        thenCalled = true;

        QCOMPARE(iq.from(), QStringLiteral("juliet"));
        // casting not supported
        QVERIFY(!dynamic_cast<QXmppRegisterIq *>(&iq));
    });
    QVERIFY(thenCalled);

    QXmppPromise<QXmppIq> p;
    QXmppRegisterIq iq;
    iq.setUsername("username");
    p.finish(std::move(iq));

    QVERIFY(p.task().hasResult());
    QVERIFY(p.task().isFinished());

    thenCalled = false;
    p.task().then(this, [&thenCalled](QXmppIq &&iq) {
        thenCalled = true;
        // casting not supported
        QVERIFY(!dynamic_cast<QXmppRegisterIq *>(&iq));
    });
    QVERIFY(thenCalled);

    QVERIFY(p.task().isFinished());
    QVERIFY(!p.task().hasResult());
}

QTEST_MAIN(tst_QXmppClient)
#include "tst_qxmppclient.moc"
