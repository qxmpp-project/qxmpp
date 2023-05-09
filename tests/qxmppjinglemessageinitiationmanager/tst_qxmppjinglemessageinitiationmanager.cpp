// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppJingleMessageInitiationManager.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "IntegrationTesting.h"
#include "util.h"
#include <QTest>

using Jmi = QXmppJingleMessageInitiation;
using JmiType = QXmppJingleMessageInitiationElement::Type;
using Result = QXmppJingleMessageInitiation::Result;

class tst_QXmppJingleMessageInitiationManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testClear();
    Q_SLOT void testClearAll();

    Q_SLOT void testRing();
    Q_SLOT void testProceed();
    Q_SLOT void testReject();
    Q_SLOT void testRetract();
    Q_SLOT void testFinish();

    Q_SLOT void testPropose();
    Q_SLOT void testSendMessage();
    Q_SLOT void testHandleNonExistingSessionLowerId();
    Q_SLOT void testHandleNonExistingSessionHigherId();
    Q_SLOT void testHandleExistingSession();
    Q_SLOT void testHandleTieBreak();
    Q_SLOT void testHandleProposeJmiElement();
    Q_SLOT void testHandleExistingJmi();
    Q_SLOT void testHandleJmiElement();
    Q_SLOT void testHandleMessage_data();
    Q_SLOT void testHandleMessage();
    Q_SLOT void testHandleMessageRinging();
    Q_SLOT void testHandleMessageProceeded();
    Q_SLOT void testHandleMessageClosedRejected();
    Q_SLOT void testHandleMessageClosedRetracted();
    Q_SLOT void testHandleMessageClosedFinished();

    QXmppClient m_client;
    QXmppLogger m_logger;
    QXmppJingleMessageInitiationManager m_manager;
};

void tst_QXmppJingleMessageInitiationManager::initTestCase()
{
    m_client.addExtension(&m_manager);

    m_logger.setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(&m_logger);

    m_client.connectToServer(IntegrationTests::clientConfiguration());

    qRegisterMetaType<QXmppJingleMessageInitiation::Result>();
}

void tst_QXmppJingleMessageInitiationManager::testClear()
{
    QCOMPARE(m_manager.jmis().size(), 0);
    auto jmi1 { m_manager.addJmi("test1") };
    auto jmi2 { m_manager.addJmi("test2") };
    QCOMPARE(m_manager.jmis().size(), 2);

    m_manager.clear(jmi1);
    m_manager.clear(jmi2);
    QCOMPARE(m_manager.jmis().size(), 0);
}

void tst_QXmppJingleMessageInitiationManager::testClearAll()
{
    QCOMPARE(m_manager.jmis().size(), 0);
    m_manager.addJmi("test1");
    m_manager.addJmi("test2");
    m_manager.addJmi("test3");
    m_manager.addJmi("test4");
    m_manager.addJmi("test5");
    QCOMPARE(m_manager.jmis().size(), 5);

    m_manager.clearAll();
    QCOMPARE(m_manager.jmis().size(), 0);
}

void tst_QXmppJingleMessageInitiationManager::testRing()
{
    auto jmi { m_manager.addJmi("julietRing@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(&m_logger, &QXmppLogger::message, this, [jmicallPartnerJid = jmi->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmicallPartnerJid) {
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), JmiType::Ringing);
            }
        }
    });

    auto future = jmi->ring();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testProceed()
{
    auto jmi { m_manager.addJmi("julietProceed@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(&m_logger, &QXmppLogger::message, this, [jmiCallPartnerJid = jmi->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmiCallPartnerJid) {
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), JmiType::Proceed);
            }
        }
    });

    auto future = jmi->proceed();

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testReject()
{
    auto jmi { m_manager.addJmi("julietReject@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Decline);
    reason.setText("Declined");

    connect(&m_logger, &QXmppLogger::message, this, [jmiCallPartnerJid = jmi->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmiCallPartnerJid) {
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), JmiType::Reject);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->type(), QXmppJingleReason::Decline);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->text(), "Declined");
                QCOMPARE(message.jingleMessageInitiationElement()->containsTieBreak(), true);
            }
        }
    });

    auto future = jmi->reject(reason, true);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testRetract()
{
    auto jmi { m_manager.addJmi("julietRetract@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Gone);
    reason.setText("Gone");

    connect(&m_logger, &QXmppLogger::message, this, [jmicallPartnerJid = jmi->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmicallPartnerJid) {
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), JmiType::Retract);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->type(), QXmppJingleReason::Gone);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->text(), "Gone");
                QCOMPARE(message.jingleMessageInitiationElement()->containsTieBreak(), true);
            }
        }
    });

    auto future = jmi->retract(reason, true);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testFinish()
{
    auto jmi { m_manager.addJmi("julietFinish@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Success);
    reason.setText("Finished");

    connect(&m_logger, &QXmppLogger::message, this, [jmicallPartnerJid = jmi->callPartnerJid()](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmicallPartnerJid) {
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), JmiType::Finish);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->type(), QXmppJingleReason::Success);
                QCOMPARE(message.jingleMessageInitiationElement()->reason()->text(), "Finished");
                QCOMPARE(message.jingleMessageInitiationElement()->migratedTo(), "fecbea35-08d3-404f-9ec7-2b57c566fa74");
            }
        }
    });

    auto future = jmi->finish(reason, "fecbea35-08d3-404f-9ec7-2b57c566fa74");

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testPropose()
{
    QString jid { "julietPropose@capulet.example" };

    QXmppJingleDescription description;
    description.setMedia(QStringLiteral("audio"));
    description.setSsrc(123);
    description.setType(ns_jingle_rtp);

    connect(&m_logger, &QXmppLogger::message, this, [&, jid, description](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jid) {
                const auto &jmiElement { message.jingleMessageInitiationElement() };
                QVERIFY(jmiElement);

                QCOMPARE(jmiElement->type(), JmiType::Propose);
                QVERIFY(!jmiElement->id().isEmpty());
                QVERIFY(jmiElement->description());
                QCOMPARE(jmiElement->description()->media(), description.media());
                QCOMPARE(jmiElement->description()->ssrc(), description.ssrc());
                QCOMPARE(jmiElement->description()->type(), description.type());

                SKIP_IF_INTEGRATION_TESTS_DISABLED()

                // verify that the JMI ID has been changed and the JMI was processed
                QCOMPARE(m_manager.jmis().size(), 1);
            }
        }
    });

    auto future = m_manager.propose(jid, description);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testSendMessage()
{
    QString jid { "julietSendMessage@capulet.example" };

    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setType(JmiType::Propose);
    jmiElement.setId(QStringLiteral("fecbea35-08d3-404f-9ec7-2b57c566fa74"));

    connect(&m_logger, &QXmppLogger::message, this, [jid, jmiElement](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jid) {
                QVERIFY(message.hasHint(QXmppMessage::Store));
                QVERIFY(message.jingleMessageInitiationElement());
                QCOMPARE(message.jingleMessageInitiationElement()->type(), jmiElement.type());
            }
        }
    });

    auto future = m_manager.sendMessage(jmiElement, jid);

    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    QVERIFY(future.isFinished());
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleNonExistingSessionLowerId()
{
    // --- request with lower id sends propose to request with higher id ---

    QByteArray xmlProposeLowId {
        "<message from='romeoNonExistingSession@montague.example/low' to='juliet@capulet.example' type='chat'>"
        "<propose xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
        "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
        "</propose>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmiWithHigherId { m_manager.addJmi("romeoNonExistingSession@montague.example") };
    jmiWithHigherId->setId("fecbea35-08d3-404f-9ec7-2b57c566fa74");

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText("Tie-Break");

    // make sure that request with higher ID is being retracted
    connect(&m_logger, &QXmppLogger::message, this, [jmiWithHigherId, reason](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmiWithHigherId->callPartnerJid()) {
                const auto &jmiElement { message.jingleMessageInitiationElement() };
                QVERIFY(jmiElement);

                QCOMPARE(jmiElement->type(), JmiType::Retract);
                QCOMPARE(jmiElement->id(), "fecbea35-08d3-404f-9ec7-2b57c566fa74");
                QVERIFY(jmiElement->reason());
                QCOMPARE(jmiElement->reason()->type(), reason.type());
                QCOMPARE(jmiElement->reason()->text(), reason.text());

                SKIP_IF_INTEGRATION_TESTS_DISABLED()

                // verify that the JMI ID has been changed and the JMI was processed
                QCOMPARE(jmiWithHigherId->id(), "ca3cf894-5325-482f-a412-a6e9f832298d");
                QVERIFY(jmiWithHigherId->isProceeded());
            }
        }
    });

    QXmppMessage message;
    message.parse(xmlToDom(xmlProposeLowId));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleNonExistingSessionHigherId()
{
    // --- request with higher id sends propose to request with lower id ---
    QByteArray xmlProposeHighId {
        "<message from='julietNonExistingSession@capulet.example/high' to='romeo@montague.example' type='chat'>"
        "<propose xmlns='urn:xmpp:jingle-message:0' id='fecbea35-08d3-404f-9ec7-2b57c566fa74'>"
        "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
        "</propose>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText("Tie-Break");

    auto jmiWithLowerId { m_manager.addJmi("julietNonExistingSession@capulet.example") };
    jmiWithLowerId->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    // make sure that request with lower id rejects request with higher id
    connect(&m_logger, &QXmppLogger::message, this, [jid = jmiWithLowerId->callPartnerJid(), reason](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jid) {
                const auto &jmiElement { message.jingleMessageInitiationElement() };
                QVERIFY(jmiElement);

                QCOMPARE(jmiElement->type(), JmiType::Reject);
                QCOMPARE(jmiElement->id(), "fecbea35-08d3-404f-9ec7-2b57c566fa74");
                QVERIFY(jmiElement->reason());
                QCOMPARE(jmiElement->reason()->type(), reason.type());
                QCOMPARE(jmiElement->reason()->text(), reason.text());
            }
        }
    });

    QXmppMessage message;
    message.parse(xmlToDom(xmlProposeHighId));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleExistingSession()
{
    QXmppMessage message;

    QByteArray xmlPropose {
        "<message from='julietExistingSession@capulet.example/tablet' to='romeo@montague.example' type='chat'>"
        "<propose xmlns='urn:xmpp:jingle-message:0' id='989a46a6-f202-4910-a7c3-83c6ba3f3947'>"
        "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
        "</propose>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("julietExistingSession@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");
    jmi->setIsProceeded(true);

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText("Session migrated");

    connect(&m_logger, &QXmppLogger::message, this, [jmi, reason](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            QXmppMessage message;
            parsePacket(message, text.toUtf8());

            if (message.to() == jmi->callPartnerJid()) {
                const auto &jmiElement { message.jingleMessageInitiationElement() };
                QVERIFY(jmiElement);

                QCOMPARE(jmiElement->type(), JmiType::Finish);
                QCOMPARE(jmiElement->id(), jmi->id());
                QCOMPARE(jmiElement->migratedTo(), "989a46a6-f202-4910-a7c3-83c6ba3f3947");
                QVERIFY(jmiElement->reason());
                QCOMPARE(jmiElement->reason()->type(), reason.type());
                QCOMPARE(jmiElement->reason()->text(), reason.text());
            }
        }
    });

    message.parse(xmlToDom(xmlPropose));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleTieBreak()
{
    QString callPartnerJid { "romeoHandleTieBreakExistingSession@montague.example/orchard" };
    QString jmiId { "ca3cf894-5325-482f-a412-a6e9f832298d" };
    auto jmi { m_manager.addJmi(QXmppUtils::jidToBareJid(callPartnerJid)) };
    jmi->setId(jmiId);

    QXmppJingleMessageInitiationElement jmiElement;
    QString newJmiId("989a46a6-f202-4910-a7c3-83c6ba3f3947");
    jmiElement.setId(newJmiId);

    // Cannot use macro SKIP_IF_INTEGRATION_TESTS_DISABLED() here since
    // this would also skip the manager cleanup.
    if (IntegrationTests::enabled()) {
        // --- ensure handleExistingSession ---
        jmi->setIsProceeded(true);
        QSignalSpy closedSpy(jmi.get(), &QXmppJingleMessageInitiation::closed);
        QVERIFY(m_manager.handleTieBreak(jmi, jmiElement, QXmppUtils::jidToResource(callPartnerJid)));
        QCOMPARE(closedSpy.count(), 1);

        // --- ensure handleNonExistingSession ---
        jmi->setIsProceeded(false);
        QSignalSpy proceededSpy(jmi.get(), &QXmppJingleMessageInitiation::proceeded);
        QVERIFY(m_manager.handleTieBreak(jmi, jmiElement, QXmppUtils::jidToResource(callPartnerJid)));
        QCOMPARE(proceededSpy.count(), 1);
    }

    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleProposeJmiElement()
{
    QXmppJingleMessageInitiationElement jmiElement;

    QXmppJingleDescription description;
    description.setMedia("audio");
    description.setSsrc(321);
    description.setType("abcd");

    jmiElement.setId("ca3cf123-5325-482f-a412-a6e9f832298d");
    jmiElement.setDescription(description);

    QString callPartnerJid { "juliet@capulet.example" };

    // --- Tie break ---

    auto jmi { m_manager.addJmi(callPartnerJid) };
    jmi->setId("989a4123-f202-4910-a7c3-83c6ba3f3947");

    QVERIFY(m_manager.handleProposeJmiElement(jmiElement, callPartnerJid));
    QCOMPARE(m_manager.jmis().size(), 1);
    m_manager.clearAll();

    // --- usual JMI proposal ---

    connect(&m_manager, &QXmppJingleMessageInitiationManager::proposed, this, [&, jmiElement](const std::shared_ptr<Jmi> &, const QString &jmiElementId, const std::optional<QXmppJingleDescription> &description) {
        if (jmiElement.id() == jmiElementId) {
            QCOMPARE(m_manager.jmis().size(), 1);
            QVERIFY(description.has_value());
            QCOMPARE(description->media(), jmiElement.description()->media());
            QCOMPARE(description->ssrc(), jmiElement.description()->ssrc());
            QCOMPARE(description->type(), jmiElement.description()->type());
        }
    });

    callPartnerJid = "romeoHandleProposeJmiElement@montague.example";

    QVERIFY(m_manager.handleProposeJmiElement(jmiElement, callPartnerJid));
    QCOMPARE(m_manager.jmis().size(), 1);
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleExistingJmi()
{
    QString callPartnerJid { "juliet@capulet.example" };
    QString jmiId { "989a46a6-f202-4910-a7c3-83c6ba3f3947" };

    auto jmi { m_manager.addJmi(callPartnerJid) };
    jmi->setId(jmiId);

    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setId(jmiId);

    // --- ringing ---

    QSignalSpy ringingSpy(jmi.get(), &QXmppJingleMessageInitiation::ringing);

    jmiElement.setType(JmiType::Ringing);

    QVERIFY(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid));
    QCOMPARE(ringingSpy.count(), 1);
    m_manager.clearAll();

    // --- proceeded ---

    jmi = m_manager.addJmi(callPartnerJid);
    jmi->setId(jmiId);

    jmiElement.setType(JmiType::Proceed);
    connect(jmi.get(), &QXmppJingleMessageInitiation::proceeded, this, [jmiElement](const QString &jmiElementId) {
        if (jmiElementId == jmiElement.id()) {
            QVERIFY(true);
        }
    });

    QVERIFY(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid));
    m_manager.clearAll();

    // --- closed: rejected ---

    jmi = m_manager.addJmi(callPartnerJid);
    jmi->setId(jmiId);

    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText("Rejected because expired.");

    jmiElement.setType(JmiType::Reject);
    jmiElement.setReason(reason);

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [jmiElement](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Rejected;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &rejectedJmiElement { std::get<ResultType>(result) };

        QVERIFY(rejectedJmiElement.reason);
        QCOMPARE(rejectedJmiElement.reason->type(), jmiElement.reason()->type());
        QCOMPARE(rejectedJmiElement.reason->text(), jmiElement.reason()->text());
        QCOMPARE(rejectedJmiElement.containsTieBreak, jmiElement.containsTieBreak());
    });

    QVERIFY(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid));
    m_manager.clearAll();

    // --- closed: retracted ---

    jmi = m_manager.addJmi(callPartnerJid);
    jmi->setId(jmiId);

    reason.setType(QXmppJingleReason::ConnectivityError);
    reason.setText("Retracted due to connectivity error.");

    jmiElement.setType(JmiType::Retract);
    jmiElement.setReason(reason);

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [jmiElement](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Retracted;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &rejectedJmiElement { std::get<ResultType>(result) };

        QVERIFY(rejectedJmiElement.reason);
        QCOMPARE(rejectedJmiElement.reason->type(), jmiElement.reason()->type());
        QCOMPARE(rejectedJmiElement.reason->text(), jmiElement.reason()->text());
        QCOMPARE(rejectedJmiElement.containsTieBreak, jmiElement.containsTieBreak());
    });

    QVERIFY(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid));
    m_manager.clearAll();

    // --- closed: finished ---

    jmi = m_manager.addJmi(callPartnerJid);
    jmi->setId(jmiId);

    reason.setType(QXmppJingleReason::Success);
    reason.setText("Finished.");

    jmiElement.setType(JmiType::Finish);
    jmiElement.setReason(reason);
    jmiElement.setMigratedTo("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [jmiElement](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Finished;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &rejectedJmiElement { std::get<ResultType>(result) };

        QVERIFY(rejectedJmiElement.reason);
        QCOMPARE(rejectedJmiElement.reason->type(), jmiElement.reason()->type());
        QCOMPARE(rejectedJmiElement.reason->text(), jmiElement.reason()->text());
        QCOMPARE(rejectedJmiElement.migratedTo, jmiElement.migratedTo());
    });

    QVERIFY(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid));
    m_manager.clearAll();

    // --- none ---

    jmi = m_manager.addJmi(callPartnerJid);
    jmi->setId(jmiId);

    jmiElement.setType(JmiType::None);

    QCOMPARE(m_manager.handleExistingJmi(jmi, jmiElement, callPartnerJid), false);
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleJmiElement()
{
    QString callPartnerJid { "romeoHandleJmiElement@montague.example/orchard" };
    QString jmiId { "ca3cf894-5325-482f-a412-a6e9f832298d" };

    // case 1: no JMI found in JMIs vector and jmiElement is not a propose element
    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setType(JmiType::None);

    QCOMPARE(m_manager.handleJmiElement(std::move(jmiElement), {}), false);

    // case 2: no JMI found in JMIs vector and jmiElement is a propose element
    jmiElement = {};
    jmiElement.setType(JmiType::Propose);
    jmiElement.setId(jmiId);

    QSignalSpy proposedSpy(&m_manager, &QXmppJingleMessageInitiationManager::proposed);
    QVERIFY(m_manager.handleJmiElement(std::move(jmiElement), callPartnerJid));
    QCOMPARE(proposedSpy.count(), 1);
    m_manager.clearAll();

    // case 3: JMI found in JMIs vector, existing session
    jmiElement = {};
    jmiElement.setType(JmiType::Ringing);
    jmiElement.setId(jmiId);
    auto jmi { m_manager.addJmi(QXmppUtils::jidToBareJid(callPartnerJid)) };
    jmi->setId(jmiId);

    QSignalSpy ringingSpy(jmi.get(), &QXmppJingleMessageInitiation::ringing);
    QVERIFY(m_manager.handleJmiElement(std::move(jmiElement), callPartnerJid));
    QCOMPARE(ringingSpy.count(), 1);
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessage_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("xmlValid")
        << QByteArray(
               "<message to='julietHandleMessageValid@capulet.example' from='romeoHandleMessageValid@montague.example/orchard' type='chat'>"
               "<store xmlns=\"urn:xmpp:hints\"/>"
               "<propose xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
               "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
               "</propose>"
               "</message>")
        << true;

    QTest::newRow("xmlInvalidTypeNotChat")
        << QByteArray(
               "<message to='julietHandleMessageNoChat@capulet.example' from='romeoHandleMessageNoChat@montague.example/orchard' type='normal'>"
               "<store xmlns=\"urn:xmpp:hints\"/>"
               "<propose xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
               "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
               "</propose>"
               "</message>")
        << false;

    QTest::newRow("xmlInvalidNoStore")
        << QByteArray(
               "<message to='julietHandleMessageNoStore@capulet.example' from='romeoHandleMessageNoStore@montague.example/orchard' type='chat'>"
               "<propose xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
               "<description xmlns='urn:xmpp:jingle:apps:rtp:1' media='audio'/>"
               "</propose>"
               "</message>")
        << false;

    QTest::newRow("xmlInvalidNoJmiElement")
        << QByteArray("<message to='julietHandleMessageNoJmi@capulet.example' from='romeoHandleMessageNoJmi@montague.example/orchard' type='chat'/>")
        << false;
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessage()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QXmppMessage message;

    parsePacket(message, xml);
    QCOMPARE(m_manager.handleMessage(message), isValid);
    serializePacket(message, xml);

    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessageRinging()
{
    QXmppMessage message;
    QByteArray xmlRinging {
        "<message from='juliet@capulet.example/phone' to='romeo@montague.example' type='chat'>"
        "<ringing xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'/>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("juliet@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    QSignalSpy ringingSpy(jmi.get(), &QXmppJingleMessageInitiation::ringing);

    message.parse(xmlToDom(xmlRinging));

    QVERIFY(m_manager.handleMessage(message));
    QCOMPARE(ringingSpy.count(), 1);
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessageProceeded()
{
    QXmppMessage message;
    QByteArray xmlProceed {
        "<message from='juliet@capulet.example/phone' to='romeo@montague.example' type='chat'>"
        "<proceed xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'/>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("juliet@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    QSignalSpy proceededSpy(jmi.get(), &QXmppJingleMessageInitiation::proceeded);

    message.parse(xmlToDom(xmlProceed));

    QVERIFY(m_manager.handleMessage(message));
    QCOMPARE(proceededSpy.count(), 1);
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessageClosedRejected()
{
    QXmppMessage message;
    QByteArray xmlReject {
        "<message from='juliet@capulet.example/phone' to='romeo@montague.example' type='chat'>"
        "<reject xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
        "<reason xmlns=\"urn:xmpp:jingle:1\">"
        "<busy/>"
        "<text>Busy</text>"
        "</reason>"
        "</reject>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("juliet@capulet.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Rejected;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &rejectedJmiElement { std::get<ResultType>(result) };

        QCOMPARE(rejectedJmiElement.reason->type(), QXmppJingleReason::Busy);
        QCOMPARE(rejectedJmiElement.reason->text(), "Busy");
    });

    message.parse(xmlToDom(xmlReject));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessageClosedRetracted()
{
    QXmppMessage message;
    QByteArray xmlRetract {
        "<message from='romeo@montague.example/orchard' to='juliet@capulet.example' type='chat'>"
        "<retract xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
        "<reason xmlns=\"urn:xmpp:jingle:1\">"
        "<cancel/>"
        "<text>Retracted</text>"
        "</reason>"
        "</retract>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("romeo@montague.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Retracted;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &retractedJmiElement { std::get<ResultType>(result) };

        QCOMPARE(retractedJmiElement.reason->type(), QXmppJingleReason::Cancel);
        QCOMPARE(retractedJmiElement.reason->text(), "Retracted");
    });

    message.parse(xmlToDom(xmlRetract));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

void tst_QXmppJingleMessageInitiationManager::testHandleMessageClosedFinished()
{
    QXmppMessage message;
    QByteArray xmlFinish {
        "<message from='romeo@montague.example/orchard' to='juliet@capulet.example' type='chat'>"
        "<finish xmlns='urn:xmpp:jingle-message:0' id='ca3cf894-5325-482f-a412-a6e9f832298d'>"
        "<reason xmlns=\"urn:xmpp:jingle:1\">"
        "<success/>"
        "<text>Success</text>"
        "</reason>"
        "<migrated to='989a46a6-f202-4910-a7c3-83c6ba3f3947'/>"
        "</finish>"
        "<store xmlns=\"urn:xmpp:hints\"/>"
        "</message>"
    };

    auto jmi { m_manager.addJmi("romeo@montague.example") };
    jmi->setId("ca3cf894-5325-482f-a412-a6e9f832298d");

    connect(jmi.get(), &QXmppJingleMessageInitiation::closed, this, [](const Result &result) {
        using ResultType = QXmppJingleMessageInitiation::Finished;

        QVERIFY(std::holds_alternative<ResultType>(result));
        const ResultType &finishedJmiElement { std::get<ResultType>(result) };

        QCOMPARE(finishedJmiElement.reason->type(), QXmppJingleReason::Success);
        QCOMPARE(finishedJmiElement.reason->text(), "Success");
        QCOMPARE(finishedJmiElement.migratedTo, "989a46a6-f202-4910-a7c3-83c6ba3f3947");
    });

    message.parse(xmlToDom(xmlFinish));

    QVERIFY(m_manager.handleMessage(message));
    m_manager.clearAll();
}

QTEST_MAIN(tst_QXmppJingleMessageInitiationManager)
#include "tst_qxmppjinglemessageinitiationmanager.moc"
