// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixIq.h"

#include "util.h"
#include <QObject>

Q_DECLARE_METATYPE(QXmppIq::Type);
Q_DECLARE_METATYPE(QXmppMixIq::Type);

class tst_QXmppMixIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBase_data();
    Q_SLOT void testBase();
    Q_SLOT void testDefaults();
    Q_SLOT void testSetters();
    Q_SLOT void testInvalidActionType();
    Q_SLOT void testIsMixIq();
};

void tst_QXmppMixIq::testBase_data()
{
    QByteArray joinC2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<client-join xmlns=\"urn:xmpp:mix:pam:1\" channel=\"coven@mix.shakespeare.example\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:presence\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:participants\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<nick>third witch</nick>"
        "</join>"
        "</client-join>"
        "</iq>");
    QByteArray joinS2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"coven@mix.shakespeare.example\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"set\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:presence\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:participants\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<nick>stpeter</nick>"
        "</join>"
        "</iq>");
    QByteArray joinS2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"coven@mix.shakespeare.example\" "
        "type=\"result\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\" jid=\"123456#coven@mix.shakespeare.example\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:presence\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:participants\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<nick>third witch</nick>"
        "</join>"
        "</iq>");
    QByteArray joinC2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<client-join xmlns=\"urn:xmpp:mix:pam:1\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\" "
        "jid=\"123456#coven@mix.shakespeare.example\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:presence\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:participants\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "</join>"
        "</client-join>"
        "</iq>");
    QByteArray leaveC2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<client-leave xmlns=\"urn:xmpp:mix:pam:1\" channel=\"coven@mix.shakespeare.example\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</client-leave>"
        "</iq>");
    QByteArray leaveS2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"coven@mix.shakespeare.example\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"set\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</iq>");
    QByteArray leaveS2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"coven@mix.shakespeare.example\" "
        "type=\"result\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</iq>");
    QByteArray leaveC2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<client-leave xmlns=\"urn:xmpp:mix:pam:1\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</client-leave>"
        "</iq>");
    QByteArray updateSubscriptionSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<update-subscription xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "</update-subscription>"
        "</iq>");
    QByteArray updateSubscriptionResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<update-subscription xmlns=\"urn:xmpp:mix:core:1\" jid=\"hag66@shakespeare.example\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "</update-subscription>"
        "</iq>");
    QByteArray setNickSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<setnick xmlns=\"urn:xmpp:mix:core:1\">"
        "<nick>thirdwitch</nick>"
        "</setnick>"
        "</iq>");
    QByteArray setNickResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<setnick xmlns=\"urn:xmpp:mix:core:1\">"
        "<nick>thirdwitch</nick>"
        "</setnick>"
        "</iq>");
    QByteArray createXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<create xmlns=\"urn:xmpp:mix:core:1\" channel=\"coven\"/>"
        "</iq>");
    QByteArray createWithoutNameXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<create xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</iq>");
    QByteArray destroyXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<destroy xmlns=\"urn:xmpp:mix:core:1\" channel=\"coven\"/>"
        "</iq>");
    QByteArray emptyXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\"/>");

    QStringList emptyNodes;
    QStringList defaultNodes;
    defaultNodes << "urn:xmpp:mix:nodes:messages"
                 << "urn:xmpp:mix:nodes:presence"
                 << "urn:xmpp:mix:nodes:participants"
                 << "urn:xmpp:mix:nodes:info";

    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QXmppIq::Type>("type");
    QTest::addColumn<QXmppMixIq::Type>("actionType");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("channelName");
    QTest::addColumn<QStringList>("nodes");
    QTest::addColumn<QString>("nick");

    QTest::newRow("join-c2s-set")
        << joinC2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::ClientJoin
        << "coven@mix.shakespeare.example"
        << ""
        << defaultNodes
        << "third witch";
    QTest::newRow("join-s2s-set")
        << joinS2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::Join
        << ""
        << ""
        << defaultNodes
        << "stpeter";
    QTest::newRow("join-s2s-result")
        << joinS2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::Join
        << "123456#coven@mix.shakespeare.example"
        << ""
        << defaultNodes
        << "third witch";
    QTest::newRow("join-c2s-result")
        << joinC2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::ClientJoin
        << "123456#coven@mix.shakespeare.example"
        << ""
        << defaultNodes
        << "";
    QTest::newRow("leave-c2s-set")
        << leaveC2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::ClientLeave
        << "coven@mix.shakespeare.example"
        << "" << emptyNodes << "";
    QTest::newRow("leave-s2s-set")
        << leaveS2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::Leave
        << ""
        << "" << emptyNodes << "";
    QTest::newRow("leave-s2s-result")
        << leaveS2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::Leave
        << ""
        << "" << emptyNodes << "";
    QTest::newRow("leave-c2s-result")
        << leaveC2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::ClientLeave
        << ""
        << "" << emptyNodes << "";
    QTest::newRow("update-subscription-set")
        << updateSubscriptionSetXml
        << QXmppIq::Set
        << QXmppMixIq::UpdateSubscription
        << ""
        << ""
        << (QStringList() << "urn:xmpp:mix:nodes:messages")
        << "";
    QTest::newRow("update-subscription-result")
        << updateSubscriptionResultXml
        << QXmppIq::Result
        << QXmppMixIq::UpdateSubscription
        << "hag66@shakespeare.example"
        << ""
        << (QStringList() << "urn:xmpp:mix:nodes:messages")
        << "";
    QTest::newRow("setnick-set")
        << setNickSetXml
        << QXmppIq::Set
        << QXmppMixIq::SetNick
        << ""
        << "" << emptyNodes
        << "thirdwitch";
    QTest::newRow("setnick-result")
        << setNickResultXml
        << QXmppIq::Result
        << QXmppMixIq::SetNick
        << ""
        << "" << emptyNodes
        << "thirdwitch";
    QTest::newRow("create")
        << createXml
        << QXmppIq::Set
        << QXmppMixIq::Create
        << ""
        << "coven" << emptyNodes << "";
    QTest::newRow("create-without-name")
        << createWithoutNameXml
        << QXmppIq::Set
        << QXmppMixIq::Create
        << ""
        << "" << emptyNodes << "";
    QTest::newRow("destroy")
        << destroyXml
        << QXmppIq::Set
        << QXmppMixIq::Destroy
        << ""
        << "coven" << emptyNodes << "";
    QTest::newRow("empty")
        << emptyXml
        << QXmppIq::Set
        << QXmppMixIq::None
        << ""
        << "" << emptyNodes << "";
}

void tst_QXmppMixIq::testBase()
{
    QFETCH(QByteArray, xml);
    QFETCH(QXmppIq::Type, type);
    QFETCH(QXmppMixIq::Type, actionType);
    QFETCH(QString, jid);
    QFETCH(QString, channelName);
    QFETCH(QStringList, nodes);
    QFETCH(QString, nick);

    QXmppMixIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), type);
    QCOMPARE(iq.actionType(), actionType);
    QCOMPARE(iq.jid(), jid);
    QCOMPARE(iq.channelName(), channelName);
    QCOMPARE(iq.nodes(), nodes);
    QCOMPARE(iq.nick(), nick);
    serializePacket(iq, xml);
}

void tst_QXmppMixIq::testDefaults()
{
    QXmppMixIq iq;
    QCOMPARE(iq.actionType(), QXmppMixIq::None);
    QCOMPARE(iq.jid(), QString());
    QCOMPARE(iq.channelName(), QString());
    QCOMPARE(iq.nodes(), QStringList());
    QCOMPARE(iq.nick(), QString());
}

void tst_QXmppMixIq::testSetters()
{
    QXmppMixIq iq;
    iq.setActionType(QXmppMixIq::Join);
    QCOMPARE(iq.actionType(), QXmppMixIq::Join);
    iq.setJid("interestingnews@mix.example.com");
    QCOMPARE(iq.jid(), QString("interestingnews@mix.example.com"));
    iq.setChannelName("interestingnews");
    QCOMPARE(iq.channelName(), QString("interestingnews"));
    iq.setNodes(QStringList() << "com:example:mix:node:custom");
    QCOMPARE(iq.nodes(), QStringList() << "com:example:mix:node:custom");
    iq.setNick("SMUDO");
    QCOMPARE(iq.nick(), QString("SMUDO"));
}

void tst_QXmppMixIq::testInvalidActionType()
{
    const QByteArray xml =
        "<iq id='E6E10350' to='hag66@example.org'"
        " from='hag66@example.org/123' type='set'>"
        "<set-on-fire xmlns='urn:xmpp:mix:core:1' channel='coven'/>"
        "</iq>";
    QXmppMixIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.actionType(), QXmppMixIq::None);
}

void tst_QXmppMixIq::testIsMixIq()
{
    const QByteArray trueXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<destroy xmlns=\"urn:xmpp:mix:core:1\" channel=\"coven\"/>"
        "</iq>");
    const QByteArray truePamXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<client-leave xmlns=\"urn:xmpp:mix:pam:1\" channel=\"coven@mix.shakespeare.example\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</client-leave>"
        "</iq>");
    const QByteArray falseXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<destroy xmlns=\"something:else\" channel=\"coven\"/>"
        "</iq>");

    QDomDocument doc;
    doc.setContent(trueXml, true);
    QDomElement trueElement = doc.documentElement();
    QVERIFY(QXmppMixIq::isMixIq(trueElement));

    doc.setContent(truePamXml, true);
    QDomElement truePamElement = doc.documentElement();
    QVERIFY(QXmppMixIq::isMixIq(truePamElement));

    doc.setContent(falseXml, true);
    QDomElement falseElement = doc.documentElement();
    QVERIFY(!QXmppMixIq::isMixIq(falseElement));
}

QTEST_MAIN(tst_QXmppMixIq)
#include "tst_qxmppmixiq.moc"
