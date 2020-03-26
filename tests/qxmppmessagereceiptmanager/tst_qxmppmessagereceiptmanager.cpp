/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn <lnj@kaidan.im>
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
#include "QXmppMessageReceiptManager.h"

#include "util.h"
#include <QObject>

class tst_QXmppMessageReceiptManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testReceipt_data();
    void testReceipt();

    void handleMessageDelivered(const QString&, const QString&)
    {
        m_messageDelivered = true;
    }
    void onLoggerMessage(QXmppLogger::MessageType type, const QString& text) 
    {
        m_receiptSent = true;
    }

private:
    QXmppMessageReceiptManager* m_manager;
    QXmppClient m_client;
    QXmppLogger* m_logger;
    bool m_messageDelivered = false;
    bool m_receiptSent = false;
};

void tst_QXmppMessageReceiptManager::initTestCase()
{
    m_manager = new QXmppMessageReceiptManager();
    m_logger = new QXmppLogger();
    
    m_client.addExtension(m_manager);
    m_logger->setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(m_logger);
    
    connect(m_logger, &QXmppLogger::message, 
            this, &tst_QXmppMessageReceiptManager::onLoggerMessage);
    
    connect(m_manager, &QXmppMessageReceiptManager::messageDelivered,
            this, &tst_QXmppMessageReceiptManager::handleMessageDelivered);
}

void tst_QXmppMessageReceiptManager::testReceipt_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<bool>("sent");
    QTest::addColumn<bool>("handled");

    QTest::newRow("correct")
        << QByteArray(
               "<message id=\"bi29sg183b4v\" "
               "to=\"northumberland@shakespeare.lit/westminster\" "
               "from=\"kingrichard@royalty.england.lit/throne\" "
               "type=\"normal\">"
               "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
               "</message>")
        << true
        << false
        << true;
    QTest::newRow("from-to-equal")
        << QByteArray(
               "<message id=\"bi29sg183b4v\" "
               "to=\"kingrichard@royalty.england.lit/westminster\" "
               "from=\"kingrichard@royalty.england.lit/throne\" "
               "type=\"normal\">"
               "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
               "</message>")
        << false
        << false
        << true;
    QTest::newRow("error")
        << QByteArray(
            "<message xml:lang=\"en\" "
                "to=\"northumberland@shakespeare.lit/westminster\" "
                "from=\"kingrichard@royalty.england.lit/throne\" "
                "type=\"error\" id=\"bi29sg183b4v\" "
            "> "
                "<archived xmlns=\"urn:xmpp:mam:tmp\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<delay xmlns=\"urn:xmpp:delay\" stamp=\"2020-03-26T20:30:41.678Z\"/> "
                "<request xmlns=\"urn:xmpp:receipts\"/> "
                "<error code=\"500\" type=\"wait\"> "
                    "<resource-constraint xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/> "
                    "<text xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\" xml:lang=\"en\">"
                        "Your contact offline message queue is full. The message has been discarded."
                    "</text>"
                "</error>"
                "<body>1</body> "
            "</message>")
        << false
        << false
        << false;
    QTest::newRow("message with receipt request")
        << QByteArray(
            "<message xml:lang=\"en\" "
                "to=\"northumberland@shakespeare.lit/westminster\" "
                "from=\"kingrichard@royalty.england.lit/throne\" "
                "type=\"chat\" id=\"bi29sg183b4v\" "
            "> "
                "<archived xmlns=\"urn:xmpp:mam:tmp\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<request xmlns=\"urn:xmpp:receipts\"/> "
                "<body>1</body> "
            "</message>")
        << false
        << true
        << false;
        
    QTest::newRow("message with no receipt request")
        << QByteArray(
            "<message xml:lang=\"en\" "
                "to=\"northumberland@shakespeare.lit/westminster\" "
                "from=\"kingrichard@royalty.england.lit/throne\" "
                "type=\"chat\" id=\"bi29sg183b4v\" "
            "> "
                "<archived xmlns=\"urn:xmpp:mam:tmp\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<stanza-id xmlns=\"urn:xmpp:sid:0\" by=\"kingrichard@royalty.england.lit\" id=\"1585254642941569\"/> "
                "<body>1</body> "
            "</message>")
        << false
        << false
        << false;
}

void tst_QXmppMessageReceiptManager::testReceipt()
{
    m_messageDelivered = false;
    m_receiptSent = false;

    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);
    QFETCH(bool, sent);
    QFETCH(bool, handled);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    QCOMPARE(m_manager->handleStanza(element), handled);
    QCOMPARE(m_messageDelivered, accept);
    QCOMPARE(m_receiptSent, sent);
}

QTEST_MAIN(tst_QXmppMessageReceiptManager)
#include "tst_qxmppmessagereceiptmanager.moc"
