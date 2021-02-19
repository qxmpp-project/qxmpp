/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Melvin Keskin
 *  Linus Jahn
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
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"
#include "util.h"

Q_DECLARE_METATYPE(QXmppVCardIq);

class tst_QXmppVCardManager : public QObject
{
    Q_OBJECT

private slots:
    void testHandleStanza_data();
    void testHandleStanza();

private:
    QXmppClient m_client;
};

void tst_QXmppVCardManager::testHandleStanza_data()
{
    QTest::addColumn<QXmppVCardIq>("expectedIq");
    QTest::addColumn<bool>("isClientVCard");

#define ROW(name, iq, clientVCard) \
    QTest::newRow(QT_STRINGIFY(name)) << iq << clientVCard

    QXmppVCardIq iq;
    iq.setType(QXmppIq::Result);
    iq.setTo("stpeter@jabber.org/roundabout");
    iq.setFullName("Jeremie Miller");

    auto iqFromBare = iq;
    iqFromBare.setFrom("stpeter@jabber.org");

    auto iqFromFull = iq;
    iqFromFull.setFrom("stpeter@jabber.org/roundabout");

    ROW(client-vcard-from-empty, iq, true);
    ROW(client-vcard-from-bare, iqFromBare, true);
    ROW(client-vcard-from-full, iqFromFull, false);

#undef ROW
}

void tst_QXmppVCardManager::testHandleStanza()
{
    QFETCH(QXmppVCardIq, expectedIq);
    QFETCH(bool, isClientVCard);

    // initialize new manager to clear internal values
    QXmppVCardManager *manager = new QXmppVCardManager();
    m_client.addExtension(manager);

    // sets own jid internally
    m_client.connectToServer("stpeter@jabber.org", {});
    m_client.disconnectFromServer();

    bool vCardReceived = false;
    bool clientVCardReceived = false;

    QObject context;
    connect(manager, &QXmppVCardManager::vCardReceived, &context, [&](QXmppVCardIq iq) {
        vCardReceived = true;
        QCOMPARE(iq, expectedIq);
    });
    connect(manager, &QXmppVCardManager::clientVCardReceived, &context, [&]() {
        clientVCardReceived = true;
        QCOMPARE(manager->clientVCard(), expectedIq);
    });

    bool accepted = manager->handleStanza(writePacketToDom(expectedIq));

    QVERIFY(accepted);
    QVERIFY(vCardReceived);
    QCOMPARE(clientVCardReceived, isClientVCard);

    // clean up (client deletes manager)
    m_client.removeExtension(manager);
}

QTEST_MAIN(tst_QXmppVCardManager)
#include "tst_qxmppvcardmanager.moc"
