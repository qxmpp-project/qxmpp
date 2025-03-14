// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppE2eeExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMamManager.h"
#include "QXmppMessage.h"

#include "TestClient.h"
#include "util.h"

#include <QObject>

using namespace QXmpp::Private;

class EncryptionExtension : public QXmppE2eeExtension
{
public:
    QXmppTask<MessageEncryptResult> encryptMessage(QXmppMessage &&, const std::optional<QXmppSendStanzaParams> &) override
    {
        return makeReadyTask<MessageEncryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }
    QXmppTask<MessageDecryptResult> decryptMessage(QXmppMessage &&m) override
    {
        m.setBody(m.e2eeFallbackBody());
        m.setE2eeFallbackBody({});
        return makeReadyTask<MessageDecryptResult>(std::move(m));
    }

    QXmppTask<IqEncryptResult> encryptIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> &) override
    {
        return makeReadyTask<IqEncryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    QXmppTask<IqDecryptResult> decryptIq(const QDomElement &) override
    {
        return makeReadyTask<IqDecryptResult>(QXmppError { "it's only a test", QXmpp::SendError::EncryptionError });
    }

    bool isEncrypted(const QDomElement &e) override { return !e.firstChildElement("test-encrypted").isNull(); };
    bool isEncrypted(const QXmppMessage &) override { return false; };
};

class QXmppMamTestHelper : public QObject
{
    Q_OBJECT

public:
    Q_SLOT void archivedMessageReceived(const QString &queryId, const QXmppMessage &message);
    Q_SLOT void resultsRecieved(const QString &queryId, const QXmppResultSetReply &resultSetReply, bool complete);

    QXmppMessage m_expectedMessage;
    QXmppResultSetReply m_expectedResultSetReply;
    QString m_expectedQueryId;
    bool m_expectedComplete;
    bool m_signalTriggered;

    void compareMessages(const QXmppMessage &lhs, const QXmppMessage &rhs) const;
    void compareResultSetReplys(const QXmppResultSetReply &lhs, const QXmppResultSetReply &rhs) const;
};

class tst_QXmppMamManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testHandleStanza_data();
    Q_SLOT void testHandleStanza();

    Q_SLOT void testHandleResultIq_data();
    Q_SLOT void testHandleResultIq();

    // test for task-based API
    Q_SLOT void retrieveMessagesUnencrypted();
    Q_SLOT void retrieveMessagesEncrypted();

    QXmppMamTestHelper m_helper;
    QXmppMamManager m_manager;
};

void tst_QXmppMamManager::initTestCase()
{
    connect(&m_manager, &QXmppMamManager::archivedMessageReceived,
            &m_helper, &QXmppMamTestHelper::archivedMessageReceived);

    connect(&m_manager, &QXmppMamManager::resultsRecieved,
            &m_helper, &QXmppMamTestHelper::resultsRecieved);
}

void tst_QXmppMamManager::testHandleStanza_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<QByteArray>("expectedMessage");
    QTest::addColumn<QString>("expectedQueryId");

    QTest::newRow("stanza1")
        << QByteArray("<message id='aeb213' to='juliet@capulet.lit/chamber'>"
                      "<result xmlns='urn:xmpp:mam:2' queryid='f27' id='28482-98726-73623'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                      "<message xmlns='jabber:client'"
                      " to='juliet@capulet.lit/balcony'"
                      " from='romeo@montague.lit/orchard'"
                      " type='chat'>"
                      "<body>Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.</body>"
                      "</message>"
                      "</forwarded>"
                      "</result>"
                      "</message>")
        << true
        << QByteArray("<message xmlns='jabber:client'"
                      " to='juliet@capulet.lit/balcony'"
                      " from='romeo@montague.lit/orchard'"
                      " type='chat'>"
                      "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                      "<body>Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.</body>"
                      "</message>")
        << u"f27"_s;

    QTest::newRow("stanza2")
        << QByteArray("<message id='aeb214' to='juliet@capulet.lit/chamber'>"
                      "<result queryid='f27' id='5d398-28273-f7382'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:09:32Z'/>"
                      "<message xmlns='jabber:client'"
                      " to='romeo@montague.lit/orchard'"
                      " from='juliet@capulet.lit/balcony'"
                      " type='chat' id='8a54s'>"
                      "<body>What man art thou that thus bescreen'd in night so stumblest on my counsel?</body>"
                      "</message>"
                      "</forwarded>"
                      "</result>"
                      "</message>")
        << false
        << QByteArray()
        << QString();

    QTest::newRow("stanza3")
        << QByteArray(
               "<message id='aeb214' xmlns='urn:xmpp:mam:2' to='juliet@capulet.lit/chamber'>"
               "<forwarded xmlns='urn:xmpp:forward:0'>"
               "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
               "<message xmlns='jabber:client'"
               " to='juliet@capulet.lit/balcony'"
               " from='romeo@montague.lit/orchard'"
               " type='chat'>"
               "<body>Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.</body>"
               "</message>"
               "</forwarded>"
               "</message>")
        << false
        << QByteArray()
        << QString();
}

void tst_QXmppMamManager::testHandleStanza()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);
    QFETCH(QByteArray, expectedMessage);
    QFETCH(QString, expectedQueryId);

    m_helper.m_signalTriggered = false;
    m_helper.m_expectedMessage = QXmppMessage();
    if (!expectedMessage.isEmpty()) {
        parsePacket(m_helper.m_expectedMessage, expectedMessage);
    }
    m_helper.m_expectedQueryId = expectedQueryId;

    bool accepted = m_manager.handleStanza(xmlToDom(xml));
    QCOMPARE(accepted, accept);
    QCOMPARE(m_helper.m_signalTriggered, accept);
}

void tst_QXmppMamManager::testHandleResultIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<QByteArray>("expectedResultSetReply");
    QTest::addColumn<bool>("expectedComplete");

    QTest::newRow("stanza1")
        << QByteArray("<iq type='result' id='juliet1'>"
                      "<fin xmlns='urn:xmpp:mam:2'>"
                      "<set xmlns='http://jabber.org/protocol/rsm'>"
                      "<first index='0'>28482-98726-73623</first>"
                      "<last>09af3-cc343-b409f</last>"
                      "</set>"
                      "</fin>"
                      "</iq>")
        << true
        << QByteArray("<set xmlns='http://jabber.org/protocol/rsm'>"
                      "<first index='0'>28482-98726-73623</first>"
                      "<last>09af3-cc343-b409f</last>"
                      "</set>")
        << false;

    QTest::newRow("stanza2")
        << QByteArray("<iq type='result' id='juliet1'>"
                      "<fin xmlns='urn:xmpp:mam:2' complete='true'>"
                      "<set xmlns='http://jabber.org/protocol/rsm'>"
                      "<first index='0'>28482-98726-73623</first>"
                      "<last>09af3-cc343-b409f</last>"
                      "</set>"
                      "</fin>"
                      "</iq>")
        << true
        << QByteArray("<set xmlns='http://jabber.org/protocol/rsm'>"
                      "<first index='0'>28482-98726-73623</first>"
                      "<last>09af3-cc343-b409f</last>"
                      "</set>")
        << true;
}

void tst_QXmppMamManager::testHandleResultIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);
    QFETCH(QByteArray, expectedResultSetReply);
    QFETCH(bool, expectedComplete);

    m_helper.m_signalTriggered = false;
    m_helper.m_expectedResultSetReply = QXmppResultSetReply();
    if (!expectedResultSetReply.isEmpty()) {
        parsePacket(m_helper.m_expectedResultSetReply, expectedResultSetReply);
    }
    m_helper.m_expectedComplete = expectedComplete;

    bool accepted = m_manager.handleStanza(xmlToDom(xml));
    QCOMPARE(accepted, accept);
    QCOMPARE(m_helper.m_signalTriggered, accept);
}

void tst_QXmppMamManager::retrieveMessagesUnencrypted()
{
    TestClient test;
    auto *mam = test.addNewExtension<QXmppMamManager>();
    auto task = mam->retrieveMessages("mam.server.org");
    test.expect("<iq id='qxmpp1' to='mam.server.org' type='set'>"
                "<query xmlns='urn:xmpp:mam:2' queryid='qxmpp1'>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>urn:xmpp:mam:2</value></field>"
                "</x>"
                "</query>"
                "</iq>");
    mam->handleStanza(xmlToDom("<message id='aeb213' to='juliet@capulet.lit/chamber' from='mam.server.org'>"
                               "<result xmlns='urn:xmpp:mam:2' queryid='qxmpp1' id='28482-98726-73623'>"
                               "<forwarded xmlns='urn:xmpp:forward:0'>"
                               "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                               "<message xmlns='jabber:client'"
                               " to='juliet@capulet.lit/balcony'"
                               " from='romeo@montague.lit/orchard'"
                               " type='chat'>"
                               "<body>Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.</body>"
                               "</message>"
                               "</forwarded>"
                               "</result>"
                               "</message>"));
    mam->handleStanza(xmlToDom("<message id='aeb214' to='juliet@capulet.lit/chamber' from='mam.server.org'>"
                               "<result xmlns='urn:xmpp:mam:2' queryid='qxmpp1' id='5d398-28273-f7382'>"
                               "<forwarded xmlns='urn:xmpp:forward:0'>"
                               "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:09:32Z'/>"
                               "<message xmlns='jabber:client'"
                               " to='romeo@montague.lit/orchard'"
                               " from='juliet@capulet.lit/balcony'"
                               " type='chat' id='8a54s'>"
                               "<body>What man art thou that thus bescreen'd in night so stumblest on my counsel?</body>"
                               "</message>"
                               "</forwarded>"
                               "</result>"
                               "</message>"));
    test.inject("<iq type='result' id='qxmpp1'>"
                "<fin xmlns='urn:xmpp:mam:2'>"
                "<set xmlns='http://jabber.org/protocol/rsm'>"
                "<first index='0'>28482-98726-73623</first>"
                "<last>09af3-cc343-b409f</last>"
                "</set>"
                "</fin>"
                "</iq>");

    auto retrieved = expectFutureVariant<QXmppMamManager::RetrievedMessages>(task);
    QCOMPARE(retrieved.messages.size(), 2);
    QCOMPARE(retrieved.messages.at(0).body(), "Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.");
    QCOMPARE(retrieved.messages.at(1).body(), "What man art thou that thus bescreen'd in night so stumblest on my counsel?");
    QCOMPARE(retrieved.result.resultSetReply().first(), "28482-98726-73623");
}

void tst_QXmppMamManager::retrieveMessagesEncrypted()
{
    TestClient test;
    // e2ee
    auto e2ee = std::make_unique<EncryptionExtension>();
    test.setEncryptionExtension(e2ee.get());
    // mam manager
    auto *mam = test.addNewExtension<QXmppMamManager>();

    // start request
    auto task = mam->retrieveMessages("mam.server.org");
    test.expect("<iq id='qxmpp1' to='mam.server.org' type='set'>"
                "<query xmlns='urn:xmpp:mam:2' queryid='qxmpp1'>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>urn:xmpp:mam:2</value></field>"
                "</x>"
                "</query>"
                "</iq>");
    mam->handleStanza(xmlToDom("<message id='aeb213' to='juliet@capulet.lit/chamber' from='mam.server.org'>"
                               "<result xmlns='urn:xmpp:mam:2' queryid='qxmpp1' id='28482-98726-73623'>"
                               "<forwarded xmlns='urn:xmpp:forward:0'>"
                               "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                               "<message xmlns='jabber:client'"
                               " to='juliet@capulet.lit/balcony'"
                               " from='romeo@montague.lit/orchard'"
                               " type='chat'>"
                               "<test-encrypted/>"
                               "<body>Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.</body>"
                               "</message>"
                               "</forwarded>"
                               "</result>"
                               "</message>"));
    mam->handleStanza(xmlToDom("<message id='aeb214' to='juliet@capulet.lit/chamber' from='mam.server.org'>"
                               "<result xmlns='urn:xmpp:mam:2' queryid='qxmpp1' id='5d398-28273-f7382'>"
                               "<forwarded xmlns='urn:xmpp:forward:0'>"
                               "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:09:32Z'/>"
                               "<message xmlns='jabber:client'"
                               " to='romeo@montague.lit/orchard'"
                               " from='juliet@capulet.lit/balcony'"
                               " type='chat' id='8a54s'>"
                               "<body>What man art thou that thus bescreen'd in night so stumblest on my counsel?</body>"
                               "</message>"
                               "</forwarded>"
                               "</result>"
                               "</message>"));
    test.inject("<iq type='result' id='qxmpp1'>"
                "<fin xmlns='urn:xmpp:mam:2'>"
                "<set xmlns='http://jabber.org/protocol/rsm'>"
                "<first index='0'>28482-98726-73623</first>"
                "<last>09af3-cc343-b409f</last>"
                "</set>"
                "</fin>"
                "</iq>");

    // check results
    auto retrieved = expectFutureVariant<QXmppMamManager::RetrievedMessages>(task);
    QCOMPARE(retrieved.messages.size(), 2);
    QCOMPARE(retrieved.messages.at(0).body(), "Call me but love, and I'll be new baptized; Henceforth I never will be Romeo.");
    QCOMPARE(retrieved.messages.at(1).body(), "What man art thou that thus bescreen'd in night so stumblest on my counsel?");
    QCOMPARE(retrieved.result.resultSetReply().first(), "28482-98726-73623");
}

void QXmppMamTestHelper::archivedMessageReceived(const QString &queryId, const QXmppMessage &message)
{
    m_signalTriggered = true;

    compareMessages(message, m_expectedMessage);
    QCOMPARE(queryId, m_expectedQueryId);
}

void QXmppMamTestHelper::resultsRecieved(const QString &queryId, const QXmppResultSetReply &resultSetReply, bool complete)
{
    Q_UNUSED(queryId);
    m_signalTriggered = true;

    compareResultSetReplys(resultSetReply, m_expectedResultSetReply);
    QCOMPARE(complete, m_expectedComplete);
}

void QXmppMamTestHelper::compareMessages(const QXmppMessage &lhs, const QXmppMessage &rhs) const
{
    QCOMPARE(lhs.body(), rhs.body());
    QCOMPARE(lhs.from(), rhs.from());
    QCOMPARE(lhs.id(), rhs.id());
    QCOMPARE(lhs.isAttentionRequested(), rhs.isAttentionRequested());
    QCOMPARE(lhs.isMarkable(), rhs.isMarkable());
    QCOMPARE(lhs.isPrivate(), rhs.isPrivate());
    QCOMPARE(lhs.isReceiptRequested(), rhs.isReceiptRequested());
    QCOMPARE(lhs.lang(), rhs.lang());
    QCOMPARE(lhs.to(), rhs.to());
    QCOMPARE(lhs.thread(), rhs.thread());
    QCOMPARE(lhs.stamp(), rhs.stamp());
    QCOMPARE(lhs.type(), rhs.type());
}

void QXmppMamTestHelper::compareResultSetReplys(const QXmppResultSetReply &lhs, const QXmppResultSetReply &rhs) const
{
    QCOMPARE(lhs.first(), rhs.first());
    QCOMPARE(lhs.last(), rhs.last());
    QCOMPARE(lhs.count(), rhs.count());
    QCOMPARE(lhs.index(), rhs.index());
    QCOMPARE(lhs.isNull(), rhs.isNull());
}

QTEST_MAIN(tst_QXmppMamManager)
#include "tst_qxmppmammanager.moc"
