/*
 * Copyright (C) 2008-2019 The QXmpp developers
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

#include <QObject>
#include "QXmppClient.h"
#include "QXmppMessageReceiptManager.h"
#include "util.h"

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

private:
    QXmppMessageReceiptManager m_manager;
    bool m_messageDelivered = false;
};

void tst_QXmppMessageReceiptManager::initTestCase()
{
    connect(&m_manager, &QXmppMessageReceiptManager::messageDelivered,
            this, &tst_QXmppMessageReceiptManager::handleMessageDelivered);
}

void tst_QXmppMessageReceiptManager::testReceipt_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");

    QTest::newRow("correct")
            << QByteArray(
                   "<message id=\"bi29sg183b4v\" "
                        "to=\"northumberland@shakespeare.lit/westminster\" "
                        "from=\"kingrichard@royalty.england.lit/throne\" "
                        "type=\"normal\">"
                     "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
                   "</message>"
               )
            << true;
    QTest::newRow("from-to-equal")
            << QByteArray(
                   "<message id=\"bi29sg183b4v\" "
                        "to=\"kingrichard@royalty.england.lit/westminster\" "
                        "from=\"kingrichard@royalty.england.lit/throne\" "
                        "type=\"normal\">"
                     "<received xmlns=\"urn:xmpp:receipts\" id=\"richard2-4.1.247\"/>"
                   "</message>"
               )
            << false;
}

void tst_QXmppMessageReceiptManager::testReceipt()
{
    m_messageDelivered = false;

    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    QVERIFY(m_manager.handleStanza(element));
    QCOMPARE(m_messageDelivered, accept);
}

QTEST_MAIN(tst_QXmppMessageReceiptManager)
#include "tst_qxmppmessagereceiptmanager.moc"
