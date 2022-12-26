// SPDX-FileCopyrightText: 2016 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManager.h"
#include "QXmppCarbonManagerV2.h"
#include "QXmppClient.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppMessage.h"
#include "QXmppMessageHandler.h"

#include "util.h"
#include <QObject>

void compareMessages(const QXmppMessage &lhs, const QXmppMessage &rhs)
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
    QCOMPARE(lhs.isCarbonForwarded(), rhs.isCarbonForwarded());
}

class QXmppCarbonTestHelper : public QObject
{
    Q_OBJECT

public:
    Q_SLOT void messageSent(const QXmppMessage &msg)
    {
        m_signalTriggered = true;
        QCOMPARE(m_expectSent, true);

        compareMessages(m_expectedMessage, msg);
    }
    Q_SLOT void messageReceived(const QXmppMessage &msg)
    {
        m_signalTriggered = true;
        QCOMPARE(m_expectSent, false);

        compareMessages(m_expectedMessage, msg);
    }

    QXmppMessage m_expectedMessage;
    bool m_expectSent;
    bool m_signalTriggered;
};

class MessageHandler : public QXmppClientExtension, public QXmppMessageHandler
{
public:
    bool handleMessage(const QXmppMessage &msg) override
    {
        received.push_back(msg);
        return false;
    }

    std::vector<QXmppMessage> received;
};

class tst_QXmppCarbonManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testHandleStanza_data();
    Q_SLOT void testHandleStanza();

    QXmppCarbonTestHelper m_helper;
    MessageHandler *m_messageHandler;
    QXmppCarbonManager *m_managerV1;
    QXmppCarbonManagerV2 *m_managerV2;
    QXmppClient client;
};

void tst_QXmppCarbonManager::initTestCase()
{
    client.configuration().setJid("romeo@montague.example");
    m_managerV1 = client.addNewExtension<QXmppCarbonManager>();
    m_managerV2 = client.addNewExtension<QXmppCarbonManagerV2>();
    m_messageHandler = client.addNewExtension<MessageHandler>();

    connect(m_managerV1, &QXmppCarbonManager::messageSent,
            &m_helper, &QXmppCarbonTestHelper::messageSent);

    connect(m_managerV1, &QXmppCarbonManager::messageReceived,
            &m_helper, &QXmppCarbonTestHelper::messageReceived);
}

void tst_QXmppCarbonManager::testHandleStanza_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<bool>("sent");
    QTest::addColumn<QByteArray>("forwardedxml");

    QTest::newRow("received1")
        << QByteArray("<message xmlns='jabber:client'"
                      " from='romeo@montague.example'"
                      " to='romeo@montague.example/home'"
                      " type='chat'>"
                      "<received xmlns='urn:xmpp:carbons:2'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<message xmlns='jabber:client'"
                      " from='juliet@capulet.example/balcony'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>"
                      "</forwarded>"
                      "</received>"
                      "</message>")
        << true << false
        << QByteArray("<message xmlns='jabber:client'"
                      " from='juliet@capulet.example/balcony'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>");

    QTest::newRow("sent1")
        << QByteArray("<message xmlns='jabber:client'"
                      " from='romeo@montague.example'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<sent xmlns='urn:xmpp:carbons:2'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<message xmlns='jabber:client'"
                      " to='juliet@capulet.example/balcony'"
                      " from='romeo@montague.example/home'"
                      " type='chat'>"
                      "<body>Neither, fair saint, if either thee dislike.</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>"
                      "</forwarded>"
                      "</sent>"
                      "</message>")
        << true << true
        << QByteArray("<message xmlns='jabber:client'"
                      " to='juliet@capulet.example/balcony'"
                      " from='romeo@montague.example/home'"
                      " type='chat'>"
                      "<body>Neither, fair saint, if either thee dislike.</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>");

    QTest::newRow("received-wrong-from")
        << QByteArray("<message xmlns='jabber:client'"
                      " from='not-romeo@montague.example'"
                      " to='romeo@montague.example/home'"
                      " type='chat'>"
                      "<received xmlns='urn:xmpp:carbons:2'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<message xmlns='jabber:client'"
                      " from='juliet@capulet.example/balcony'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>"
                      "</forwarded>"
                      "</received>"
                      "</message>")
        << false << false
        << QByteArray("<message xmlns='jabber:client'"
                      " from='juliet@capulet.example/balcony'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>");

    QTest::newRow("sent-wrong-from")
        << QByteArray("<message xmlns='jabber:client'"
                      " from='not-romeo@montague.example'"
                      " to='romeo@montague.example/garden'"
                      " type='chat'>"
                      "<sent xmlns='urn:xmpp:carbons:2'>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<message xmlns='jabber:client'"
                      " to='juliet@capulet.example/balcony'"
                      " from='romeo@montague.example/home'"
                      " type='chat'>"
                      "<body>Neither, fair saint, if either thee dislike.</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>"
                      "</forwarded>"
                      "</sent>"
                      "</message>")
        << false << true
        << QByteArray("<message xmlns='jabber:client'"
                      " to='juliet@capulet.example/balcony'"
                      " from='romeo@montague.example/home'"
                      " type='chat'>"
                      "<body>Neither, fair saint, if either thee dislike.</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>");

    QTest::newRow("forwarded_normal")
        << QByteArray("<message to='mercutio@verona.lit' from='romeo@montague.lit/orchard' type='chat' id='28gs'>"
                      "<body>A most courteous exposition!</body>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                      "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                      "<message from='juliet@capulet.lit/orchard'"
                      " id='0202197'"
                      " to='romeo@montague.lit'"
                      " type='chat'"
                      " xmlns='jabber:client'>"
                      "<body>Yet I should kill thee with much cherishing.</body>"
                      "<mood xmlns='http://jabber.org/protocol/mood'>"
                      "<amorous/>"
                      "</mood>"
                      "</message>"
                      "</forwarded>"
                      "</message>")
        << false << false
        << QByteArray();
}

void tst_QXmppCarbonManager::testHandleStanza()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);
    QFETCH(bool, sent);
    QFETCH(QByteArray, forwardedxml);

    QXmppMessage expectedMessage;
    if (!forwardedxml.isEmpty()) {
        parsePacket(expectedMessage, forwardedxml);
    }
    expectedMessage.setCarbonForwarded(true);

    {
        m_helper.m_expectedMessage = expectedMessage;
        m_helper.m_expectSent = sent;
        m_helper.m_signalTriggered = false;

        QDomDocument doc;
        QVERIFY(doc.setContent(xml, true));
        QDomElement element = doc.documentElement();

        bool accepted = m_managerV1->handleStanza(element);

        QCOMPARE(accepted, accept);
        QCOMPARE(m_helper.m_signalTriggered, accept);
    }
    {
        m_messageHandler->received.clear();

        bool accepted = m_managerV2->handleStanza(xmlToDom(xml), {});
        QCOMPARE(accepted, accept);

        if (accept) {
            QCOMPARE(m_messageHandler->received.size(), size_t(1));
            compareMessages(m_messageHandler->received[0], expectedMessage);
        } else {
            QCOMPARE(m_messageHandler->received.size(), size_t(0));
        }
    }
}

QTEST_MAIN(tst_QXmppCarbonManager)
#include "tst_qxmppcarbonmanager.moc"
