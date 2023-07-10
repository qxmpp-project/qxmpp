// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCallInviteManager.h"
#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "IntegrationTesting.h"
#include "util.h"
#include <QTest>

using CallInviteType = QXmppCallInviteElement::Type;
using Result = QXmppCallInvite::Result;

class tst_QXmppCallInviteManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testClear();
    Q_SLOT void testClearAll();

    Q_SLOT void testAccept();
    Q_SLOT void testReject();
    Q_SLOT void testRetract();
    Q_SLOT void testLeft();

    Q_SLOT void testInvite();
    Q_SLOT void testSendMessage();

    Q_SLOT void testHandleExistingCallInvite();
    Q_SLOT void testHandleCallInviteElement();
    Q_SLOT void testHandleMessage_data();
    Q_SLOT void testHandleMessage();
    Q_SLOT void testHandleMessageAccepted();
    Q_SLOT void testHandleMessageRejected();
    Q_SLOT void testHandleMessageRetracted();
    Q_SLOT void testHandleMessageLeft();

    QXmppClient m_client;
    QXmppLogger m_logger;
    QXmppCallInviteManager m_manager;
};

void tst_QXmppCallInviteManager::initTestCase()
{
    m_client.addExtension(&m_manager);

    m_logger.setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(&m_logger);

    m_client.connectToServer(IntegrationTests::clientConfiguration());
    m_client.configuration().setJid("mixer@example.com");

    qRegisterMetaType<QXmppCallInvite::Result>();
    qRegisterMetaType<std::shared_ptr<QXmppCallInvite>>();
}

void tst_QXmppCallInviteManager::testClear()
{
    QCOMPARE(m_manager.callInvites().size(), 0);
    auto callInvite1 { m_manager.addCallInvite("test1") };
    auto callInvite2 { m_manager.addCallInvite("test2") };
    QCOMPARE(m_manager.callInvites().size(), 2);

    m_manager.clear(callInvite1);
    m_manager.clear(callInvite2);
    QCOMPARE(m_manager.callInvites().size(), 0);
}

void tst_QXmppCallInviteManager::testClearAll()
{
    QCOMPARE(m_manager.callInvites().size(), 0);
    m_manager.addCallInvite("test1");
    m_manager.addCallInvite("test2");
    m_manager.addCallInvite("test3");
    m_manager.addCallInvite("test4");
    m_manager.addCallInvite("test5");
    QCOMPARE(m_manager.callInvites().size(), 5);

    m_manager.clearAll();
    QCOMPARE(m_manager.callInvites().size(), 0);
}

void tst_QXmppCallInviteManager::testAccept()
{
    auto callInvite { m_manager.addCallInvite("maraTestAccept@example.com") };
    callInvite->setId("id1_testAccept");

    connect(&m_logger, &QXmppLogger::message, this, [callInviteCallPartnerJid = callInvite->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == callInviteCallPartnerJid) {
                QVERIFY(message.callInviteElement());
                QCOMPARE(message.callInviteElement()->type(), CallInviteType::Accept);
            }
        }
    });

    auto future = callInvite->accept();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testReject()
{
    auto callInvite { m_manager.addCallInvite("maraTestReject@example.com") };
    callInvite->setId("id1_testReject");

    connect(&m_logger, &QXmppLogger::message, this, [callInviteCallPartnerJid = callInvite->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == callInviteCallPartnerJid) {
                QVERIFY(message.callInviteElement());
                QCOMPARE(message.callInviteElement()->id(), QStringLiteral("id1_testReject"));
                QCOMPARE(message.callInviteElement()->type(), CallInviteType::Reject);
            }
        }
    });

    auto future = callInvite->reject();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testRetract()
{
    auto callInvite { m_manager.addCallInvite("maraTestRetract@example.com") };
    callInvite->setId("id1_testRetract");

    connect(&m_logger, &QXmppLogger::message, this, [callInviteCallPartnerJid = callInvite->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == callInviteCallPartnerJid) {
                QVERIFY(message.callInviteElement());
                QCOMPARE(message.callInviteElement()->id(), QStringLiteral("id1_testRetract"));
                QCOMPARE(message.callInviteElement()->type(), CallInviteType::Retract);
            }
        }
    });

    auto future = callInvite->retract();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testLeft()
{
    auto callInvite { m_manager.addCallInvite("maraTestLeft@example.com") };
    callInvite->setId("id1_testLeft");

    connect(&m_logger, &QXmppLogger::message, this, [callInviteCallPartnerJid = callInvite->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == callInviteCallPartnerJid) {
                QVERIFY(message.callInviteElement());
                QCOMPARE(message.callInviteElement()->id(), QStringLiteral("id1_testLeft"));
                QCOMPARE(message.callInviteElement()->type(), CallInviteType::Left);
            }
        }
    });

    auto future = callInvite->leave();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testInvite()
{
    QString jid { "maraTestInvite@example.com" };
    bool video { true };
    bool audio { false };

    QXmppCallInviteElement::Jingle jingle;
    jingle.jid = "mixer@example.com/uuid";
    jingle.sid = "sid1";

    QVector<QXmppCallInviteElement::External> external;
    external.append({ "https://example.com/uuid" });
    external.append({ "tel:+12345678" });

    connect(&m_logger, &QXmppLogger::message, this, [&, jid, video, audio, jingle, external](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jid) {
                const auto &callInviteElement { message.callInviteElement() };
                QVERIFY(callInviteElement);

                QCOMPARE(callInviteElement->type(), CallInviteType::Invite);
                QVERIFY(!callInviteElement->id().isEmpty());
                QCOMPARE(callInviteElement->video(), video);
                QCOMPARE(callInviteElement->audio(), audio);
                QVERIFY(callInviteElement->jingle());
                QCOMPARE(callInviteElement->jingle().value(), jingle);
                QVERIFY(callInviteElement->external());
                QCOMPARE(callInviteElement->external().value(), external);

                SKIP_IF_INTEGRATION_TESTS_DISABLED()

                // verify that the Call Invite ID has been changed and the Call Invite was processed
                QCOMPARE(m_manager.callInvites().size(), 1);
            }
        }
    });

    auto future = m_manager.invite(jid, audio, video, jingle, external);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testSendMessage()
{
    QString jid { "maraSendMessage@example.com" };

    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Invite);
    callInviteElement.setId(QStringLiteral("id1_testSendMessage"));

    connect(&m_logger, &QXmppLogger::message, this, [jid, callInviteElement](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jid) {
                QVERIFY(message.callInviteElement());
                QCOMPARE(message.callInviteElement()->type(), callInviteElement.type());
                QCOMPARE(message.callInviteElement()->id(), callInviteElement.id());
            }
        }
    });

    auto future = m_manager.sendMessage(callInviteElement, jid);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleExistingCallInvite()
{
    QString callPartnerJid { "maraTestHandleExistingCallInvite@example.com" };
    QString callInviteId { "id1_testHandleExistingCallInvite" };

    auto callInvite { m_manager.addCallInvite(callPartnerJid) };
    callInvite->setId(callInviteId);

    QXmppCallInviteElement callInviteElement;
    callInviteElement.setId(callInviteId);

    // --- closed: rejected ---

    callInvite = m_manager.addCallInvite(callPartnerJid);
    callInvite->setId(callInviteId);

    callInviteElement.setType(CallInviteType::Reject);

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [callInviteElement](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Rejected>(result));
    });

    QVERIFY(m_manager.handleExistingCallInvite(callInvite, callInviteElement, callPartnerJid));
    m_manager.clearAll();

    // --- closed: retracted ---

    callInvite = m_manager.addCallInvite(callPartnerJid);
    callInvite->setId(callInviteId);

    callInviteElement.setType(CallInviteType::Retract);

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [callInviteElement](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Retracted>(result));
    });

    QVERIFY(m_manager.handleExistingCallInvite(callInvite, callInviteElement, callPartnerJid));
    m_manager.clearAll();

    // --- closed: left ---

    callInvite = m_manager.addCallInvite(callPartnerJid);
    callInvite->setId(callInviteId);

    callInviteElement.setType(CallInviteType::Left);

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [callInviteElement](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Left>(result));
    });

    QVERIFY(m_manager.handleExistingCallInvite(callInvite, callInviteElement, callPartnerJid));
    m_manager.clearAll();

    // --- none ---

    callInvite = m_manager.addCallInvite(callPartnerJid);
    callInvite->setId(callInviteId);

    callInviteElement.setType(CallInviteType::None);

    QCOMPARE(m_manager.handleExistingCallInvite(callInvite, callInviteElement, callPartnerJid), false);
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleCallInviteElement()
{
    QString callPartnerJid { "maraTestHandleCallInviteElement@example.com/orchard" };
    QString callInviteId { "id1_HandleCallInviteElement" };

    // case 1: no Call Invite element found in Call Invites vector and callInviteElement is not an invite element
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::None);

    QCOMPARE(m_manager.handleCallInviteElement(std::move(callInviteElement), {}), false);

    // case 2: no Call Invite found in Call Invites vector and callInviteElement is an invite element
    callInviteElement = {};
    callInviteElement.setType(CallInviteType::Invite);
    callInviteElement.setId(callInviteId);

    QSignalSpy invitedSpy(&m_manager, &QXmppCallInviteManager::invited);
    QVERIFY(m_manager.handleCallInviteElement(std::move(callInviteElement), callPartnerJid));
    QCOMPARE(invitedSpy.count(), 1);
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleMessage_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("xmlValid")
        << QByteArray(
               "<message id='id1' to='mara@example.com' type='chat'>"
               "<invite xmlns='urn:xmpp:call-invites:0' video='true'>"
               "<jingle sid='sid1'/>"
               "</invite>"
               "</message>")
        << true;

    QTest::newRow("xmlValidWithJingleJid")
        << QByteArray(
               "<message id='id1' to='mara@example.com' type='chat'>"
               "<invite xmlns='urn:xmpp:call-invites:0' video='true'>"
               "<jingle sid='sid1' jid='mixer@example.com/uuid'/>"
               "</invite>"
               "</message>")
        << true;

    QTest::newRow("xmlValidWithExternal")
        << QByteArray(
               "<message id='id1' to='mara@example.com' type='chat'>"
               "<invite xmlns='urn:xmpp:call-invites:0' video='true'>"
               "<jingle sid='sid1'/>"
               "<external uri='https://example.com/uuid'/>"
               "<external uri='tel:+12345678'/>"
               "</invite>"
               "</message>")
        << true;

    QTest::newRow("xmlInvalidNoJingle")
        << QByteArray(
               "<message id='id1' to='mara@example.com' type='chat'>"
               "<invite xmlns='urn:xmpp:call-invites:0' video='true'/>"
               "</message>")
        << true;

    QTest::newRow("xmlInvalidTypeNotChat")
        << QByteArray(
               "<message id='id1' to='mara@example.com' type='normal'>"
               "<invite xmlns='urn:xmpp:call-invites:0' video='true'>"
               "<jingle sid='sid1'/>"
               "</invite>"
               "</message>")
        << false;

    QTest::newRow("xmlInvalidNoCallInviteElement")
        << QByteArray("<message id='id1' to='mara@example.com' type='chat'/>")
        << false;
}

void tst_QXmppCallInviteManager::testHandleMessage()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QXmppMessage message;

    parsePacket(message, xml);
    QCOMPARE(m_manager.handleMessage(message), isValid);
    serializePacket(message, xml);

    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleMessageAccepted()
{
    QXmppMessage message;
    QByteArray xmlAccept {
        "<message to='maraTestHandleMessageAccepted@example.com' type='chat'>"
        "<accept id='id1_testHandleMessageAccepted' xmlns='urn:xmpp:call-invites:0'>"
        "<jingle sid='sid1' jid='mixer@example.com/uuid'/>"
        "</accept>"
        "</message>"
    };

    auto callInvite { m_manager.addCallInvite("mixer@example.com") };
    callInvite->setId("id1_testHandleMessageAccepted");

    QSignalSpy acceptedSpy(callInvite.get(), &QXmppCallInvite::accepted);

    message.parse(xmlToDom(xmlAccept));

    QVERIFY(m_manager.handleMessage(message));
    QCOMPARE(acceptedSpy.count(), 1);
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleMessageRejected()
{
    QXmppMessage message;
    QByteArray xmlReject {
        "<message to='maraTestHandleMessageRejected@example.com' type='chat'>"
        "<reject xmlns='urn:xmpp:call-invites:0' id='id1_testHandleMessageRejected'/>"
        "</message>"
    };

    auto callInvite { m_manager.addCallInvite("mixer@example.com") };
    callInvite->setId("id1_testHandleMessageRejected");

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Rejected>(result));
    });

    message.parse(xmlToDom(xmlReject));

    QVERIFY(m_manager.handleMessage(message));
    serializePacket(message, xmlReject);

    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleMessageRetracted()
{
    QXmppMessage message;
    QByteArray xmlRetract {
        "<message to='maraTestHandleMessageRetracted@example.com' type='chat'>"
        "<retract xmlns='urn:xmpp:call-invites:0' id='id1_testHandleMessageRetracted'/>"
        "</message>"
    };

    auto callInvite { m_manager.addCallInvite("mixer@example.com") };
    callInvite->setId("id1_testHandleMessageRetracted");

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Retracted>(result));
    });

    message.parse(xmlToDom(xmlRetract));

    QVERIFY(m_manager.handleMessage(message));
    serializePacket(message, xmlRetract);
    m_manager.clearAll();
}

void tst_QXmppCallInviteManager::testHandleMessageLeft()
{
    QXmppMessage message;
    QByteArray xmlLeft {
        "<message to='maraTestHandleMessageLeft@example.com' type='chat'>"
        "<left xmlns='urn:xmpp:call-invites:0' id='id1_testHandleMessageLeft'/>"
        "</message>"
    };

    auto callInvite { m_manager.addCallInvite("mixer@example.com") };
    callInvite->setId("id1_testHandleMessageLeft");

    connect(callInvite.get(), &QXmppCallInvite::closed, this, [](const Result &result) {
        QVERIFY(std::holds_alternative<QXmppCallInvite::Left>(result));
    });

    message.parse(xmlToDom(xmlLeft));

    QVERIFY(m_manager.handleMessage(message));
    serializePacket(message, xmlLeft);
    m_manager.clearAll();
}

QTEST_MAIN(tst_QXmppCallInviteManager)
#include "tst_qxmppcallinvitemanager.moc"
