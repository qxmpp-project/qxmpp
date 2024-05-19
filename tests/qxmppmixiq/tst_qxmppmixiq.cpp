// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixInvitation.h"
#include "QXmppMixIq.h"
#include "QXmppMixIq_p.h"

#include "util.h"

#include <QObject>

using namespace QXmpp::Private;

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
    Q_SLOT void testListToMixNodes();
    Q_SLOT void testMixNodesToList();
    Q_SLOT void testIsMixInvitationResponseIq_data();
    Q_SLOT void testIsMixInvitationResponseIq();
    Q_SLOT void testMixInvitationResponseIq();
    Q_SLOT void testIsMixInvitationRequestIq_data();
    Q_SLOT void testIsMixInvitationRequestIq();
    Q_SLOT void testMixInvitationRequestIq();
    Q_SLOT void testIsMixSubscriptionUpdateIq_data();
    Q_SLOT void testIsMixSubscriptionUpdateIq();
    Q_SLOT void testMixSubscriptionUpdateIq();
};

void tst_QXmppMixIq::testBase_data()
{
    QByteArray joinC2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<client-join xmlns=\"urn:xmpp:mix:pam:2\" channel=\"coven@mix.shakespeare.example\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<nick>third witch</nick>"
        "<invitation xmlns=\"urn:xmpp:mix:misc:0\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>"
        "</join>"
        "</client-join>"
        "</iq>");
    QByteArray joinS2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"coven@mix.shakespeare.example\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"set\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<nick>stpeter</nick>"
        "<invitation xmlns=\"urn:xmpp:mix:misc:0\">"
        "<inviter>hag66@shakespeare.example</inviter>"
        "<invitee>cat@shakespeare.example</invitee>"
        "<channel>coven@mix.shakespeare.example</channel>"
        "<token>ABCDEF</token>"
        "</invitation>"
        "</join>"
        "</iq>");
    QByteArray joinS2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"coven@mix.shakespeare.example\" "
        "type=\"result\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\" id=\"123456\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "<nick>third witch</nick>"
        "</join>"
        "</iq>");
    QByteArray joinC2sResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<client-join xmlns=\"urn:xmpp:mix:pam:2\">"
        "<join xmlns=\"urn:xmpp:mix:core:1\" "
        "id=\"123456\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "</join>"
        "</client-join>"
        "</iq>");
    QByteArray leaveC2sSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<client-leave xmlns=\"urn:xmpp:mix:pam:2\" channel=\"coven@mix.shakespeare.example\">"
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
        "<client-leave xmlns=\"urn:xmpp:mix:pam:2\">"
        "<leave xmlns=\"urn:xmpp:mix:core:1\"/>"
        "</client-leave>"
        "</iq>");
    // Using QXmppMixIq::UpdateSubscription is deprecated since QXmpp 1.7.
    QByteArray updateSubscriptionSetXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example\" "
        "from=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "type=\"set\">"
        "<update-subscription xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
        "<subscribe node=\"urn:xmpp:mix:nodes:messages\"/>"
        "</update-subscription>"
        "</iq>");
    // Using QXmppMixIq::UpdateSubscription is deprecated since QXmpp 1.7.
    QByteArray updateSubscriptionResultXml(
        "<iq id=\"E6E10350-76CF-40C6-B91B-1EA08C332FC7\" "
        "to=\"hag66@shakespeare.example/UUID-a1j/7533\" "
        "from=\"hag66@shakespeare.example\" "
        "type=\"result\">"
        "<update-subscription xmlns=\"urn:xmpp:mix:core:1\">"
        "<subscribe node=\"urn:xmpp:mix:nodes:info\"/>"
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
    QByteArray createWithoutIdXml(
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

    QStringList emptyNodeList;
    QStringList nodeList = { "urn:xmpp:mix:nodes:info", "urn:xmpp:mix:nodes:messages" };
    QXmppMixConfigItem::Nodes subscriptions = { QXmppMixConfigItem::Node::Information | QXmppMixConfigItem::Node::Messages };
    QXmppMixConfigItem::Nodes noNodes;

    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QXmppIq::Type>("type");
    QTest::addColumn<QXmppMixIq::Type>("actionType");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("participantId");
    QTest::addColumn<QString>("channelName");
    QTest::addColumn<QString>("channelId");
    QTest::addColumn<QString>("channelJid");
    QTest::addColumn<QStringList>("nodes");
    QTest::addColumn<QXmppMixConfigItem::Nodes>("subscriptions");
    QTest::addColumn<QString>("nick");
    QTest::addColumn<QString>("invitationToken");

    QTest::newRow("join-c2s-set")
        << joinC2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::ClientJoin
        << "coven@mix.shakespeare.example"
        << ""
        << ""
        << ""
        << "coven@mix.shakespeare.example"
        << nodeList
        << subscriptions
        << "third witch"
        << "ABCDEF";
    QTest::newRow("join-s2s-set")
        << joinS2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::Join
        << ""
        << ""
        << ""
        << ""
        << ""
        << nodeList
        << subscriptions
        << "stpeter"
        << "ABCDEF";
    QTest::newRow("join-s2s-result")
        << joinS2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::Join
        << ""
        << "123456"
        << ""
        << ""
        << ""
        << nodeList
        << subscriptions
        << "third witch"
        << "";
    QTest::newRow("join-c2s-result")
        << joinC2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::ClientJoin
        << ""
        << "123456"
        << ""
        << ""
        << ""
        << nodeList
        << subscriptions
        << ""
        << "";
    QTest::newRow("leave-c2s-set")
        << leaveC2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::ClientLeave
        << "coven@mix.shakespeare.example"
        << ""
        << ""
        << ""
        << "coven@mix.shakespeare.example"
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("leave-s2s-set")
        << leaveS2sSetXml
        << QXmppIq::Set
        << QXmppMixIq::Leave
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("leave-s2s-result")
        << leaveS2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::Leave
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("leave-c2s-result")
        << leaveC2sResultXml
        << QXmppIq::Result
        << QXmppMixIq::ClientLeave
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    // Using QXmppMixIq::UpdateSubscription is deprecated since QXmpp 1.7.
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QTest::newRow("update-subscription-set")
        << updateSubscriptionSetXml
        << QXmppIq::Set
        << QXmppMixIq::UpdateSubscription
        << ""
        << ""
        << ""
        << ""
        << ""
        << nodeList
        << subscriptions
        << ""
        << "";
    QTest::newRow("update-subscription-result")
        << updateSubscriptionResultXml
        << QXmppIq::Result
        << QXmppMixIq::UpdateSubscription
        << ""
        << ""
        << ""
        << ""
        << ""
        << nodeList
        << subscriptions
        << ""
        << "";
    QT_WARNING_POP
    QTest::newRow("setnick-set")
        << setNickSetXml
        << QXmppIq::Set
        << QXmppMixIq::SetNick
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << "thirdwitch"
        << "";
    QTest::newRow("setnick-result")
        << setNickResultXml
        << QXmppIq::Result
        << QXmppMixIq::SetNick
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << "thirdwitch"
        << "";
    QTest::newRow("create")
        << createXml
        << QXmppIq::Set
        << QXmppMixIq::Create
        << ""
        << ""
        << "coven"
        << "coven"
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("create-without-id")
        << createWithoutIdXml
        << QXmppIq::Set
        << QXmppMixIq::Create
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("destroy")
        << destroyXml
        << QXmppIq::Set
        << QXmppMixIq::Destroy
        << ""
        << ""
        << "coven"
        << "coven"
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
    QTest::newRow("empty")
        << emptyXml
        << QXmppIq::Set
        << QXmppMixIq::None
        << ""
        << ""
        << ""
        << ""
        << ""
        << emptyNodeList
        << noNodes
        << ""
        << "";
}

void tst_QXmppMixIq::testBase()
{
    QFETCH(QByteArray, xml);
    QFETCH(QXmppIq::Type, type);
    QFETCH(QXmppMixIq::Type, actionType);
    QFETCH(QString, jid);
    QFETCH(QString, participantId);
    QFETCH(QString, channelName);
    QFETCH(QString, channelId);
    QFETCH(QString, channelJid);
    QFETCH(QStringList, nodes);
    QFETCH(QXmppMixConfigItem::Nodes, subscriptions);
    QFETCH(QString, nick);
    QFETCH(QString, invitationToken);

    QXmppMixIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), type);
    QCOMPARE(iq.actionType(), actionType);
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.jid(), jid);
    QT_WARNING_POP
    QCOMPARE(iq.participantId(), participantId);
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.channelName(), channelName);
    QT_WARNING_POP
    QCOMPARE(iq.channelId(), channelId);
    QCOMPARE(iq.channelJid(), channelJid);
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.nodes(), nodes);
    QT_WARNING_POP
    QCOMPARE(iq.subscriptions(), subscriptions);
    QCOMPARE(iq.nick(), nick);
    QCOMPARE(iq.invitation().has_value(), !invitationToken.isEmpty());
    if (iq.invitation()) {
        QCOMPARE(iq.invitation()->token(), invitationToken);
    }
    serializePacket(iq, xml);
}

void tst_QXmppMixIq::testDefaults()
{
    QXmppMixIq iq;
    QCOMPARE(iq.actionType(), QXmppMixIq::None);
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.jid(), QString());
    QT_WARNING_POP
    QCOMPARE(iq.participantId(), QString());
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.channelName(), QString());
    QT_WARNING_POP
    QCOMPARE(iq.channelId(), QString());
    QCOMPARE(iq.channelJid(), QString());
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE(iq.nodes(), QStringList());
    QT_WARNING_POP
    QCOMPARE(iq.subscriptions(), QXmppMixConfigItem::Nodes {});
    QCOMPARE(iq.nick(), QString());
    QVERIFY(!iq.invitation());
}

void tst_QXmppMixIq::testSetters()
{
    QXmppMixIq iq;

    iq.setActionType(QXmppMixIq::Join);
    QCOMPARE(iq.actionType(), QXmppMixIq::Join);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    iq.setJid("coven@mix.example.com");
    QCOMPARE(iq.jid(), u"coven@mix.example.com"_s);
    QT_WARNING_POP

    iq.setParticipantId("123456");
    QCOMPARE(iq.participantId(), "123456");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    iq.setChannelName("coven");
    QCOMPARE(iq.channelName(), u"coven"_s);
    QT_WARNING_POP

    iq.setChannelId("coven");
    QCOMPARE(iq.channelId(), "coven");

    iq.setChannelJid("coven@mix.shakespeare.example");
    QCOMPARE(iq.channelJid(), "coven@mix.shakespeare.example");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    iq.setNodes(QStringList() << "urn:xmpp:mix:nodes:info");
    QCOMPARE(iq.nodes(), QStringList() << "urn:xmpp:mix:nodes:info");
    QT_WARNING_POP

    iq.setSubscriptions(QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids);
    QCOMPARE(iq.subscriptions(), QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids);

    iq.setNick("third witch");
    QCOMPARE(iq.nick(), u"third witch"_s);

    QXmppMixInvitation invitation;
    invitation.setToken(u"ABCDEF"_s);

    iq.setInvitation(invitation);
    QCOMPARE(iq.invitation()->token(), u"ABCDEF"_s);
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
        "<client-leave xmlns=\"urn:xmpp:mix:pam:2\" channel=\"coven@mix.shakespeare.example\">"
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

void tst_QXmppMixIq::testListToMixNodes()
{
    QVERIFY(!listToMixNodes({}));
    const QXmppMixConfigItem::Nodes nodes = { QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids };
    const QVector<QString> nodeList = { u"urn:xmpp:mix:nodes:allowed"_s, u"urn:xmpp:mix:nodes:banned"_s };
    QCOMPARE(listToMixNodes(nodeList), nodes);
}

void tst_QXmppMixIq::testMixNodesToList()
{
    QVERIFY(mixNodesToList({}).isEmpty());
    const QXmppMixConfigItem::Nodes nodes = { QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids };
    const QVector<QString> nodeList = { u"urn:xmpp:mix:nodes:allowed"_s, u"urn:xmpp:mix:nodes:banned"_s };
    QCOMPARE(mixNodesToList(nodes), nodeList);
}

void tst_QXmppMixIq::testIsMixInvitationResponseIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="hag66@shakespeare.example/UUID-h5z/0253" from="coven@mix.shakespeare.example" type="result">
                <invite xmlns="urn:xmpp:mix:misc:0"/>
            </iq>
        )")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="hag66@shakespeare.example/UUID-h5z/0253" from="coven@mix.shakespeare.example" type="result">
                <invalid xmlns="urn:xmpp:mix:misc:0"/>
            </iq>
        )")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="hag66@shakespeare.example/UUID-h5z/0253" from="coven@mix.shakespeare.example" type="result">
                <invite xmlns="invalid"/>
            </iq>
        )")
        << false;
}

void tst_QXmppMixIq::testIsMixInvitationResponseIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, valid);

    QCOMPARE(QXmppMixInvitationResponseIq::isMixInvitationResponseIq(xmlToDom(xml)), valid);
}

void tst_QXmppMixIq::testMixInvitationResponseIq()
{
    const QByteArray xml(R"(
        <iq id="kl2fax27" to="hag66@shakespeare.example/UUID-h5z/0253" from="coven@mix.shakespeare.example" type="result">
            <invite xmlns="urn:xmpp:mix:misc:0">
                <invitation xmlns="urn:xmpp:mix:misc:0">
                    <token>ABCDEF</token>
                </invitation>
            </invite>
        </iq>
    )");

    QXmppMixInvitationResponseIq iq1;
    QVERIFY(iq1.invitation().token().isEmpty());

    parsePacket(iq1, xml);
    QCOMPARE(iq1.invitation().token(), u"ABCDEF"_s);
    serializePacket(iq1, xml);

    QXmppMixInvitation invitation;
    invitation.setToken(u"ABCDEF"_s);

    QXmppMixInvitationResponseIq iq2;
    iq2.setType(QXmppIq::Result);
    iq2.setId(u"kl2fax27"_s);
    iq2.setFrom(u"coven@mix.shakespeare.example"_s);
    iq2.setTo(u"hag66@shakespeare.example/UUID-h5z/0253"_s);
    iq2.setInvitation(invitation);

    QCOMPARE(iq2.invitation().token(), u"ABCDEF"_s);
    serializePacket(iq2, xml);
}

void tst_QXmppMixIq::testIsMixInvitationRequestIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-h5z/0253" type="get">
                <invite xmlns="urn:xmpp:mix:misc:0"/>
            </iq>
        )")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-h5z/0253" type="get">
                <invalid xmlns="urn:xmpp:mix:misc:0"/>
            </iq>
        )")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral(R"(
            <iq id="kl2fax27" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-h5z/0253" type="get">
                <invite xmlns="invalid"/>
            </iq>
        )")
        << false;
}

void tst_QXmppMixIq::testIsMixInvitationRequestIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, valid);

    QCOMPARE(QXmppMixInvitationRequestIq::isMixInvitationRequestIq(xmlToDom(xml)), valid);
}

void tst_QXmppMixIq::testMixInvitationRequestIq()
{
    const QByteArray xml(R"(
        <iq id="kl2fax27" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-h5z/0253" type="get">
            <invite xmlns="urn:xmpp:mix:misc:0">
                <invitee>cat@shakespeare.example</invitee>
            </invite>
        </iq>
    )");

    QXmppMixInvitationRequestIq iq1;
    QVERIFY(iq1.inviteeJid().isEmpty());

    parsePacket(iq1, xml);
    QCOMPARE(iq1.inviteeJid(), u"cat@shakespeare.example"_s);
    serializePacket(iq1, xml);

    QXmppMixInvitationRequestIq iq2;
    iq2.setType(QXmppIq::Get);
    iq2.setId(u"kl2fax27"_s);
    iq2.setFrom(u"hag66@shakespeare.example/UUID-h5z/0253"_s);
    iq2.setTo(u"coven@mix.shakespeare.example"_s);
    iq2.setInviteeJid(u"cat@shakespeare.example"_s);

    QCOMPARE(iq2.inviteeJid(), u"cat@shakespeare.example"_s);
    serializePacket(iq2, xml);
}

void tst_QXmppMixIq::testIsMixSubscriptionUpdateIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid")
        << QByteArrayLiteral(R"(
            <iq id="E6E10350-76CF-40C6-B91B-1EA08C332FC7" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-a1j/7533" type="set">
                <update-subscription xmlns="urn:xmpp:mix:core:1"/>
            </iq>
        )")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral(R"(
            <iq id="E6E10350-76CF-40C6-B91B-1EA08C332FC7" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-a1j/7533" type="set">
                <invalid xmlns="urn:xmpp:mix:core:1"/>
            </iq>
        )")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral(R"(
            <iq id="E6E10350-76CF-40C6-B91B-1EA08C332FC7" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-a1j/7533" type="set">
                <update-subscription xmlns="invalid"/>
            </iq>
        )")
        << false;
}

void tst_QXmppMixIq::testIsMixSubscriptionUpdateIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, valid);

    QCOMPARE(QXmppMixSubscriptionUpdateIq::isMixSubscriptionUpdateIq(xmlToDom(xml)), valid);
}

void tst_QXmppMixIq::testMixSubscriptionUpdateIq()
{
    const QByteArray xml(R"(
        <iq id="E6E10350-76CF-40C6-B91B-1EA08C332FC7" to="coven@mix.shakespeare.example" from="hag66@shakespeare.example/UUID-a1j/7533" type="set">
            <update-subscription xmlns="urn:xmpp:mix:core:1">
                <subscribe node="urn:xmpp:mix:nodes:allowed"/>
                <subscribe node="urn:xmpp:mix:nodes:banned"/>
                <unsubscribe node="urn:xmpp:mix:nodes:info"/>
                <unsubscribe node="urn:xmpp:mix:nodes:messages"/>
            </update-subscription>
        </iq>
    )");

    QXmppMixSubscriptionUpdateIq iq1;
    QVERIFY(!iq1.additions());
    QVERIFY(!iq1.removals());

    const QXmppMixConfigItem::Nodes additions = { QXmppMixConfigItem::Node::AllowedJids | QXmppMixConfigItem::Node::BannedJids };
    const QXmppMixConfigItem::Nodes removals = { QXmppMixConfigItem::Node::Information | QXmppMixConfigItem::Node::Messages };

    parsePacket(iq1, xml);
    QCOMPARE(iq1.additions(), additions);
    QCOMPARE(iq1.removals(), removals);
    serializePacket(iq1, xml);

    QXmppMixSubscriptionUpdateIq iq2;
    iq2.setType(QXmppIq::Set);
    iq2.setId(u"E6E10350-76CF-40C6-B91B-1EA08C332FC7"_s);
    iq2.setFrom(u"hag66@shakespeare.example/UUID-a1j/7533"_s);
    iq2.setTo(u"coven@mix.shakespeare.example"_s);
    iq2.setAdditions(additions);
    iq2.setRemovals(removals);

    QCOMPARE(iq2.additions(), additions);
    QCOMPARE(iq2.removals(), removals);
    serializePacket(iq2, xml);
}

QTEST_MAIN(tst_QXmppMixIq)
#include "tst_qxmppmixiq.moc"
