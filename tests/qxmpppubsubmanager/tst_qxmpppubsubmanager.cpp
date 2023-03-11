// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppPubSubAffiliation.h"
#include "QXmppPubSubBaseItem.h"
#include "QXmppPubSubEventHandler.h"
#include "QXmppPubSubManager.h"
#include "QXmppPubSubPublishOptions.h"
#include "QXmppPubSubSubscribeOptions.h"
#include "QXmppUserTuneItem.h"

#include "TestClient.h"
#include "util.h"
#include <QObject>

Q_DECLARE_METATYPE(QXmpp::Private::PubSubIq<>);
Q_DECLARE_METATYPE(std::optional<QXmppPubSubPublishOptions>);

using PSManager = QXmppPubSubManager;
using Affiliation = QXmppPubSubAffiliation;
using AffiliationType = QXmppPubSubAffiliation::Affiliation;

const char *ns_pubsub = "http://jabber.org/protocol/pubsub";
const char *ns_pubsub_auto_create = "http://jabber.org/protocol/pubsub#auto-create";

class TestEventManager : public QXmppClientExtension, public QXmppPubSubEventHandler
{
public:
    QXmppPubSubManager *pubSub()
    {
        return client()->findExtension<QXmppPubSubManager>();
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

struct Client
{
    Client() { }
    Client(const QString &jid)
    {
        test.configuration().setJid(jid);
    }

    TestClient test;
    PSManager *psManager = test.addNewExtension<PSManager>();
};

class tst_QXmppPubSubManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testDiscoFeatures();
    Q_SLOT void testRequestFeatures();
    Q_SLOT void testRequestPepFeatures();
    Q_SLOT void testFetchNodes();
    Q_SLOT void testFetchPepNodes();
    Q_SLOT void testCreateNodes_data();
    Q_SLOT void testCreateNodes();
    Q_SLOT void testCreateNodeWithConfig();
    Q_SLOT void testCreateInstantNode();
    Q_SLOT void testCreateInstantNodeWithConfig();
    Q_SLOT void testDeleteNodes_data();
    Q_SLOT void testDeleteNodes();
    Q_SLOT void testPublishItems_data();
    Q_SLOT void testPublishItems();
    Q_SLOT void testRetractCurrentItem();
    Q_SLOT void testRetractItem_data();
    Q_SLOT void testRetractItem();
    Q_SLOT void testRetractCurrentPepItem();
    Q_SLOT void testPurgeItems();
    Q_SLOT void testPurgePepItems();
    Q_SLOT void testRequestItemIds();
    Q_SLOT void testRequestPepItemIds();
    Q_SLOT void testRequestCurrentItem();
    Q_SLOT void testRequestItems_data();
    Q_SLOT void testRequestItems();
    Q_SLOT void testRequestCurrentPepItem();
    Q_SLOT void testRequestPepItem();
    Q_SLOT void testRequestPepItems();
    Q_SLOT void testRequestItemNotFound();
    Q_SLOT void testRequestNodeAffiliations();
    Q_SLOT void testRequestAffiliations();
    Q_SLOT void testRequestAffiliationsNode();
    Q_SLOT void testRequestOptions();
    Q_SLOT void testRequestOptionsError();
    Q_SLOT void testSetOptions();
    Q_SLOT void testRequestNodeConfig();
    Q_SLOT void testConfigureNode();
    Q_SLOT void testCancelConfig();
    Q_SLOT void testSubscribeToNode();
    Q_SLOT void testUnsubscribeFromNode();
    Q_SLOT void testEventNotifications_data();
    Q_SLOT void testEventNotifications();
    Q_SLOT void testStandardItemToString();
};

void tst_QXmppPubSubManager::testDiscoFeatures()
{
    // so the coverage report is happy:
    PSManager manager;
    QCOMPARE(manager.discoveryFeatures(), QStringList { "http://jabber.org/protocol/pubsub#rsm" });
}

void tst_QXmppPubSubManager::testRequestFeatures()
{
    auto [test, psManager] = Client();

    auto future = psManager->requestFeatures("pubsub.shakespeare.lit");
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    expectFutureVariant<QXmppPubSubManager::InvalidServiceType>(future);

    future = psManager->requestFeatures("pubsub.shakespeare.lit");
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='service'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    auto features = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(features, (QVector<QString> { ns_pubsub, ns_pubsub_auto_create }));

    future = psManager->requestFeatures("juliet@capulet.lit");
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='pep'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    features = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(features, (QVector<QString> { ns_pubsub, ns_pubsub_auto_create }));

    future = psManager->requestFeatures("juliet@capulet.lit", QXmppPubSubManager::PubSub);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='pep'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    expectFutureVariant<QXmppPubSubManager::InvalidServiceType>(future);

    future = psManager->requestFeatures("pubsub.shakespeare.lit", QXmppPubSubManager::PubSub);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='service'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    features = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(features, (QVector<QString> { ns_pubsub, ns_pubsub_auto_create }));

    future = psManager->requestFeatures("pubsub.shakespeare.lit", QXmppPubSubManager::Pep);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='service'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    expectFutureVariant<QXmppPubSubManager::InvalidServiceType>(future);

    future = psManager->requestFeatures("juliet@capulet.lit", QXmppPubSubManager::Pep);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='pep'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    features = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(features, (QVector<QString> { ns_pubsub, ns_pubsub_auto_create }));
}

void tst_QXmppPubSubManager::testRequestPepFeatures()
{
    auto [test, psManager] = Client();
    test.configuration().setJid("juliet@capulet.lit");

    auto future = psManager->requestOwnPepFeatures();
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq type='result' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' id='qxmpp1'>"
                               "<query xmlns='http://jabber.org/protocol/disco#info'>"
                               "<identity category='pubsub' type='pep'/>"
                               "<feature var='http://jabber.org/protocol/pubsub'/>"
                               "<feature var='http://jabber.org/protocol/pubsub#auto-create'/>"
                               "</query></iq>"));

    auto features = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(features, (QVector<QString> { ns_pubsub, ns_pubsub_auto_create }));
}

void tst_QXmppPubSubManager::testFetchNodes()
{
    auto [test, psManager] = Client();

    auto future = psManager->requestNodes("pubsub.shakespeare.lit");
    test.expect("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>");
    test.inject(QStringLiteral("<iq type='result' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' id='qxmpp1'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items'>"
                               "<item jid='pubsub.shakespeare.lit' node='blogs' name='Weblog updates'/>"
                               "<item jid='pubsub.shakespeare.lit' node='news' name='News and announcements'/>"
                               "</query></iq>"));

    const auto nodes = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(nodes, (QVector<QString> { "blogs", "news" }));
}

void tst_QXmppPubSubManager::testFetchPepNodes()
{
    auto [test, psManager] = Client();
    test.configuration().setJid("juliet@capulet.lit");

    auto future = psManager->requestOwnPepNodes();
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items'>"
                               "<item jid='juliet@capulet.lit' node='blogs' name='Weblog updates'/>"
                               "<item jid='juliet@capulet.lit' node='news' name='News and announcements'/>"
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

    auto [test, psManager] = Client();

    QXmppTask<PSManager::Result> future = [=, &t = test, psM = psManager]() {
        if (isPep) {
            t.configuration().setJid(jid);
            return psM->createOwnPepNode(node);
        } else {
            return psM->createNode(jid, node);
        }
    }();

    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub'><create node='%2'/></pubsub></iq>").arg(jid, node));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='%1' type='result'/>").arg(jid));
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testCreateNodeWithConfig()
{
    auto [test, psManager] = Client();
    QXmppPubSubNodeConfig config;
    config.setTitle("Princely Musings (Atom)");
    auto future = psManager->createNode("pubsub.qxmpp.org", "princely_musings", config);
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                "<create node='princely_musings'/>"
                "<configure>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>http://jabber.org/protocol/pubsub#node_config</value></field>"
                "<field type='text-single' var='pubsub#title'><value>Princely Musings (Atom)</value></field>"
                "</x>"
                "</configure>"
                "</pubsub>"
                "</iq>");
    test.inject<QString>("<iq id='qxmpp1' from='pubsub.qxmpp.org' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testCreateInstantNode()
{
    auto [test, psManager] = Client();
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

void tst_QXmppPubSubManager::testCreateInstantNodeWithConfig()
{
    auto [test, psManager] = Client();
    QXmppPubSubNodeConfig config;
    config.setTitle("Princely Musings (Atom)");
    auto future = psManager->createInstantNode("pubsub.qxmpp.org", config);
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                "<create/>"
                "<configure>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>http://jabber.org/protocol/pubsub#node_config</value></field>"
                "<field type='text-single' var='pubsub#title'><value>Princely Musings (Atom)</value></field>"
                "</x>"
                "</configure>"
                "</pubsub>"
                "</iq>");
    test.inject<QString>("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                         "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                         "<create node='25e3d37dabbab9541f7523321421edc5bfeb2dae'/>"
                         "</pubsub></iq>");
    QCOMPARE(expectFutureVariant<QString>(future), QStringLiteral("25e3d37dabbab9541f7523321421edc5bfeb2dae"));
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

    auto [test, psManager] = Client();
    if (isPep) {
        test.configuration().setJid(jid);
    }

    QXmppTask<PSManager::Result> future = [=, psM = psManager]() {
        if (isPep) {
            return psM->deleteOwnPepNode(node);
        } else {
            return psM->deleteNode(jid, node);
        }
    }();

    // FIXME: pubsub#owner here, but not for <create/>?
    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub#owner'><delete node='%2'/></pubsub></iq>").arg(jid, node));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='%1' type='result'/>").arg(jid));
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testPublishItems_data()
{
    using OptionsOpt = std::optional<QXmppPubSubPublishOptions>;
    QTest::addColumn<bool>("isPep");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QVector<QXmppPubSubBaseItem>>("items");
    QTest::addColumn<OptionsOpt>("publishOptions");
    QTest::addColumn<bool>("returnIds");

    QXmppTuneItem item1;
    item1.setId("1234");
    item1.setTitle("Hello Goodbye");

    QXmppTuneItem item2;
    item2.setId("5678");
    item2.setArtist("Rick Astley");
    item2.setTitle("Never gonna give you up");

    QVector<QXmppPubSubBaseItem> items1 { item1 };
    QVector<QXmppPubSubBaseItem> items2 { item1, item2 };

    QXmppPubSubPublishOptions publishOptions;
    publishOptions.setAccessModel(QXmppPubSubPublishOptions::Presence);

    auto addRow = [&](const char *name, bool isPep, QString &&jid,
                      QString &&node, const QVector<QXmppPubSubBaseItem> &items) {
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
    QFETCH(QVector<QXmppPubSubBaseItem>, items);
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
    PSManager *psManager = test.addNewExtension<PSManager>();

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
            test.inject(QStringLiteral("<iq id='qxmpp1' from='%1' type='result'/>").arg(jid));
        }
    };

    if (items.size() == 1) {
        QXmppTask<PSManager::PublishItemResult> future = [=, psM = psManager]() {
            if (isPep) {
                if (publishOptions) {
                    return psManager->publishOwnPepItem(node, items.constFirst(), *publishOptions);
                } else {
                    return psManager->publishOwnPepItem(node, items.constFirst());
                }
            } else {
                if (publishOptions) {
                    return psManager->publishItem(jid, node, items.constFirst(), *publishOptions);
                } else {
                    return psManager->publishItem(jid, node, items.constFirst());
                }
            }
        }();

        injectXml();
        const auto id = expectFutureVariant<QString>(future);
        if (returnIds) {
            QCOMPARE(id, items.constFirst().id());
        } else {
            QVERIFY(id.isNull());
        }
    } else {
        QXmppTask<PSManager::PublishItemsResult> future = [=, psM = psManager]() {
            if (isPep) {
                if (publishOptions) {
                    return psManager->publishOwnPepItems(node, items, *publishOptions);
                } else {
                    return psManager->publishOwnPepItems(node, items);
                }
            } else {
                if (publishOptions) {
                    return psManager->publishItems(jid, node, items, *publishOptions);
                } else {
                    return psManager->publishItems(jid, node, items);
                }
            }
        }();

        injectXml();
        const auto ids = expectFutureVariant<QVector<QString>>(future);
        if (returnIds) {
            QCOMPARE(ids, itemIds);
        } else {
            QVERIFY(ids.empty());
        }
    }
}

void tst_QXmppPubSubManager::testRetractCurrentItem()
{
    auto [test, psManager] = Client();

    auto future = psManager->retractItem(QStringLiteral("pubsub.shakespeare.lit"), QStringLiteral("princely_musings"), PSManager::Current);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='set'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<retract node='princely_musings'>"
                               "<item id='current'/>"
                               "</retract>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<retract node='princely_musings'>"
                               "<item id='current'/>"
                               "</retract>"
                               "</pubsub></iq>"));

    expectFutureVariant<QXmpp::Success>(future);
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

    auto [test, psManager] = Client();

    QXmppTask<PSManager::Result> future = [=, psM = psManager, &t = test]() {
        if (isPep) {
            t.configuration().setJid(jid);
            return psM->retractOwnPepItem(node, itemId);
        } else {
            return psM->retractItem(jid, node, itemId);
        }
    }();

    test.expect(QStringLiteral("<iq id='qxmpp1' to='%1' type='set'><pubsub xmlns='http://jabber.org/protocol/pubsub'><retract node='%2'><item id='%3'/></retract></pubsub></iq>")
                    .arg(jid, node, itemId));
    test.inject(QStringLiteral("<iq type='result' from='%1' id='qxmpp1'/>")
                    .arg(jid));

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testRetractCurrentPepItem()
{
    auto [test, psManager] = Client();
    test.configuration().setJid(QStringLiteral("juliet@capulet.lit"));

    auto future = psManager->retractOwnPepItem(QStringLiteral("princely_musings"), PSManager::Current);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='set'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<retract node='princely_musings'>"
                               "<item id='current'/>"
                               "</retract>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<retract node='princely_musings'>"
                               "<item id='current'/>"
                               "</retract>"
                               "</pubsub></iq>"));

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testPurgeItems()
{
    auto [test, psManager] = Client();
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
    auto [test, psManager] = Client("user@qxmpp.org");
    auto future = psManager->purgeOwnPepItems("urn:xmpp:x-avatar:0");
    test.expect("<iq id='qxmpp1' to='user@qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<purge node='urn:xmpp:x-avatar:0'/>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq type='result' from='user@qxmpp.org' id='qxmpp1'/>"));
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testRequestItemIds()
{
    auto [test, psManager] = Client();

    auto future = psManager->requestItemIds(QStringLiteral("pubsub.shakespeare.lit"), QStringLiteral("princely_musings"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items' node='princely_musings'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items' node='princely_musings'>"
                               "<item jid='pubsub.shakespeare.lit' name='368866411b877c30064a5f62b917cffe'/>"
                               "<item jid='pubsub.shakespeare.lit' name='3300659945416e274474e469a1f0154c'/>"
                               "</query></iq>"));

    auto itemIds = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(itemIds, (QVector<QString> { QStringLiteral("368866411b877c30064a5f62b917cffe"), QStringLiteral("3300659945416e274474e469a1f0154c") }));
}

void tst_QXmppPubSubManager::testRequestPepItemIds()
{
    auto [test, psManager] = Client();
    test.configuration().setJid(QStringLiteral("juliet@capulet.lit"));

    auto future = psManager->requestOwnPepItemIds(QStringLiteral("princely_musings"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items' node='princely_musings'/>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<query xmlns='http://jabber.org/protocol/disco#items' node='princely_musings'>"
                               "<item jid='juliet@capulet.lit' name='368866411b877c30064a5f62b917cffe'/>"
                               "<item jid='juliet@capulet.lit' name='3300659945416e274474e469a1f0154c'/>"
                               "</query></iq>"));

    auto itemIds = expectFutureVariant<QVector<QString>>(future);
    QCOMPARE(itemIds, (QVector<QString> { QStringLiteral("368866411b877c30064a5f62b917cffe"), QStringLiteral("3300659945416e274474e469a1f0154c") }));
}

void tst_QXmppPubSubManager::testRequestCurrentItem()
{
    auto [test, psManager] = Client();

    auto future = psManager->requestItem(QStringLiteral("pubsub.shakespeare.lit"), QStringLiteral("princely_musings"), PSManager::Current);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='current'/>"
                               "</items>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='current'/>"
                               "</items>"
                               "</pubsub></iq>"));

    const auto item = expectFutureVariant<QXmppPubSubBaseItem>(future);
    QCOMPARE(item.id(), QStringLiteral("current"));
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

    auto [test, psManager] = Client();
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
        QCOMPARE(*item.length(), uint16_t(686));
        QCOMPARE(*item.rating(), uint8_t(8));
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

void tst_QXmppPubSubManager::testRequestCurrentPepItem()
{
    auto [test, psManager] = Client();
    test.configuration().setJid(QStringLiteral("juliet@capulet.lit"));

    auto future = psManager->requestOwnPepItem(QStringLiteral("princely_musings"), PSManager::Current);
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='current'/>"
                               "</items>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='current'/>"
                               "</items>"
                               "</pubsub></iq>"));

    const auto item = expectFutureVariant<QXmppPubSubBaseItem>(future);
    QCOMPARE(item.id(), QStringLiteral("current"));
}

void tst_QXmppPubSubManager::testRequestPepItem()
{
    auto [test, psManager] = Client();
    test.configuration().setJid(QStringLiteral("juliet@capulet.lit"));

    auto future = psManager->requestOwnPepItem(QStringLiteral("princely_musings"), QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='ae890ac52d0df67ed7cfdf51b644e901'/>"
                               "</items>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='ae890ac52d0df67ed7cfdf51b644e901'/>"
                               "</items>"
                               "</pubsub></iq>"));

    const auto item = expectFutureVariant<QXmppPubSubBaseItem>(future);
    QCOMPARE(item.id(), QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
}

void tst_QXmppPubSubManager::testRequestPepItems()
{
    auto [test, psManager] = Client();
    test.configuration().setJid(QStringLiteral("juliet@capulet.lit"));

    auto future = psManager->requestOwnPepItems(QStringLiteral("princely_musings"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='juliet@capulet.lit' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'/>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='juliet@capulet.lit' to='juliet@capulet.lit/balcony' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='princely_musings'>"
                               "<item id='368866411b877c30064a5f62b917cffe'/>"
                               "<item id='3300659945416e274474e469a1f0154c'/>"
                               "</items>"
                               "</pubsub></iq>"));

    const auto items = expectFutureVariant<QXmppPubSubManager::Items<QXmppPubSubBaseItem>>(future);
    QCOMPARE(items.items.first().id(), QStringLiteral("368866411b877c30064a5f62b917cffe"));
    QCOMPARE(items.items.last().id(), QStringLiteral("3300659945416e274474e469a1f0154c"));
}

void tst_QXmppPubSubManager::testRequestItemNotFound()
{
    auto [test, psManager] = Client();
    auto future = psManager->requestItem("pubsub.qxmpp.org", "features", "item1");
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='features'><item id='item1'/></items></pubsub>"
                               "</iq>"));
    test.inject(QStringLiteral("<iq type='result' from='pubsub.qxmpp.org' id='qxmpp1'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<items node='features'/>"
                               "</pubsub></iq>"));
    auto error = expectFutureVariant<QXmppError>(future);
}

void tst_QXmppPubSubManager::testRequestNodeAffiliations()
{
    auto [test, psManager] = Client();
    auto future = psManager->requestNodeAffiliations("pubsub.qxmpp.org", "news");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<affiliations node='news'/>"
                "</pubsub></iq>");
    test.inject(QStringLiteral("<iq id='qxmpp1' type='result' from='pubsub.qxmpp.org'>"
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
    auto [test, psManager] = Client();
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
    auto [test, psManager] = Client();
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

    auto testOpts = [&](QXmppTask<PSManager::OptionsResult> &&future) {
        test.expect("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'><pubsub xmlns='http://jabber.org/protocol/pubsub'>"
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

    testOpts(psManager->requestSubscribeOptions("pubsub.shakespeare.lit", "node1", "me@qxmpp.org"));

    test.configuration().setJid("me@qxmpp.org");
    testOpts(psManager->requestSubscribeOptions("pubsub.shakespeare.lit", "node1"));
}

void tst_QXmppPubSubManager::testRequestOptionsError()
{
    auto [test, psManager] = Client();
    auto future = psManager->requestSubscribeOptions("pubsub.shakespeare.lit", "node1", "me@qxmpp.org");
    test.expect("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='get'><pubsub xmlns='http://jabber.org/protocol/pubsub'>"
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
    auto err = expectFutureVariant<QXmppError>(future);
    QVERIFY(!err.description.isEmpty());
}

void tst_QXmppPubSubManager::testSetOptions()
{
    using PresenceStates = QXmppPubSubSubscribeOptions::PresenceState;

    auto [test, psManager] = Client("francisco@denmark.lit");

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
    test.inject<QString>("<iq id='qxmpp1' from='pubsub.shakespeare.lit' type='result'/>");

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testRequestNodeConfig()
{
    auto [test, psManager] = Client();

    auto future = psManager->requestNodeConfiguration("pubsub.qxmpp.org", "tune");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='get'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<configure node='tune'/>"
                "</pubsub>"
                "</iq>");
    test.inject<QString>("<iq id='qxmpp1' from='pubsub.qxmpp.org' type='result'>"
                         "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                         "<configure node='princely_musings'>"
                         "<x xmlns='jabber:x:data' type='form'>"
                         "<field var='FORM_TYPE' type='hidden'><value>http://jabber.org/protocol/pubsub#node_config</value></field>"
                         "<field var='pubsub#title' type='text-single' label='A friendly name for the node'/>"
                         "<field var='pubsub#deliver_notifications' type='boolean' label='Whether to deliver event notifications'><value>true</value></field>"
                         "<field var='pubsub#deliver_payloads' type='boolean' label='Whether to deliver payloads with event notifications'><value>true</value></field>"
                         "<field var='pubsub#dataform_xslt' type='text-single' label='Payload XSLT'/>"
                         "</x>"
                         "</configure>"
                         "</pubsub>"
                         "</iq>");

    const auto form = expectFutureVariant<QXmppPubSubNodeConfig>(future);
    QCOMPARE(form.title(), QString());
    QVERIFY(form.notificationsEnabled().has_value());
    QCOMPARE(form.notificationsEnabled().value_or(false), true);
    QVERIFY(form.dataFormXslt().isNull());
}

void tst_QXmppPubSubManager::testConfigureNode()
{
    auto [test, psManager] = Client();

    QXmppPubSubNodeConfig config;
    config.setTitle("Princely Musings (Atom)");
    config.setNotificationsEnabled(true);
    config.setIncludePayloads(true);
    config.setPersistItems(true);
    config.setMaxItems(10ULL);
    config.setItemExpiry(604800);
    config.setAccessModel(QXmppPubSubNodeConfig::Roster);
    config.setAllowedRosterGroups({ "friends", "servants", "courtiers" });
    config.setPublishModel(QXmppPubSubNodeConfig::Publishers);
    config.setPurgeWhenOffline(false);
    config.setSendLastItem(QXmppPubSubNodeConfig::Never);
    config.setPresenceBasedNotifications(false);
    config.setNotificationType(QXmppPubSubNodeConfig::Headline);
    config.setConfigNotificationsEnabled(false);
    config.setDeleteNotificationsEnabled(false);
    config.setRetractNotificationsEnabled(false);
    config.setSubNotificationsEnabled(false);
    config.setMaxPayloadSize(1028);
    config.setPayloadType("http://www.w3.org/2005/Atom");
    config.setBodyXslt("http://jabxslt.jabberstudio.org/atom_body.xslt");

    auto future = psManager->configureNode("pubsub.shakespeare.lit", "princely_musings", config);
    test.expect("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<configure node='princely_musings'>"
                "<x xmlns='jabber:x:data' type='submit'>"
                "<field type='hidden' var='FORM_TYPE'><value>http://jabber.org/protocol/pubsub#node_config</value></field>"
                "<field type=\"list-single\" var='pubsub#access_model'><value>roster</value></field>"
                "<field type=\"text-single\" var='pubsub#body_xslt'><value>http://jabxslt.jabberstudio.org/atom_body.xslt</value></field>"
                "<field type=\"boolean\" var='pubsub#deliver_notifications'><value>1</value></field>"
                "<field type=\"boolean\" var='pubsub#deliver_payloads'><value>1</value></field>"
                "<field type=\"text-single\" var='pubsub#item_expire'><value>604800</value></field>"
                "<field type=\"text-single\" var='pubsub#max_items'><value>10</value></field>"
                "<field type=\"text-single\" var='pubsub#max_payload_size'><value>1028</value></field>"
                "<field type=\"list-single\" var='pubsub#notification_type'><value>headline</value></field>"
                "<field type=\"boolean\" var='pubsub#notify_config'><value>0</value></field>"
                "<field type=\"boolean\" var='pubsub#notify_delete'><value>0</value></field>"
                "<field type=\"boolean\" var='pubsub#notify_retract'><value>0</value></field>"
                "<field type=\"boolean\" var='pubsub#notify_sub'><value>0</value></field>"
                "<field type=\"boolean\" var='pubsub#persist_items'><value>1</value></field>"
                "<field type=\"boolean\" var='pubsub#presence_based_delivery'><value>0</value></field>"
                "<field type=\"list-single\" var='pubsub#publish_model'><value>publishers</value></field>"
                "<field type=\"boolean\" var='pubsub#purge_offline'><value>0</value></field>"
                "<field type=\"list-multi\" var='pubsub#roster_groups_allowed'><value>friends</value><value>servants</value><value>courtiers</value></field>"
                "<field type=\"list-single\" var='pubsub#send_last_published_item'><value>never</value></field>"
                "<field type=\"text-single\" var='pubsub#title'><value>Princely Musings (Atom)</value></field>"
                "<field type=\"text-single\" var='pubsub#type'><value>http://www.w3.org/2005/Atom</value></field>"
                "</x>"
                "</configure>"
                "</pubsub>"
                "</iq>");
    test.inject<QString>("<iq type='result' from='pubsub.shakespeare.lit' id='qxmpp1'/>");

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testCancelConfig()
{
    auto [test, psManager] = Client();
    auto future = psManager->cancelNodeConfiguration("pubsub.qxmpp.org", "nodeee");
    test.expect("<iq id='qxmpp1' to='pubsub.qxmpp.org' type='set'>"
                "<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>"
                "<configure node='nodeee'>"
                "<x xmlns='jabber:x:data' type='cancel'/>"
                "</configure>"
                "</pubsub>"
                "</iq>");
    test.inject<QString>("<iq id='qxmpp1' from='pubsub.qxmpp.org' type='result'/>");
    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testSubscribeToNode()
{
    auto [test, psManager] = Client();

    auto future = psManager->subscribeToNode(QStringLiteral("pubsub.shakespeare.lit"), QStringLiteral("princely_musings"), QStringLiteral("francisco@denmark.lit"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='set'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<subscribe jid='francisco@denmark.lit' node='princely_musings'/>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<subscription jid='francisco@denmark.lit' node='princely_musings' subid='ba49252aaa4f5d320c24d3766f0bdcade78c78d3' subscription='subscribed'/>"
                               "</pubsub></iq>"));

    expectFutureVariant<QXmpp::Success>(future);
}

void tst_QXmppPubSubManager::testUnsubscribeFromNode()
{
    auto [test, psManager] = Client();

    auto future = psManager->unsubscribeFromNode(QStringLiteral("pubsub.shakespeare.lit"), QStringLiteral("princely_musings"), QStringLiteral("francisco@denmark.lit"));
    test.expect(QStringLiteral("<iq id='qxmpp1' to='pubsub.shakespeare.lit' type='set'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<unsubscribe jid='francisco@denmark.lit' node='princely_musings'/>"
                               "</pubsub></iq>"));
    test.inject(QStringLiteral("<iq id='qxmpp1' from='pubsub.shakespeare.lit' to='francisco@denmark.lit/barracks' type='result'>"
                               "<pubsub xmlns='http://jabber.org/protocol/pubsub'>"
                               "<subscription jid='francisco@denmark.lit' node='princely_musings' subid='ba49252aaa4f5d320c24d3766f0bdcade78c78d3' subscription='none'/>"
                               "</pubsub></iq>"));

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

    auto [client, psManager] = Client();
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

void tst_QXmppPubSubManager::testStandardItemToString()
{
    auto standardItemString = PSManager::standardItemIdToString(PSManager::Current);
    QCOMPARE(standardItemString, QStringLiteral("current"));
}

QTEST_MAIN(tst_QXmppPubSubManager)
#include "tst_qxmpppubsubmanager.moc"
