// SPDX-FileCopyrightText: 2016 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppMessage.h"

#include "util.h"
#include <QObject>

class QXmppCarbonTestHelper : public QObject
{
    Q_OBJECT

public slots:
    void messageSent(const QXmppMessage &msg);
    void messageReceived(const QXmppMessage &msg);

public:
    QXmppMessage m_expectedMessage;
    bool m_expectSent;
    bool m_signalTriggered;

    void compareMessages(const QXmppMessage &lhs, const QXmppMessage &rhs);
};

class tst_QXmppCarbonManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testHandleStanza_data();
    void testHandleStanza();

private:
    QXmppCarbonTestHelper m_helper;
    QXmppCarbonManager *m_manager;
    QXmppClient client;
};

void tst_QXmppCarbonManager::initTestCase()
{
    m_manager = new QXmppCarbonManager();

    connect(m_manager, &QXmppCarbonManager::messageSent,
            &m_helper, &QXmppCarbonTestHelper::messageSent);

    connect(m_manager, &QXmppCarbonManager::messageReceived,
            &m_helper, &QXmppCarbonTestHelper::messageReceived);

    client.connectToServer("romeo@montague.example", "a");
    client.disconnectFromServer();

    client.addExtension(m_manager);
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

    m_helper.m_expectedMessage = QXmppMessage();

    if (!forwardedxml.isEmpty())
        parsePacket(m_helper.m_expectedMessage, forwardedxml);

    m_helper.m_expectSent = sent;
    m_helper.m_signalTriggered = false;

    QDomDocument doc;
    QVERIFY(doc.setContent(xml, true));
    QDomElement element = doc.documentElement();

    bool accepted = m_manager->handleStanza(element, {});

    QCOMPARE(accepted, accept);
    QCOMPARE(m_helper.m_signalTriggered, accept);
}

void QXmppCarbonTestHelper::messageSent(const QXmppMessage &msg)
{
    m_signalTriggered = true;
    QCOMPARE(m_expectSent, true);

    compareMessages(m_expectedMessage, msg);
}

void QXmppCarbonTestHelper::messageReceived(const QXmppMessage &msg)
{
    m_signalTriggered = true;
    QCOMPARE(m_expectSent, false);

    compareMessages(m_expectedMessage, msg);
}

void QXmppCarbonTestHelper::compareMessages(const QXmppMessage &lhs, const QXmppMessage &rhs)
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

QTEST_MAIN(tst_QXmppCarbonManager)
#include "tst_qxmppcarbonmanager.moc"
