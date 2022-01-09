/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Authors:
 *  Niels Ole Salscheider
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

#include "QXmppMamManager.h"
#include "QXmppMessage.h"

#include "util.h"
#include <QObject>

class QXmppMamTestHelper : public QObject
{
    Q_OBJECT

public slots:
    void archivedMessageReceived(const QString &queryId, const QXmppMessage &message);
    void resultsRecieved(const QString &queryId, const QXmppResultSetReply &resultSetReply, bool complete);

public:
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

private slots:
    void initTestCase();

    void testHandleStanza_data();
    void testHandleStanza();

    void testHandleResultIq_data();
    void testHandleResultIq();

private:
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
        << QString("f27");

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

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    m_helper.m_signalTriggered = false;
    m_helper.m_expectedMessage = QXmppMessage();
    if (!expectedMessage.isEmpty()) {
        parsePacket(m_helper.m_expectedMessage, expectedMessage);
    }
    m_helper.m_expectedQueryId = expectedQueryId;

    bool accepted = m_manager.handleStanza(element);
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

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    m_helper.m_signalTriggered = false;
    m_helper.m_expectedResultSetReply = QXmppResultSetReply();
    if (!expectedResultSetReply.isEmpty()) {
        parsePacket(m_helper.m_expectedResultSetReply, expectedResultSetReply);
    }
    m_helper.m_expectedComplete = expectedComplete;

    bool accepted = m_manager.handleStanza(element);
    QCOMPARE(accepted, accept);
    QCOMPARE(m_helper.m_signalTriggered, accept);
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
