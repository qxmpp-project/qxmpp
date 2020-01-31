/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *  Melvin Keskin
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
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"

#include "util.h"

class tst_QXmppClient : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void handleMessageSent(QXmppLogger::MessageType type, const QString &text) const;
    void testSendMessage();

    void testIndexOfExtension();

    void testAddExtension();

private:
    QXmppClient *client;
};

void tst_QXmppClient::handleMessageSent(QXmppLogger::MessageType type, const QString &text) const
{
    QCOMPARE(type, QXmppLogger::MessageType::SentMessage);

    QXmppMessage msg;
    parsePacket(msg, text.toUtf8());

    QCOMPARE(msg.from(), QString());
    QCOMPARE(msg.to(), QStringLiteral("support@qxmpp.org"));
    QCOMPARE(msg.body(), QStringLiteral("implement XEP-* plz"));
}

void tst_QXmppClient::initTestCase()
{
    client = new QXmppClient(this);
}

void tst_QXmppClient::testSendMessage()
{
    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::SignalLogging);
    client->setLogger(&logger);

    connect(&logger, &QXmppLogger::message, this, &tst_QXmppClient::handleMessageSent);

    client->sendMessage(
        QStringLiteral("support@qxmpp.org"),
        QStringLiteral("implement XEP-* plz"));

    // see handleMessageSent()

    client->setLogger(nullptr);
}

void tst_QXmppClient::testIndexOfExtension()
{
    auto client = new QXmppClient;

    for (auto *ext : client->extensions()) {
        client->removeExtension(ext);
    }

    auto rosterManager = new QXmppRosterManager(client);
    auto vCardManager = new QXmppVCardManager;

    client->addExtension(rosterManager);
    client->addExtension(vCardManager);

    // This extension is not in the list.
    QCOMPARE(client->indexOfExtension<QXmppVersionManager>(), -1);

    // These extensions are in the list.
    QCOMPARE(client->indexOfExtension<QXmppRosterManager>(), 0);
    QCOMPARE(client->indexOfExtension<QXmppVCardManager>(), 1);
}

class PreferredIndexExtension : public QXmppClientExtension
{
    Q_OBJECT

public:
    int preferredInsertionIndex(QXmppClient *) const override
    {
        return 2;
    }

    bool handleStanza(const QDomElement &) override
    {
        return false;
    }
};

void tst_QXmppClient::testAddExtension()
{
    auto client = new QXmppClient;

    for (auto *ext : client->extensions()) {
        client->removeExtension(ext);
    }

    auto rosterManager = new QXmppRosterManager(client);
    auto vCardManager = new QXmppVCardManager;
    auto versionManager = new QXmppVersionManager;
    auto preferredIndexExtension = new PreferredIndexExtension;

    client->addExtension(rosterManager);
    client->addExtension(vCardManager);
    client->addExtension(versionManager);
    client->addExtension(preferredIndexExtension);

    QCOMPARE(client->extensions().at(0), rosterManager);
    QCOMPARE(client->extensions().at(1), vCardManager);
    QCOMPARE(client->extensions().at(2), preferredIndexExtension);
    QCOMPARE(client->extensions().at(3), versionManager);
}

QTEST_MAIN(tst_QXmppClient)
#include "tst_qxmppclient.moc"
