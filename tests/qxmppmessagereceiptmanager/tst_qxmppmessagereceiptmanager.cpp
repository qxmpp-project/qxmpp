// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppMessageReceiptManager.h"

#include "util.h"
#include <QObject>

class tst_QXmppMessageReceiptManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void initTestCase();

    Q_SLOT void testReceipt_data();
    Q_SLOT void testReceipt();

    void handleMessageDelivered(const QString &, const QString &)
    {
        m_messageDelivered = true;
    }
    void onLoggerMessage(QXmppLogger::MessageType, const QString &)
    {
        m_receiptSent = true;
    }

    QXmppMessageReceiptManager *m_manager;
    QXmppClient m_client;
    QXmppLogger m_logger;
    bool m_messageDelivered = false;
    bool m_receiptSent = false;
};

void tst_QXmppMessageReceiptManager::initTestCase()
{
    m_manager = new QXmppMessageReceiptManager();

    m_client.addExtension(m_manager);
    m_logger.setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(&m_logger);

    connect(&m_logger, &QXmppLogger::message,
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
    QTest::newRow("error-request")
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
    QTest::newRow("error-receipt")
        << QByteArray(
               "<message xml:lang=\"en\" "
               "to=\"northumberland@shakespeare.lit/westminster\" "
               "from=\"kingrichard@royalty.england.lit/throne\" "
               "type=\"error\" id=\"bi29sg183b4v\" "
               "> "
               "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
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
    QVERIFY(doc.setContent(xml, true));
    QDomElement element = doc.documentElement();
    QXmppMessage msg;
    msg.parse(element);

    QCOMPARE(m_manager->handleMessage(msg), handled);
    QCOMPARE(m_messageDelivered, accept);
    QCOMPARE(m_receiptSent, sent);
}

QTEST_MAIN(tst_QXmppMessageReceiptManager)
#include "tst_qxmppmessagereceiptmanager.moc"
