// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRosterIq.h"

#include "util.h"

#include <QObject>

class tst_QXmppRosterIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testItem_data();
    Q_SLOT void testItem();
    Q_SLOT void testApproved_data();
    Q_SLOT void testApproved();
    Q_SLOT void testVersion_data();
    Q_SLOT void testVersion();
    Q_SLOT void testMixAnnotate();
    Q_SLOT void testMixChannel();
};

void tst_QXmppRosterIq::testItem_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("subscriptionStatus");
    QTest::addColumn<int>("subscriptionType");
    QTest::addColumn<bool>("approved");

    QTest::newRow("none")
        << QByteArray(R"(<item jid="foo@example.com" subscription="none" approved="true"/>)")
        << ""
        << ""
        << int(QXmppRosterIq::Item::None)
        << true;
    QTest::newRow("from")
        << QByteArray(R"(<item jid="foo@example.com" subscription="from"/>)")
        << ""
        << ""
        << int(QXmppRosterIq::Item::From)
        << false;
    QTest::newRow("to")
        << QByteArray(R"(<item jid="foo@example.com" subscription="to"/>)")
        << ""
        << ""
        << int(QXmppRosterIq::Item::To)
        << false;
    QTest::newRow("both")
        << QByteArray(R"(<item jid="foo@example.com" subscription="both"/>)")
        << ""
        << ""
        << int(QXmppRosterIq::Item::Both)
        << false;
    QTest::newRow("remove")
        << QByteArray(R"(<item jid="foo@example.com" subscription="remove"/>)")
        << ""
        << ""
        << int(QXmppRosterIq::Item::Remove)
        << false;
    QTest::newRow("notset")
        << QByteArray("<item jid=\"foo@example.com\"/>")
        << ""
        << ""
        << int(QXmppRosterIq::Item::NotSet)
        << false;

    QTest::newRow("ask-subscribe")
        << QByteArray("<item jid=\"foo@example.com\" ask=\"subscribe\"/>")
        << ""
        << "subscribe"
        << int(QXmppRosterIq::Item::NotSet)
        << false;
    QTest::newRow("ask-unsubscribe")
        << QByteArray("<item jid=\"foo@example.com\" ask=\"unsubscribe\"/>")
        << ""
        << "unsubscribe"
        << int(QXmppRosterIq::Item::NotSet)
        << false;

    QTest::newRow("name")
        << QByteArray(R"(<item jid="foo@example.com" name="foo bar"/>)")
        << "foo bar"
        << ""
        << int(QXmppRosterIq::Item::NotSet)
        << false;
}

void tst_QXmppRosterIq::testItem()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, name);
    QFETCH(QString, subscriptionStatus);
    QFETCH(int, subscriptionType);
    QFETCH(bool, approved);

    QXmppRosterIq::Item item;
    parsePacket(item, xml);
    QCOMPARE(item.bareJid(), QLatin1String("foo@example.com"));
    QCOMPARE(item.groups(), QSet<QString>());
    QCOMPARE(item.name(), name);
    QCOMPARE(item.subscriptionStatus(), subscriptionStatus);
    QCOMPARE(int(item.subscriptionType()), subscriptionType);
    QCOMPARE(item.isApproved(), approved);
    serializePacket(item, xml);

    item = QXmppRosterIq::Item();
    item.setBareJid("foo@example.com");
    item.setName(name);
    item.setSubscriptionStatus(subscriptionStatus);
    item.setSubscriptionType(QXmppRosterIq::Item::SubscriptionType(subscriptionType));
    item.setIsApproved(approved);
    serializePacket(item, xml);
}

void tst_QXmppRosterIq::testApproved_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("approved");

    QTest::newRow("true") << QByteArray(R"(<item jid="foo@example.com" approved="true"/>)") << true;
    QTest::newRow("1") << QByteArray(R"(<item jid="foo@example.com" approved="1"/>)") << true;
    QTest::newRow("false") << QByteArray(R"(<item jid="foo@example.com" approved="false"/>)") << false;
    QTest::newRow("0") << QByteArray(R"(<item jid="foo@example.com" approved="0"/>)") << false;
    QTest::newRow("empty") << QByteArray(R"(<item jid="foo@example.com"/>)") << false;
}

void tst_QXmppRosterIq::testApproved()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, approved);

    QXmppRosterIq::Item item;
    parsePacket(item, xml);
    QCOMPARE(item.isApproved(), approved);
}

void tst_QXmppRosterIq::testVersion_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("version");

    QTest::newRow("noversion")
        << QByteArray(R"(<iq id="woodyisacat" to="woody@zam.tw/cat" type="result"><query xmlns="jabber:iq:roster"/></iq>)")
        << "";

    QTest::newRow("version")
        << QByteArray(R"(<iq id="woodyisacat" to="woody@zam.tw/cat" type="result"><query xmlns="jabber:iq:roster" ver="3345678"/></iq>)")
        << "3345678";
}

void tst_QXmppRosterIq::testVersion()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, version);

    QXmppRosterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.version(), version);
    serializePacket(iq, xml);
}

void tst_QXmppRosterIq::testMixAnnotate()
{
    const QByteArray xml(
        "<iq from=\"juliet@example.com/balcony\" "
        "type=\"get\">"
        "<query xmlns=\"jabber:iq:roster\">"
        "<annotate xmlns=\"urn:xmpp:mix:roster:0\"/>"
        "</query>"
        "</iq>");

    QXmppRosterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.mixAnnotate(), true);
    serializePacket(iq, xml);

    iq.setMixAnnotate(false);
    QCOMPARE(iq.mixAnnotate(), false);
}

void tst_QXmppRosterIq::testMixChannel()
{
    const QByteArray xml(
        "<item jid=\"balcony@example.net\">"
        "<channel xmlns=\"urn:xmpp:mix:roster:0\" participant-id=\"123456\"/>"
        "</item>");

    QXmppRosterIq::Item item;
    parsePacket(item, xml);
    QCOMPARE(item.isMixChannel(), true);
    QCOMPARE(item.mixParticipantId(), u"123456"_s);
    serializePacket(item, xml);

    item.setIsMixChannel(false);
    QCOMPARE(item.isMixChannel(), false);
    item.setMixParticipantId("23a7n");
    QCOMPARE(item.mixParticipantId(), u"23a7n"_s);
}

QTEST_MAIN(tst_QXmppRosterIq)
#include "tst_qxmpprosteriq.moc"
