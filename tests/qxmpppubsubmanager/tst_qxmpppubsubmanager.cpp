/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Germán Márquez Mejía
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
#include "QXmppMessage.h"
#include "QXmppPubSubIq.h"
#include "QXmppPubSubManager.h"
#include "util.h"

Q_DECLARE_METATYPE(QXmppDataForm);
Q_DECLARE_METATYPE(QXmppPubSubIq);
Q_DECLARE_METATYPE(QXmppPubSubItem);

class tst_QXmppPubSubManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testCreateNodes_data();
    void testCreateNodes();

    void testDeleteNodes_data();
    void testDeleteNodes();

    void testPublishItems_data();
    void testPublishItems();

    void testRetractItem_data();
    void testRetractItem();

    void testRequestItems_data();
    void testRequestItems();

    void testHandleStanza_data();
    void testHandleStanza();

    void testMessageReceived_data();
    void testMessageReceived();

private:
    QXmppPubSubManager *m_manager;
    QXmppClient m_client;
    QXmppLogger m_logger;
};

void tst_QXmppPubSubManager::initTestCase()
{
    m_manager = new QXmppPubSubManager();
    m_client.addExtension(m_manager);
    m_logger.setLoggingType(QXmppLogger::SignalLogging);
    m_client.setLogger(&m_logger);
}

void tst_QXmppPubSubManager::testCreateNodes_data()
{
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");

    QTest::addRow("createNode")
        << false
        << "pubsub.shakespeare.lit"
        << "princely_musings";

    QTest::addRow("createPepNode")
        << true
        << "juliet@capulet.lit"
        << "urn:xmpp:omemo:1:bundles";
}

void tst_QXmppPubSubManager::testCreateNodes()
{
    QFETCH(bool, isPep);
    QFETCH(QString, jid);
    QFETCH(QString, node);

    bool signalTriggered = false;

    QObject context;
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType messageType, const QString &messageText) {
        if (messageType == QXmppLogger::SentMessage) {
            signalTriggered = true;

            QXmppPubSubIq iq;
            parsePacket(iq, messageText.toUtf8());

            QCOMPARE(iq.type(), QXmppIq::Set);
            QCOMPARE(iq.to(), jid);
            QCOMPARE(iq.queryNodeName(), node);
            QCOMPARE(iq.queryType(), QXmppPubSubIq::QueryType::CreateQuery);
        }
    });

    if (isPep) {
        m_client.connectToServer(jid, {});
        m_client.disconnectFromServer();
        m_manager->createPepNode(node);
    } else {
        m_manager->createNode(jid, node);
    }
}

void tst_QXmppPubSubManager::testDeleteNodes_data()
{
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");

    QTest::addRow("deleteNode")
        << false
        << "pubsub.shakespeare.lit"
        << "princely_musings";

    QTest::addRow("deletePepNode")
        << true
        << "juliet@capulet.lit"
        << "urn:xmpp:omemo:1:bundles";
}

void tst_QXmppPubSubManager::testDeleteNodes()
{
    QFETCH(bool, isPep);
    QFETCH(QString, jid);
    QFETCH(QString, node);

    bool signalTriggered = false;

    QObject context;
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType messageType, const QString &messageText) {
        if (messageType == QXmppLogger::SentMessage) {
            signalTriggered = true;

            QXmppPubSubIq iq;
            parsePacket(iq, messageText.toUtf8());

            QCOMPARE(iq.type(), QXmppIq::Set);
            QCOMPARE(iq.to(), jid);
            QCOMPARE(iq.queryNodeName(), node);
            QCOMPARE(iq.queryType(), QXmppPubSubIq::QueryType::DeleteQuery);
        }
    });

    if (isPep) {
        m_client.connectToServer(jid, {});
        m_client.disconnectFromServer();
        m_manager->deletePepNode(node);
    } else {
        m_manager->deleteNode(jid, node);
    }
}

void tst_QXmppPubSubManager::testPublishItems_data()
{
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QList<QXmppPubSubItem>>("items");
    QTest::addColumn<QXmppDataForm>("publishOptions");

    QXmppElement publishItemPayload;
    publishItemPayload.setTagName("bundle");
    publishItemPayload.setAttribute("xmlns", "urn:xmpp:omemo:1");

    QXmppPubSubItem publishItem1("31415", publishItemPayload);
    QXmppPubSubItem publishItem2("549038", publishItemPayload);

    QList<QXmppPubSubItem> listWithOnePublishItem {publishItem1};
    QList<QXmppPubSubItem> listWithMultiplePublishItems {publishItem1, publishItem2};

    QXmppDataForm::Field formTypeField(QXmppDataForm::Field::HiddenField);
    formTypeField.setKey("FORM_TYPE");
    formTypeField.setValue("http://jabber.org/protocol/pubsub#publish-options");

    QXmppDataForm::Field accessModelField;
    accessModelField.setKey("pubsub#access_model");
    accessModelField.setValue("presence");

    QList<QXmppDataForm::Field> dataFormFields {formTypeField, accessModelField};

    QXmppDataForm publishOptions(QXmppDataForm::Submit);
    publishOptions.setFields(dataFormFields);

    QTest::addRow("publishItem")
            << false
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << listWithOnePublishItem
            << QXmppDataForm();

    QTest::addRow("publishItemWithOptions")
            << false
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << listWithOnePublishItem
            << publishOptions;

    QTest::addRow("publishItems")
            << false
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << listWithMultiplePublishItems
            << QXmppDataForm();

    QTest::addRow("publishItemsWithOptions")
            << false
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << listWithMultiplePublishItems
            << publishOptions;

    QTest::addRow("publishPepItem")
            << true
            << "juliet@capulet.lit"
            << "urn:xmpp:omemo:1:bundles"
            << listWithOnePublishItem
            << QXmppDataForm();

    QTest::addRow("publishPepItemWithOptions")
            << true
            << "juliet@capulet.lit"
            << "urn:xmpp:omemo:1:bundles"
            << listWithOnePublishItem
            << publishOptions;

    QTest::addRow("publishPepItems")
            << true
            << "juliet@capulet.lit"
            << "urn:xmpp:omemo:1:bundles"
            << listWithMultiplePublishItems
            << QXmppDataForm();

    QTest::addRow("publishPepItemsWithOptions")
            << true
            << "juliet@capulet.lit"
            << "urn:xmpp:omemo:1:bundles"
            << listWithMultiplePublishItems
            << publishOptions;
}

void tst_QXmppPubSubManager::testPublishItems()
{
    QFETCH(bool, isPep);
    QFETCH(QString, jid);
    QFETCH(QString, node);
    QFETCH(QList<QXmppPubSubItem>, items);
    QFETCH(QXmppDataForm, publishOptions);

    bool signalTriggered = false;

    QObject context;
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType messageType, const QString &messageText) {
        if (messageType == QXmppLogger::SentMessage) {
            signalTriggered = true;

            QXmppPubSubIq iq;
            parsePacket(iq, messageText.toUtf8());

            QCOMPARE(iq.type(), QXmppIq::Set);
            QCOMPARE(iq.to(), jid);
            QCOMPARE(iq.queryNodeName(), node);
            QCOMPARE(iq.queryType(), QXmppPubSubIq::PublishQuery);

            QStringList actualItemIds;
            const auto actualItems = iq.items();
            for (const auto &item : actualItems)
                actualItemIds << item.id();

            QStringList expectedItemIds;
            const auto expectedItems = items;
            for (const auto &item : expectedItems)
                expectedItemIds << item.id();

            QCOMPARE(actualItemIds, expectedItemIds);
        }
    });

    if (isPep) {
        m_client.connectToServer(jid, {});
        m_client.disconnectFromServer();

        if (publishOptions.isNull()) {
            if (items.size() == 1)
                m_manager->publishPepItem(node, items.first());
            else
                m_manager->publishPepItems(node, items);
        } else {
            if (items.size() == 1)
                m_manager->publishPepItem(node, items.first(), publishOptions);
            else
                m_manager->publishPepItems(node, items, publishOptions);
        }
    } else {
        if (publishOptions.isNull()) {
            if (items.size() == 1)
                m_manager->publishItem(jid, node, items.first());
            else
                m_manager->publishItems(jid, node, items);
        } else {
            if (items.size() == 1)
                m_manager->publishItem(jid, node, items.first(), publishOptions);
            else
                m_manager->publishItems(jid, node, items, publishOptions);
        }
    }

    QVERIFY(signalTriggered);
}

void tst_QXmppPubSubManager::testRetractItem_data()
{
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QString>("itemId");

    QTest::addRow("retractItem")
        << false
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << "ae890ac52d0df67ed7cfdf51b644e901";

    QTest::addRow("retractPepItem")
        << true
        << "juliet@capulet.lit"
        << "urn:xmpp:omemo:1:bundles"
        << "31415";
}

void tst_QXmppPubSubManager::testRetractItem()
{
    QFETCH(bool, isPep);
    QFETCH(QString, jid);
    QFETCH(QString, node);
    QFETCH(QString, itemId);

    bool signalTriggered = false;

    QObject context;
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType messageType, const QString &messageText) {
        if (messageType == QXmppLogger::SentMessage) {
            signalTriggered = true;

            QXmppPubSubIq iq;
            parsePacket(iq, messageText.toUtf8());

            QCOMPARE(iq.type(), QXmppIq::Set);
            QCOMPARE(iq.to(), jid);
            QCOMPARE(iq.queryNodeName(), node);
            QCOMPARE(iq.queryType(), QXmppPubSubIq::QueryType::RetractQuery);
            QCOMPARE(iq.items().size(), 1);
            QCOMPARE(iq.items().first().id(), itemId);
        }
    });

    if (isPep) {
        m_client.connectToServer(jid, {});
        m_client.disconnectFromServer();
        m_manager->retractPepItem(node, itemId);
    } else {
        m_manager->retractItem(jid, node, itemId);
    }
}

void tst_QXmppPubSubManager::testRequestItems_data()
{
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QStringList>("itemIds");

    QTest::addRow("oneItem")
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << QStringList {"ae890ac52d0df67ed7cfdf51b644e901"};

    QTest::addRow("someItems")
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << QStringList {"ae890ac52d0df67ed7cfdf51b644e901", "3300659945416e274474e469a1f0154c"};

    QTest::addRow("allItems")
            << "pubsub.shakespeare.lit"
            << "princely_musings"
            << QStringList();
}

void tst_QXmppPubSubManager::testRequestItems()
{
    QFETCH(QString, jid);
    QFETCH(QString, node);
    QFETCH(QStringList, itemIds);

    bool signalTriggered = false;

    QObject context;
    connect(&m_logger, &QXmppLogger::message, &context, [&](QXmppLogger::MessageType messageType, const QString &messageText) {
        if (messageType == QXmppLogger::SentMessage) {
            signalTriggered = true;

            QXmppPubSubIq iq;
            parsePacket(iq, messageText.toUtf8());

            QCOMPARE(iq.type(), QXmppIq::Get);
            QCOMPARE(iq.to(), jid);
            QCOMPARE(iq.queryNodeName(), node);
            QCOMPARE(iq.queryType(), QXmppPubSubIq::ItemsQuery);

            QStringList actualItemIds;
            const auto items = iq.items();
            for (const auto &item : items)
                actualItemIds << item.id();

            QCOMPARE(actualItemIds, itemIds);
        }
    });

    switch (itemIds.size()) {
    case 0:
        m_manager->requestItems(jid, node);
        break;
    case 1:
        m_manager->requestItem(jid, node, itemIds.first());
        break;
    default:
        m_manager->requestItems(jid, node, itemIds);
    }

    QVERIFY(signalTriggered);
}

void tst_QXmppPubSubManager::testHandleStanza_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<QXmppPubSubIq>("expectedIq");

    QXmppPubSubIq expectedIq;
    expectedIq.setId("items1");
    expectedIq.setQueryNodeName("princely_musings");
    expectedIq.setItems(QList<QXmppPubSubItem>()
        << QXmppPubSubItem("368866411b877c30064a5f62b917cffe")
        << QXmppPubSubItem("3300659945416e274474e469a1f0154c")
    );

    QTest::newRow("received1")
        << QByteArray(
               "<iq type='result' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' id='items1'>"
                 "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                   "<items node='princely_musings'>"
                     "<item id='368866411b877c30064a5f62b917cffe'>"
                       "<entry xmlns='http://www.w3.org/2005/Atom'>"
                         "<title>The Uses of This World</title>"
                         "<summary>"
                           "O, that this too too solid flesh would melt"
                           "Thaw and resolve itself into a dew!"
                         "</summary>"
                         "<link rel='alternate' type='text/html' href='http://denmark.lit/2003/12/13/atom03'/>"
                         "<id>tag:denmark.lit,2003:entry-32396</id>"
                         "<published>2003-12-12T17:47:23Z</published>"
                         "<updated>2003-12-12T17:47:23Z</updated>"
                       "</entry>"
                     "</item>"
                     "<item id='3300659945416e274474e469a1f0154c'>"
                       "<entry xmlns='http://www.w3.org/2005/Atom'>"
                         "<title>Ghostly Encounters</title>"
                         "<summary>"
                           "O all you host of heaven! O earth! what else?"
                           "And shall I couple hell? O, fie! Hold, hold, my heart;"
                           "And you, my sinews, grow not instant old,"
                           "But bear me stiffly up. Remember thee!"
                         "</summary>"
                         "<link rel='alternate' type='text/html' href='http://denmark.lit/2003/12/13/atom03'/>"
                         "<id>tag:denmark.lit,2003:entry-32396</id>"
                         "<published>2003-12-12T23:21:34Z</published>"
                         "<updated>2003-12-12T23:21:34Z</updated>"
                       "</entry>"
                     "</item>"
                   "</items>"
                 "</pubsub>"
               "</iq>"
               )
        << true
        << expectedIq;
}

void tst_QXmppPubSubManager::testHandleStanza()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, accept);
    QFETCH(QXmppPubSubIq, expectedIq);

    bool signalTriggered = false;

    QObject context;
    connect(m_manager, &QXmppPubSubManager::itemsReceived, &context, [&](const QXmppPubSubIq &iq) {
        signalTriggered = true;
        QCOMPARE(iq.id(), expectedIq.id());
        QCOMPARE(iq.queryNodeName(), expectedIq.queryNodeName());
        QCOMPARE(iq.items().size(), expectedIq.items().size());
        for (int i = 0; i < iq.items().size(); i++)
            QCOMPARE(iq.items().at(i).id(), expectedIq.items().at(i).id());
    });

    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();

    bool accepted = m_manager->handleStanza(element);

    QCOMPARE(accepted, accept);
    QCOMPARE(signalTriggered, accept);
}

void tst_QXmppPubSubManager::testMessageReceived_data()
{
    QTest::addColumn<QXmppMessage>("message");

    QXmppMessage messageForValidEventNotification;
    parsePacket(
        messageForValidEventNotification,
        QString(
            "<message from='pubsub.shakespeare.lit' to='francisco@denmark.lit' id='foo'>"
              "<event xmlns='http://jabber.org/protocol/pubsub#event'>"
                "<items node='princely_musings'>"
                  "<item id='ae890ac52d0df67ed7cfdf51b644e901'>"
                    "<entry xmlns='http://www.w3.org/2005/Atom'>"
                      "<title>Soliloquy</title>"
                      "<summary>"
                        "To be, or not to be: that is the question:"
                        "Whether 'tis nobler in the mind to suffer"
                        "The slings and arrows of outrageous fortune,"
                        "Or to take arms against a sea of troubles,"
                        "And by opposing end them?"
                      "</summary>"
                      "<link rel='alternate' type='text/html' href='http://denmark.lit/2003/12/13/atom03'/>"
                      "<id>tag:denmark.lit,2003:entry-32397</id>"
                      "<published>2003-12-13T18:30:02Z</published>"
                      "<updated>2003-12-13T18:30:02Z</updated>"
                    "</entry>"
                  "</item>"
                "</items>"
              "</event>"
            "</message>"
        ).toUtf8()
    );

    QTest::newRow("validEventNotification")
        << messageForValidEventNotification;

    QXmppMessage messageForInvalidEventNotification;
    parsePacket(
        messageForInvalidEventNotification,
        QString(
            "<message from='romeo@example.net/orchard' id='sl3nx51f' to='juliet@example.com/balcony'>"
              "<body>Neither, fair saint, if either thee dislike.</body>"
            "</message>"
        ).toUtf8()
    );

    QTest::newRow("invalidEventNotification")
        << messageForInvalidEventNotification;
}

void tst_QXmppPubSubManager::testMessageReceived()
{
    QFETCH(QXmppMessage, message);

    bool signalTriggered = false;

    QObject context;
    connect(m_manager, &QXmppPubSubManager::eventNotificationReceived, &context, [&] {
        signalTriggered = true;
    });

    emit m_client.messageReceived(message);

    QCOMPARE(signalTriggered, message.isPubSubEvent());
}

QTEST_MAIN(tst_QXmppPubSubManager)
#include "tst_qxmpppubsubmanager.moc"
