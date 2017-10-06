/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QObject>
#include "QXmppMessage.h"
#include "QXmppCarbonManager.h"
#include "util.h"

class QXmppCarbonTestHelper  : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void messageSent(const QXmppMessage& msg);
    void messageReceived(const QXmppMessage& msg);

public:
    QXmppMessage m_expectedMessage;
    bool m_expectSent;
    bool m_signalTriggered;

    void compareMessages(const QXmppMessage& lhs, const QXmppMessage& rhs);
};

class tst_QXmppCarbonManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testHandleStanza_data();
    void testHandleStanza();



private:
    QXmppCarbonTestHelper m_helper;
    QXmppCarbonManager m_manager;
};

void tst_QXmppCarbonManager::initTestCase()
{
    connect(&m_manager, SIGNAL(messageSent(const QXmppMessage&)),
            &m_helper, SLOT(messageSent(const QXmppMessage&)));

    connect(&m_manager, SIGNAL(messageReceived(const QXmppMessage&)),
            &m_helper, SLOT(messageReceived(const QXmppMessage&)));
}

void tst_QXmppCarbonManager::testHandleStanza_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<bool>("sent");
    QTest::addColumn<QByteArray>("forwardedxml");

    QTest::newRow("received1")
        << QByteArray("<message xmlns='jabber:client'"
                      "from='romeo@montague.example'"
                      "to='romeo@montague.example/home'"
                      "type='chat'>"
               "<received xmlns='urn:xmpp:carbons:2'>"
                 "<forwarded xmlns='urn:xmpp:forward:0'>"
                   "<message xmlns='jabber:client'"
                            "from='juliet@capulet.example/balcony'"
                            "to='romeo@montague.example/garden'"
                            "type='chat'>"
                     "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                     "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                   "</message>"
                 "</forwarded>"
               "</received>"
             "</message>")
        << true << false
        << QByteArray("<message xmlns='jabber:client'"
                            "from='juliet@capulet.example/balcony'"
                            "to='romeo@montague.example/garden'"
                            "type='chat'>"
                        "<body>What man art thou that, thus bescreen'd in night, so stumblest on my counsel?</body>"
                        "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                      "</message>");

    QTest::newRow("sent1")
        << QByteArray("<message xmlns='jabber:client'"
                      "from='romeo@montague.example'"
                      "to='romeo@montague.example/garden'"
                      "type='chat'>"
                "<sent xmlns='urn:xmpp:carbons:2'>"
                  "<forwarded xmlns='urn:xmpp:forward:0'>"
                    "<message xmlns='jabber:client'"
                             "to='juliet@capulet.example/balcony'"
                             "from='romeo@montague.example/home'"
                             "type='chat'>"
                      "<body>Neither, fair saint, if either thee dislike.</body>"
                      "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                    "</message>"
                  "</forwarded>"
                "</sent>"
              "</message>")
        << true << true
        << QByteArray("<message xmlns='jabber:client'"
                          "to='juliet@capulet.example/balcony'"
                          "from='romeo@montague.example/home'"
                          "type='chat'>"
                   "<body>Neither, fair saint, if either thee dislike.</body>"
                   "<thread>0e3141cd80894871a68e6fe6b1ec56fa</thread>"
                 "</message>");


    QTest::newRow("forwarded_normal")
        << QByteArray("<message to='mercutio@verona.lit' from='romeo@montague.lit/orchard' type='chat' id='28gs'>"
                      "<body>A most courteous exposition!</body>"
                      "<forwarded xmlns='urn:xmpp:forward:0'>"
                        "<delay xmlns='urn:xmpp:delay' stamp='2010-07-10T23:08:25Z'/>"
                        "<message from='juliet@capulet.lit/orchard'"
                                 "id='0202197'"
                                 "to='romeo@montague.lit'"
                                 "type='chat'"
                                 "xmlns='jabber:client'>"
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

    if(!forwardedxml.isEmpty())
        parsePacket(m_helper.m_expectedMessage, forwardedxml);

    m_helper.m_expectSent = sent;
    m_helper.m_signalTriggered = false;

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    bool accepted = m_manager.handleStanza(element);

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
