// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDataForm.h"
#include "QXmppPubSubBaseItem.h"
#include "QXmppPubSubEvent.h"

#include "pubsubutil.h"
#include "util.h"
#include <QObject>

Q_DECLARE_METATYPE(std::optional<QXmppPubSubSubscription>)
Q_DECLARE_METATYPE(std::optional<QXmppDataForm>)

class tst_QXmppPubSubEvent : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic_data();
    Q_SLOT void testBasic();
    Q_SLOT void testCustomItem();
};

void tst_QXmppPubSubEvent::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QXmppPubSubEventBase::EventType>("eventType");
    QTest::addColumn<QString>("node");
    QTest::addColumn<QStringList>("retractIds");
    QTest::addColumn<QString>("redirectUri");
    QTest::addColumn<std::optional<QXmppPubSubSubscription>>("subscription");
    QTest::addColumn<QVector<QXmppPubSubBaseItem>>("items");
    QTest::addColumn<std::optional<QXmppDataForm>>("configurationForm");

#define ROW(name, xml, type, node, retractIds, redirectUri, subscription, items, configForm) \
    QTest::newRow(name) << QByteArrayLiteral(xml)                                            \
                        << type                                                              \
                        << node                                                              \
                        << (retractIds)                                                      \
                        << redirectUri                                                       \
                        << static_cast<std::optional<QXmppPubSubSubscription>>(subscription) \
                        << (items)                                                           \
                        << static_cast<std::optional<QXmppDataForm>>(configForm)

    ROW("items",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<items node=\"princely_musings\">"
        "<item id=\"ae890ac52d0df67ed7cfdf51b644e901\"/>"
        "</items>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Items,
        "princely_musings",
        QStringList(),
        QString(),
        std::nullopt,
        QVector<QXmppPubSubBaseItem>() << QXmppPubSubBaseItem("ae890ac52d0df67ed7cfdf51b644e901"),
        std::nullopt);

    ROW("retract",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<items node=\"princely_musings\">"
        "<retract id=\"ae890ac52d0df67ed7cfdf51b644e901\"/>"
        "<retract id=\"34324897shdfjk948577342343243243\"/>"
        "</items>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Retract,
        "princely_musings",
        QStringList() << "ae890ac52d0df67ed7cfdf51b644e901"
                      << "34324897shdfjk948577342343243243",
        QString(),
        std::nullopt,
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

    ROW("configuration-notify",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<configuration node=\"princely_musings\"/>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Configuration,
        "princely_musings",
        QStringList(),
        QString(),
        std::nullopt,
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

    ROW("configuration",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<configuration node=\"princely_musings\">"
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>http://jabber.org/protocol/pubsub#node_config</value>"
        "</field>"
        "<field type=\"text-single\" var=\"pubsub#title\">"
        "<value>Princely Musings (Atom)</value>"
        "</field>"
        "</x>"
        "</configuration>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Configuration,
        "princely_musings",
        QStringList(),
        QString(),
        std::nullopt,
        QVector<QXmppPubSubBaseItem>(),
        QXmppDataForm(QXmppDataForm::Result,
                      QList<QXmppDataForm::Field>()
                          << QXmppDataForm::Field(QXmppDataForm::Field::HiddenField,
                                                  "FORM_TYPE",
                                                  "http://jabber.org/protocol/pubsub#node_config")
                          << QXmppDataForm::Field(QXmppDataForm::Field::TextSingleField,
                                                  "pubsub#title",
                                                  "Princely Musings (Atom)")));

    ROW("purge",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<purge node=\"princely_musings\"/>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Purge,
        "princely_musings",
        QStringList(),
        QString(),
        std::nullopt,
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

    ROW("subscription-subscribed",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<subscription jid=\"horatio@denmark.lit\" node=\"princely_musings\" subscription=\"subscribed\"/>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Subscription,
        QString(),
        QStringList(),
        QString(),
        QXmppPubSubSubscription("horatio@denmark.lit", "princely_musings", {}, QXmppPubSubSubscription::Subscribed),
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

    ROW("subscription-none",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<subscription jid=\"polonius@denmark.lit\" node=\"princely_musings\" subscription=\"none\"/>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Subscription,
        QString(),
        QStringList(),
        QString(),
        QXmppPubSubSubscription("polonius@denmark.lit", "princely_musings", {}, QXmppPubSubSubscription::None),
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

    ROW("subscription-expiry",
        "<message id=\"foo\" type=\"normal\">"
        "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
        "<subscription jid=\"francisco@denmark.lit\" node=\"princely_musings\" subscription=\"subscribed\" subid=\"ba49252aaa4f5d320c24d3766f0bdcade78c78d3\" expiry=\"2006-02-28T23:59:59Z\"/>"
        "</event>"
        "</message>",
        QXmppPubSubEventBase::Subscription,
        QString(),
        QStringList(),
        QString(),
        QXmppPubSubSubscription("francisco@denmark.lit",
                                "princely_musings",
                                "ba49252aaa4f5d320c24d3766f0bdcade78c78d3",
                                QXmppPubSubSubscription::Subscribed,
                                QXmppPubSubSubscription::Unavailable,
                                QDateTime({ 2006, 02, 28 }, { 23, 59, 59 }, Qt::UTC)),
        QVector<QXmppPubSubBaseItem>(),
        std::nullopt);

#undef ROW
}

void tst_QXmppPubSubEvent::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(QXmppPubSubEventBase::EventType, eventType);
    QFETCH(QString, node);
    QFETCH(QStringList, retractIds);
    QFETCH(QString, redirectUri);
    QFETCH(std::optional<QXmppPubSubSubscription>, subscription);
    QFETCH(QVector<QXmppPubSubBaseItem>, items);
    QFETCH(std::optional<QXmppDataForm>, configurationForm);

    // parse
    QVERIFY(QXmppPubSubEvent<>::isPubSubEvent(xmlToDom(xml)));
    QXmppPubSubEvent event;
    parsePacket(event, xml);

    QCOMPARE(event.eventType(), eventType);
    QCOMPARE(event.node(), node);
    QCOMPARE(event.retractIds(), retractIds);
    QCOMPARE(event.redirectUri(), redirectUri);
    QCOMPARE(event.subscription().has_value(), subscription.has_value());
    if (subscription) {
        QCOMPARE(event.subscription()->jid(), subscription->jid());
        QCOMPARE(event.subscription()->node(), subscription->node());
        QCOMPARE(event.subscription()->state(), subscription->state());
        QCOMPARE(event.subscription()->subId(), subscription->subId());
        QCOMPARE(event.subscription()->expiry(), subscription->expiry());
    }
    QCOMPARE(event.items().count(), items.count());
    for (int i = 0; i < items.size(); i++) {
        QCOMPARE(event.items().at(i).id(), items.at(i).id());
        QCOMPARE(event.items().at(i).publisher(), items.at(i).publisher());
    }
    QCOMPARE(event.configurationForm().has_value(), configurationForm.has_value());
    if (configurationForm) {
        const auto parsedConfig = event.configurationForm();
        QCOMPARE(parsedConfig->fields().count(), configurationForm->fields().count());
        for (int i = 0; i < configurationForm->fields().count(); i++) {
            QCOMPARE(parsedConfig->fields().at(i).key(), configurationForm->fields().at(i).key());
            QCOMPARE(parsedConfig->fields().at(i).value(), configurationForm->fields().at(i).value());
            QCOMPARE(parsedConfig->fields().at(i).type(), configurationForm->fields().at(i).type());
        }
    }

    // serialize from parsed
    serializePacket(event, xml);

    // serialize from setters
    event = QXmppPubSubEvent();
    event.setId("foo");
    event.setEventType(eventType);
    event.setNode(node);
    event.setRetractIds(retractIds);
    event.setRedirectUri(redirectUri);
    event.setSubscription(subscription);
    event.setItems(items);
    event.setConfigurationForm(configurationForm);

    serializePacket(event, xml);
}

void tst_QXmppPubSubEvent::testCustomItem()
{
    const QByteArray xml = "<message id=\"foo\" type=\"normal\">"
                           "<event xmlns=\"http://jabber.org/protocol/pubsub#event\">"
                           "<items node=\"princely_musings\">"
                           "<item id=\"42\"><test-payload/></item>"
                           "<item id=\"23\"><test-payload/></item>"
                           "</items>"
                           "</event>"
                           "</message>";

    // test isPubSubIq also checks item validity
    TestItem::isItemCalled = false;
    QVERIFY(QXmppPubSubEvent<TestItem>::isPubSubEvent(xmlToDom(xml)));
    QVERIFY(TestItem::isItemCalled);

    QXmppPubSubEvent<TestItem> event;
    parsePacket(event, xml);

    QCOMPARE(event.id(), QString("foo"));
    QCOMPARE(event.eventType(), QXmppPubSubEvent<>::Items);
    QCOMPARE(event.node(), QString("princely_musings"));
    QCOMPARE(event.items().count(), 2);
    QCOMPARE(event.items().at(0).id(), QString::number(42));
    QCOMPARE(event.items().at(1).id(), QString::number(23));
    QCOMPARE(event.items().at(0).publisher(), QString());
    QCOMPARE(event.items().at(1).publisher(), QString());
    QVERIFY(event.items().at(0).parseCalled);
    QVERIFY(event.items().at(1).parseCalled);
    QVERIFY(!event.items().at(0).serializeCalled);
    QVERIFY(!event.items().at(1).serializeCalled);

    // serialize from parsed
    serializePacket(event, xml);

    QVERIFY(event.items().at(0).serializeCalled);
    QVERIFY(event.items().at(1).serializeCalled);

    // serialize from setters
    event = QXmppPubSubEvent<TestItem>();
    event.setId("foo");
    event.setEventType(QXmppPubSubEvent<>::Items);
    event.setNode("princely_musings");
    event.setItems({ TestItem("42"), TestItem("23") });
    serializePacket(event, xml);
}

QTEST_MAIN(tst_QXmppPubSubEvent)
#include "tst_qxmpppubsubevent.moc"
