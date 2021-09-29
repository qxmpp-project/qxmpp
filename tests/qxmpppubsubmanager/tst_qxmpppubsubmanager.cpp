/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *  Germán Márquez Mejía
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
#include "QXmppMessage.h"
#include "QXmppPubSubAffiliation.h"
#include "QXmppPubSubEventManager.h"
#include "QXmppPubSubIq.h"
#include "QXmppPubSubItem.h"
#include "QXmppPubSubManager.h"
#include "QXmppPubSubPublishOptions.h"
#include "QXmppPubSubSubscribeOptions.h"
#include "QXmppTuneItem.h"

#include "TestClient.h"
#include "util.h"
#include <QObject>

Q_DECLARE_METATYPE(QXmppPubSubIq<>);
Q_DECLARE_METATYPE(std::optional<QXmppPubSubPublishOptions>);

using PSManager = QXmppPubSubManager;
using Affiliation = QXmppPubSubAffiliation;
using AffiliationType = QXmppPubSubAffiliation::Affiliation;

class TestEventManager : public QXmppPubSubEventManager
{
public:
    QXmppPubSubManager *pubSub()
    {
        return QXmppPubSubEventManager::pubSub();
    }

    bool handlePubSubEvent(const QDomElement &, const QString &pubSubService, const QString &nodeName) override
    {
        m_events++;
#define return return false;
        QCOMPARE(pubSubService, m_serviceJid);
        QCOMPARE(nodeName, m_node);
#undef return
        return true;
    }

    QString m_serviceJid;
    QString m_node;
    uint m_events = 0;
};

class tst_QXmppPubSubManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testDiscoFeatures();
    Q_SLOT void testFetchNodes();
    Q_SLOT void testCreateNodes_data();
    Q_SLOT void testCreateNodes();
    Q_SLOT void testCreateInstantNode();
    Q_SLOT void testDeleteNodes_data();
    Q_SLOT void testDeleteNodes();
    Q_SLOT void testPublishItems_data();
    Q_SLOT void testPublishItems();
    Q_SLOT void testRetractItem_data();
    Q_SLOT void testRetractItem();
    Q_SLOT void testPurgeItems();
    Q_SLOT void testPurgePepItems();
    Q_SLOT void testRequestItems_data();
    Q_SLOT void testRequestItems();
    Q_SLOT void testRequestItemNotFound();
    Q_SLOT void testRequestNodeAffiliations();
    Q_SLOT void testRequestAffiliations();
    Q_SLOT void testRequestAffiliationsNode();
    Q_SLOT void testRequestOptions();
    Q_SLOT void testRequestOptionsError();
    Q_SLOT void testSetOptions();
    Q_SLOT void testEventNotifications_data();
    Q_SLOT void testEventNotifications();
};

void tst_QXmppPubSubManager::testDiscoFeatures()
{
    // so the coverage report is happy:
    PSManager manager;
    QCOMPARE(manager.discoveryFeatures(), QStringList {"http://jabber.org/protocol/pubsub#rsm"});
}

void tst_QXmppPubSubManager::testFetchNodes()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    auto future = psManager->fetchNodes("pepuser@qxmpp.org");
    test.expect("<iq id='qxmpp1' to='pepuser@qxmpp.org' type='get'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' id='qxmpp1'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items'>"
                               "<item jid='pubsub.shakespeare.lit' node='blogs' name='Weblog updates'/>"
                               "<item jid='pubsub.shakespeare.lit' node='news' name='News and announcements'/>"
                               "</query></iq>"));

    const auto nodes = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(nodes, (QVector<QString> { "blogs", "news" }));
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

    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    QFuture<PSManager::Result> future;
    if (isPep) {
        test.configuration().setJid(jid);
        future = psManager->createPepNode(node);
    } else {
        future = psManager->createNode(jid, node);
    }

    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub'><create node='%2'/></pubsub></iq>").arg(jid, node));
    test.inject<QString>("<iq id='qxmpp1' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testCreateInstantNode()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    auto future = psManager->createInstantNode("pubsub.qxmpp.org");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'><create/></pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<create node='25e3d37dabbab9541f7523321421edc5bfeb2dae'/>"
                               "</pubsub></iq>"));

    const auto nodeId = expectFutureVariant<QString>(future);
    QCOMPARE(nodeId, QString("25e3d37dabbab9541f7523321421edc5bfeb2dae"));
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

    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    QFuture<PSManager::Result> future;
    if (isPep) {
        test.configuration().setJid(jid);
        future = psManager->deletePepNode(node);
    } else {
        future = psManager->deleteNode(jid, node);
    }

    // FIXME: pubsub#owner here, but not for <create/>?
    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub#owner'><delete node='%2'/></pubsub></iq>").arg(jid, node));
    test.inject<QString>("<iq id='qxmpp1' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testPublishItems_data()
{
    using OptionsOpt = std::optional<QXmppPubSubPublishOptions>;
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QVector<QXmppPubSubItem>>("items");
    QTest::addColumn<OptionsOpt>("publishOptions");
    QTest::addColumn<bool>("returnIds");

    QXmppTuneItem item1;
    item1.setId("1234");
    item1.setTitle("Hello Goodbye");

    QXmppTuneItem item2;
    item2.setId("5678");
    item2.setArtist("Rick Astley");
    item2.setTitle("Never gonna give you up");

    QVector<QXmppPubSubItem> items1 { item1 };
    QVector<QXmppPubSubItem> items2 { item1, item2 };

    QXmppPubSubPublishOptions publishOptions;
    publishOptions.setAccessModel(QXmppPubSubPublishOptions::Presence);

    auto addRow = [&](const char *name, bool isPep, QString &&jid,
                      QString &&node, const QVector<QXmppPubSubItem> &items) {
        QTest::addRow("%s", name) << isPep << jid << node << items << OptionsOpt() << false;
        QTest::addRow("%s%s", name, "ReturnIds") << isPep << jid << node << items << OptionsOpt() << true;
        QTest::addRow("%s%s", name, "WithOptions") << isPep << jid << node << items << std::make_optional(publishOptions) << false;
        QTest::addRow("%s%s%s", name, "WithOptions", "ReturnIds") << isPep << jid << node << items << std::make_optional(publishOptions) << true;
    };

    addRow("publishItem", false, "pubsub.shakespeare.lit", "princely_musings", items1);
    addRow("publishItems", false, "pubsub.shakespeare.lit", "princely_musings", items2);
    addRow("publishPepItem", true, "juliet@capulet.lit", "urn:xmpp:omemo:1:bundles", items1);
    addRow("publishPepItems", true, "juliet@capulet.lit", "urn:xmpp:omemo:1:bundles", items2);
}

void tst_QXmppPubSubManager::testPublishItems()
{
    QFETCH(bool, isPep);
    QFETCH(QString, jid);
    QFETCH(QString, node);
    QFETCH(QVector<QXmppPubSubItem>, items);
    QFETCH(std::optional<QXmppPubSubPublishOptions>, publishOptions);
    QFETCH(bool, returnIds);

    const auto itemsXml = [=]() {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        QXmlStreamWriter writer(&buffer);
        for (const auto &item : items) {
            item.toXml(&writer);
        }
        return buffer.data();
    }();
    const auto publishOptionsXml = [&]() -> QString {
        if (publishOptions) {
            auto form = publishOptions->toDataForm();
            form.setType(QXmppDataForm::Submit);
            return "<publish-options>" + QString::fromUtf8(packetToXml(form)) + "</publish-options>";
        }
        return {};
    }();
    const auto itemIdsXml = [&]() {
        QString result;
        for (const auto &item : std::as_const(items)) {
            result += "<item id='" + item.id() + "'/>";
        }
        return result;
    }();
    const auto itemIds = [&]() {
        QVector<QString> ids;
        for (const auto &item : std::as_const(items)) {
            ids << item.id();
        }
        return ids;
    }();

    TestClient test;
    if (isPep) {
        test.configuration().setJid(jid);
    }
    auto *psManager = test.addNewExtension<PSManager>();

    auto injectXml = [&]() {
        test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub'><publish node='%2'>%3</publish>%4</pubsub></iq>")
                        .arg(jid, node, itemsXml, publishOptionsXml));
        if (returnIds) {
            test.inject(QStringLiteral(R"(
                <iq type='result' from='%1' id='qxmpp1'>
                    <pubsub xmlns='http://jabber.org/protocol/pubsub'>
                        <publish node='%2'>%3</publish>
                    </pubsub>
                </iq>)")
                            .arg(jid, node, itemIdsXml));
        } else {
            test.inject(QStringLiteral("<iq id='qxmpp1' type='result'/>"));
        }
    };

    if (items.size() == 1) {
        QFuture<PSManager::PublishItemResult> future;
        if (isPep) {
            if (publishOptions) {
                future = psManager->publishPepItem(node, items.constFirst(), *publishOptions);
            } else {
                future = psManager->publishPepItem(node, items.constFirst());
            }
        } else {
            if (publishOptions) {
                future = psManager->publishItem(jid, node, items.constFirst(), *publishOptions);
            } else {
                future = psManager->publishItem(jid, node, items.constFirst());
            }
        }

        injectXml();
        const auto id = expectFutureVariant<QString>(future);
        if (returnIds) {
            QCOMPARE(id, items.constFirst().id());
        } else {
            QVERIFY(id.isNull());
        }
    } else {
        QFuture<PSManager::PublishItemsResult> future;
        if (isPep) {
            if (publishOptions) {
                future = psManager->publishPepItems(node, items, *publishOptions);
            } else {
                future = psManager->publishPepItems(node, items);
            }
        } else {
            if (publishOptions) {
                future = psManager->publishItems(jid, node, items, *publishOptions);
            } else {
                future = psManager->publishItems(jid, node, items);
            }
        }

        injectXml();
        const auto ids = expectFutureVariant<QVector<QString>>(future);
        if (returnIds) {
            QCOMPARE(ids, itemIds);
        } else {
            QVERIFY(ids.empty());
        }
    }
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

    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    QFuture<PSManager::Result> future;
    if (isPep) {
        test.configuration().setJid(jid);
        future = psManager->retractPepItem(node, itemId);
    } else {
        future = psManager->retractItem(jid, node, itemId);
    }

    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub'><retract node='%2'><item id='%3'/></retract></pubsub></iq>")
                    .arg(jid, node, itemId));
    test.inject(QStringLiteral("<iq type='result' from='%1' id='qxmpp1'/>")
                    .arg(jid));

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testPurgeItems()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->purgeItems("pubsub.qxmpp.org", "news");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<purge node='news'/>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'/>"));
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testPurgePepItems()
{
    TestClient test;
    test.configuration().setJid("user@qxmpp.org");
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->purgePepItems("urn:xmpp:x-avatar:0");
    test.expect("<iq id='qxmpp1' to='user@qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<purge node='urn:xmpp:x-avatar:0'/>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='user@qxmpp.org' id='qxmpp1'/>"));
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testRequestItems_data()
{
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<bool>("requestIds");
    QTest::addColumn<QStringList>("itemIds");

    QTest::addRow("allItems-0")
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << false
        << QStringList();

    QTest::addRow("allItems-1")
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << false
        << QStringList { "ae890ac52d0df67ed7cfdf51b644e901" };

    QTest::addRow("allItems-2")
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << false
        << QStringList { "ae890ac52d0df67ed7cfdf51b644e901", "3300659945416e274474e469a1f0154c" };

    QTest::addRow("oneItemById")
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << true
        << QStringList { "ae890ac52d0df67ed7cfdf51b644e901" };

    QTest::addRow("twoItemsByIds")
        << "pubsub.shakespeare.lit"
        << "princely_musings"
        << true
        << QStringList { "ae890ac52d0df67ed7cfdf51b644e901", "3300659945416e274474e469a1f0154c" };
}

void tst_QXmppPubSubManager::testRequestItems()
{
    QFETCH(QString, jid);
    QFETCH(QString, node);
    QFETCH(bool, requestIds);
    QFETCH(QStringList, itemIds);

    QString itemsReplyXml;
    for (const auto &id : std::as_const(itemIds)) {
        itemsReplyXml += QStringLiteral(R"(
<item id='%1'>
<tune xmlns='http://jabber.org/protocol/tune'>
    <artist>Yes</artist>
    <length>686</length>
    <rating>8</rating>
    <source>Yessongs</source>
    <title>Heart of the Sunrise</title>
    <track>3</track>
    <uri>http://www.yesworld.com/lyrics/Fragile.html#9</uri>
</tune>
</item>)")
                             .arg(id);
    }

    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    QVector<QXmppTuneItem> returnedItems;

    if (requestIds) {
        QString itemsXml;
        for (const auto &id : std::as_const(itemIds)) {
            itemsXml += "<item id='" + id + "'/>";
        }

        if (itemIds.size() == 1) {
            auto future = psManager->requestItem<QXmppTuneItem>(jid, node, itemIds.constFirst());
            test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='get'>"
                                       "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                       "<items node='%2'>%3</items>"
                                       "</pubsub></iq>")
                            .arg(jid, node, itemsXml));
            test.inject(QStringLiteral("<iq type='result' from='%1' id='qxmpp1'>"
                                       "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                       "<items node='%2'>%3</items>"
                                       "</pubsub></iq>")
                            .arg(jid, node, itemsReplyXml));

            returnedItems = { expectFutureVariant<QXmppTuneItem>(future) };
        } else {
            auto future = psManager->requestItems<QXmppTuneItem>(jid, node, itemIds);
            test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='get'>"
                                       "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                       "<items node='%2'>%3</items>"
                                       "</pubsub></iq>")
                            .arg(jid, node, itemsXml));
            test.inject(QStringLiteral("<iq type='result' from='%1' id='qxmpp1'>"
                                       "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                       "<items node='%2'>%3</items>"
                                       "</pubsub></iq>")
                            .arg(jid, node, itemsReplyXml));

            returnedItems = expectFutureVariant<PSManager::Items<QXmppTuneItem>>(future).items;
        }
    } else {
        auto future = psManager->requestItems<QXmppTuneItem>(jid, node);
        test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='get'>"
                                   "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='%2'/></pubsub>"
                                   "</iq>")
                        .arg(jid, node));
        test.inject(QStringLiteral("<iq type='result' from='%1' id='qxmpp1'>"
                                   "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                                   "<items node='%2'>%3</items>"
                                   "</pubsub></iq>")
                        .arg(jid, node, itemsReplyXml));

        returnedItems = expectFutureVariant<PSManager::Items<QXmppTuneItem>>(future).items;
    }

    for (const auto &item : std::as_const(returnedItems)) {
        QCOMPARE(item.artist(), QStringLiteral("Yes"));
        QCOMPARE(item.length(), uint16_t(686));
        QCOMPARE(item.rating(), uint8_t(8));
        QCOMPARE(item.source(), QStringLiteral("Yessongs"));
        QCOMPARE(item.title(), QStringLiteral("Heart of the Sunrise"));
        QCOMPARE(item.track(), QLatin1String("3"));
        QCOMPARE(item.uri(), QUrl("http://www.yesworld.com/lyrics/Fragile.html#9"));
    }

    const auto itemsEqual = std::equal(returnedItems.cbegin(), returnedItems.cend(),
                                       itemIds.cbegin(), itemIds.cend(),
                                       [](const QXmppTuneItem &item, const QString &itemId) {
                                           return item.id() == itemId;
                                       });
    QVERIFY2(itemsEqual, "The items returned from the manager don't match the item IDs from the XML response");
}

void tst_QXmppPubSubManager::testRequestItemNotFound()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->requestItem("pubsub.qxmpp.org", "features", "item1");
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='features'><item id='item1'/></items></pubsub>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='features'/>"
                               "</pubsub></iq>"));
    const auto error = expectFutureVariant<QXmppStanza::Error>(future);
    QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
    QCOMPARE(error.condition(), QXmppStanza::Error::ItemNotFound);
}

void tst_QXmppPubSubManager::testRequestNodeAffiliations()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->requestNodeAffiliations("pubsub.qxmpp.org", "news");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<affiliations node='news'/>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq id='qxmpp1' type='result' from='pubsub.shakespeare.lit'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                               "<affiliations node='news'>"
                               "<affiliation jid='hamlet@denmark.lit' affiliation='owner'/>"
                               "<affiliation jid='polonius@denmark.lit' affiliation='outcast'/>"
                               "</affiliations></pubsub></iq>"));
    const auto affiliations = expectFutureVariant<QVector<Affiliation>>(future);

    QCOMPARE(affiliations.size(), 2);
    QCOMPARE(affiliations[0].node(), QString());
    QCOMPARE(affiliations[0].jid(), QString("hamlet@denmark.lit"));
    QCOMPARE(affiliations[0].type(), AffiliationType::Owner);
    QCOMPARE(affiliations[1].node(), QString());
    QCOMPARE(affiliations[1].jid(), QString("polonius@denmark.lit"));
    QCOMPARE(affiliations[1].type(), AffiliationType::Outcast);
}

void tst_QXmppPubSubManager::testRequestAffiliations()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->requestAffiliations("pubsub.qxmpp.org");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'><affiliations/></pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'><affiliations>"
                               "<affiliation node='node1' affiliation='owner'/>"
                               "<affiliation node='node2' affiliation='publisher'/>"
                               "<affiliation node='node5' affiliation='outcast'/>"
                               "<affiliation node='node6' affiliation='owner'/>"
                               "</affiliations></pubsub></iq>"));

    const auto affiliations = expectFutureVariant<QVector<Affiliation>>(future);
    QCOMPARE(affiliations.size(), 4);
    QCOMPARE(affiliations[3].node(), QString("node6"));
    QCOMPARE(affiliations[3].jid(), QString());
    QCOMPARE(affiliations[3].type(), AffiliationType::Owner);
}

void tst_QXmppPubSubManager::testRequestAffiliationsNode()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->requestAffiliations("pubsub.qxmpp.org", "node6");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'><affiliations node='node6'/></pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'><affiliations>"
                               "<affiliation node='node6' affiliation='owner'/>"
                               "</affiliations></pubsub></iq>"));

    const auto affiliations = expectFutureVariant<QVector<Affiliation>>(future);
    QCOMPARE(affiliations.size(), 1);
    QCOMPARE(affiliations[0].node(), QString("node6"));
    QCOMPARE(affiliations[0].jid(), QString());
    QCOMPARE(affiliations[0].type(), AffiliationType::Owner);
}

void tst_QXmppPubSubManager::testRequestOptions()
{
    using PresenceStates = QXmppPubSubSubscribeOptions::PresenceState;

    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();

    auto testOpts = [&](QFuture<PSManager::OptionsResult> &&future) {
        test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'><pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                    "<options jid='me@qxmpp.org' node='node1'/>"
                    "</pubsub></iq>");
        test.inject<QString>("<iq id='qxmpp1' from='pubsub.shakespeare.lit' type='result'>"
                             "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                             "<options node='princely_musings' jid='francisco@denmark.lit'>"
                             "<x xmlns='jabber:x:data' type='form'>"
                             "<field var='FORM_TYPE' type='hidden'><value>http://jabber.org/protocol/pubsub#subscribe_options</value></field>"
                             "<field var='pubsub#deliver' type='boolean' label='Enable delivery?'><value>1</value></field>"
                             "<field var='pubsub#digest' type='boolean' label='Receive digest notifications (approx. one per day)?'><value>0</value></field>"
                             "<field var='pubsub#include_body' type='boolean' label='Receive message body in addition to payload?'><value>false</value></field>"
                             "<field var='pubsub#show-values' type='list-multi' label='Select the presence types which are allowed to receive event notifications'>"
                             "<option label='Want to Chat'><value>chat</value></option>"
                             "<option label='Available'><value>online</value></option>"
                             "<option label='Away'><value>away</value></option>"
                             "<option label='Extended Away'><value>xa</value></option>"
                             "<option label='Do Not Disturb'><value>dnd</value></option>"
                             "<value>chat</value>"
                             "<value>online</value></field>"
                             "</x></options></pubsub></iq>");
        const auto form = expectFutureVariant<QXmppPubSubSubscribeOptions>(future);

        QCOMPARE(form.notificationsEnabled().value(), true);
        QCOMPARE(form.digestsEnabled().value(), false);
        QCOMPARE(form.bodyIncluded().value(), false);
        QCOMPARE(form.notificationRules(), PresenceStates::Chat | PresenceStates::Online);
        QCOMPARE(form.unknownFields().size(), 0);
    };

    testOpts(psManager->requestSubscribeOptions("pubsub.qxmpp.org", "node1", "me@qxmpp.org"));

    test.configuration().setJid("me@qxmpp.org");
    testOpts(psManager->requestSubscribeOptions("pubsub.qxmpp.org", "node1"));
}

void tst_QXmppPubSubManager::testRequestOptionsError()
{
    TestClient test;
    auto *psManager = test.addNewExtension<PSManager>();
    auto future = psManager->requestSubscribeOptions("pubsub.qxmpp.org", "node1", "me@qxmpp.org");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'><pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                "<options jid='me@qxmpp.org' node='node1'/>"
                "</pubsub></iq>");
    test.inject<QString>("<iq id='qxmpp1' from='pubsub.shakespeare.lit' type='result'>"
                         "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                         "<options node='princely_musings' jid='francisco@denmark.lit'>"
                         "<x xmlns='jabber:x:data' type='form'>"
                         "<field var='FORM_TYPE' type='hidden'><value>urn:xmpp:invlid:pubsub#subscribe_options</value></field>"
                         "<field var='pubsub#deliver' type='boolean' label='Enable delivery?'><value>1</value></field>"
                         "<field var='pubsub#digest' type='boolean' label='Receive digest notifications (approx. one per day)?'><value>0</value></field>"
                         "<field var='pubsub#include_body' type='boolean' label='Receive message body in addition to payload?'><value>false</value></field>"
                         "<field var='pubsub#show-values' type='list-multi' label='Select the presence types which are allowed to receive event notifications'>"
                         "<option label='Want to Chat'><value>chat</value></option>"
                         "<option label='Available'><value>online</value></option>"
                         "<option label='Away'><value>away</value></option>"
                         "<option label='Extended Away'><value>xa</value></option>"
                         "<option label='Do Not Disturb'><value>dnd</value></option>"
                         "<value>chat</value>"
                         "<value>online</value></field>"
                         "</x></options></pubsub></iq>");
    const auto error = expectFutureVariant<QXmppStanza::Error>(future);
    QCOMPARE(error.type(), QXmppStanza::Error::Cancel);
    QCOMPARE(error.condition(), QXmppStanza::Error::InternalServerError);
    QVERIFY(!error.text().isEmpty());
}

void tst_QXmppPubSubManager::testSetOptions()
{
    using PresenceStates = QXmppPubSubSubscribeOptions::PresenceState;

    TestClient test;
    test.configuration().setJid("francisco@denmark.lit");
    auto *psManager = test.addNewExtension<PSManager>();

    QXmppPubSubSubscribeOptions opts;
    opts.setNotificationsEnabled(true);
    opts.setDigestsEnabled(false);
    opts.setBodyIncluded(false);
    opts.setNotificationRules(PresenceStates::Chat | PresenceStates::Online | PresenceStates::Away);

    auto future = psManager->setSubscribeOptions("pubsub.shakespeare.lit", "princely_musings", opts);
    test.expect("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'><options jid='francisco@denmark.lit' node='princely_musings'>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>http://jabber.org/protocol/pubsub#subscribe_options</value></field>"
                "<field type=\"boolean\" var='pubsub#deliver'><value>1</value></field>"
                "<field type=\"boolean\" var='pubsub#digest'><value>0</value></field>"
                "<field type=\"boolean\" var='pubsub#include_body'><value>0</value></field>"
                "<field type=\"list-multi\" var='pubsub#show-values'><value>away</value><value>chat</value><value>online</value></field>"
                "</x></options></pubsub></iq>");
    test.inject<QString>("<iq id='qxmpp1' type='result'/>");

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testEventNotifications_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<bool>("accepted");

    QTest::addRow("default")
            << QStringLiteral(
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
                   "</message>")
            << true;
    QTest::addRow("additional-subelement")
            << QStringLiteral(
                   "<message from='pubsub.shakespeare.lit' to='francisco@denmark.lit' id='foo'>"
                   "<always-store xmlns='hints2'/>"
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
                   "</message>")
            << true;
    QTest::addRow("wrong-event-namespace")
            << QStringLiteral(
                   "<message from='pubsub.shakespeare.lit' to='francisco@denmark.lit' id='foo'>"
                   "<always-store xmlns='hints2'/>"
                   "<event xmlns='pubsub2#event'>"
                   "<items node='princely_musings'>"
                   "<item id='ae890ac52d0df67ed7cfdf51b644e901'>"
                   "<entry xmlns='http://www.w3.org/2005/Atom'>"
                   "<title>Soliloquy</title>"
                   "</entry>"
                   "</item>"
                   "</items>"
                   "</event>"
                   "</message>")
            << false;
}

void tst_QXmppPubSubManager::testEventNotifications()
{
    QFETCH(QString, xml);
    QFETCH(bool, accepted);
    const auto event = xmlToDom(xml);

    TestClient client;
    auto *psManager = client.addNewExtension<QXmppPubSubManager>();
    auto *eventManager = client.addNewExtension<TestEventManager>();
    eventManager->m_node = "princely_musings";
    eventManager->m_serviceJid = "pubsub.shakespeare.lit";

    QCOMPARE(psManager->handleStanza(event), accepted);
    if (accepted) {
        QCOMPARE(eventManager->m_events, 1u);
    } else {
        QCOMPARE(eventManager->m_events, 0u);
    }

    QCOMPARE(eventManager->pubSub(), psManager);
}

QTEST_MAIN(tst_QXmppPubSubManager)
#include "tst_qxmpppubsubmanager.moc"
