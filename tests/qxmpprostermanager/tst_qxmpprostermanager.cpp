/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
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

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppRosterManager.h"

#include "util.h"

class tst_QXmppRosterManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testDiscoFeatures();
    void testRenameItem();

private:
    QXmppClient client;
    QXmppLogger logger;
    QXmppRosterManager *manager;
};

void tst_QXmppRosterManager::initTestCase()
{
    logger.setLoggingType(QXmppLogger::SignalLogging);
    client.setLogger(&logger);

    manager = client.findExtension<QXmppRosterManager>();
}

void tst_QXmppRosterManager::testDiscoFeatures()
{
    QCOMPARE(manager->discoveryFeatures(), QStringList());
}

void tst_QXmppRosterManager::testRenameItem()
{
    // used to clean up lambda signal connections
    QObject context;

    auto createItem = [](const QString &jid, const QString &ask = {}) -> QXmppRosterIq::Item {
        QXmppRosterIq::Item item;
        item.setBareJid(jid);
        item.setSubscriptionStatus(ask);
        return item;
    };

    // fill roster with initial contacts to rename
    QXmppRosterIq initialItems;
    initialItems.setType(QXmppIq::Result);
    initialItems.addItem(createItem("stpeter@jabber.org"));
    initialItems.addItem(createItem("bob@qxmpp.org"));

    QVERIFY(manager->handleStanza(writePacketToDom(initialItems)));

    // set a subscription state for bob (the subscription state MUST NOT be
    // sent when renaming an item, so we need to check that it's not)
    QXmppRosterIq bobAsk;
    bobAsk.setType(QXmppIq::Set);
    bobAsk.addItem(createItem("bob@qxmpp.org", "subscribe"));

    QVERIFY(manager->handleStanza(writePacketToDom(bobAsk)));
    QCOMPARE(manager->getRosterEntry("bob@qxmpp.org").subscriptionStatus(), QStringLiteral("subscribe"));

    // rename bob
    bool requestSent = false;
    connect(&logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType type, const QString &text) {
        if (type == QXmppLogger::SentMessage) {
            requestSent = true;

            QXmppRosterIq renameRequest;
            parsePacket(renameRequest, text.toUtf8());
            QCOMPARE(renameRequest.items().size(), 1);
            QCOMPARE(renameRequest.items().first().bareJid(), QStringLiteral("bob@qxmpp.org"));
            QCOMPARE(renameRequest.items().first().name(), QStringLiteral("Bob"));
            // check that subscription state ('ask') for bob is not included
            QVERIFY(renameRequest.items().first().subscriptionStatus().isNull());
        }
    });

    manager->renameItem("bob@qxmpp.org", "Bob");
    QVERIFY(requestSent);
}

QTEST_MAIN(tst_QXmppRosterManager)
#include "tst_qxmpprostermanager.moc"
