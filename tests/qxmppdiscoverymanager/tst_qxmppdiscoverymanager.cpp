// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDiscoveryManager.h"

#include "TestClient.h"

class tst_QXmppDiscoveryManager : public QObject
{
    Q_OBJECT
private:
    Q_SLOT void testInfo();
    Q_SLOT void testItems();
    Q_SLOT void testRequests();
};

void tst_QXmppDiscoveryManager::testInfo()
{
    TestClient test;
    auto *discoManager = test.addNewExtension<QXmppDiscoveryManager>();

    auto future = discoManager->requestDiscoInfo("user@example.org");
    test.expect("<iq id='qxmpp1' to='user@example.org' type='get'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>");
    test.inject<QString>(R"(
<iq id='qxmpp1' from='user@example.org' type='result'>
    <query xmlns='http://jabber.org/protocol/disco#info'>
        <identity category='pubsub' type='service'/>
        <feature var='http://jabber.org/protocol/pubsub'/>
        <feature var='urn:xmpp:mix:core:1'/>
    </query>
</iq>)");

    const auto info = expectFutureVariant<QXmppDiscoveryIq>(future.toFuture(this));

    const QStringList expFeatures = { "http://jabber.org/protocol/pubsub", "urn:xmpp:mix:core:1" };
    QCOMPARE(info.features(), expFeatures);
    QCOMPARE(info.identities().count(), 1);
}

void tst_QXmppDiscoveryManager::testItems()
{
    TestClient test;
    auto *discoManager = test.addNewExtension<QXmppDiscoveryManager>();

    auto future = discoManager->requestDiscoItems("user@example.org");
    test.expect("<iq id='qxmpp1' to='user@example.org' type='get'><query xmlns='http://jabber.org/protocol/disco#items'/></iq>");
    test.inject<QString>(R"(
<iq type='result'
    from='user@example.org'
    id='qxmpp1'>
  <query xmlns='http://jabber.org/protocol/disco#items'>
    <item name='368866411b877c30064a5f62b917cffe'/>
    <item name='3300659945416e274474e469a1f0154c'/>
    <item name='4e30f35051b7b8b42abe083742187228'/>
    <item name='ae890ac52d0df67ed7cfdf51b644e901'/>
  </query>
</iq>)");

    const auto items = expectFutureVariant<QList<QXmppDiscoveryIq::Item>>(future.toFuture(this));

    const QStringList expFeatures = { "http://jabber.org/protocol/pubsub", "urn:xmpp:mix:core:1" };
    QCOMPARE(items.size(), 4);
    QCOMPARE(items.at(0).name(), QStringLiteral("368866411b877c30064a5f62b917cffe"));
    QCOMPARE(items.at(1).name(), QStringLiteral("3300659945416e274474e469a1f0154c"));
    QCOMPARE(items.at(2).name(), QStringLiteral("4e30f35051b7b8b42abe083742187228"));
    QCOMPARE(items.at(3).name(), QStringLiteral("ae890ac52d0df67ed7cfdf51b644e901"));
}

void tst_QXmppDiscoveryManager::testRequests()
{
    TestClient test;
    test.configuration().setJid("user@qxmpp.org/a");
    auto *discoManager = test.addNewExtension<QXmppDiscoveryManager>();

    discoManager->handleStanza(xmlToDom(R"(
<iq type='get' from='romeo@montague.net/orchard' to='user@qxmpp.org/a' id='info1'>
  <query xmlns='http://jabber.org/protocol/disco#info'/>
</iq>)"));

    test.expect("<iq id='info1' to='romeo@montague.net/orchard' type='result'><query xmlns='http://jabber.org/protocol/disco#info'><identity category='client' name='tst_qxmppdiscoverymanager ' type='pc'/><feature var='jabber:x:data'/><feature var='http://jabber.org/protocol/rsm'/><feature var='jabber:x:oob'/><feature var='http://jabber.org/protocol/xhtml-im'/><feature var='http://jabber.org/protocol/chatstates'/><feature var='http://jabber.org/protocol/caps'/><feature var='urn:xmpp:ping'/><feature var='jabber:x:conference'/><feature var='urn:xmpp:message-correct:0'/><feature var='urn:xmpp:chat-markers:0'/><feature var='urn:xmpp:hints'/><feature var='urn:xmpp:sid:0'/><feature var='urn:xmpp:message-attaching:1'/><feature var='urn:xmpp:eme:0'/><feature var='urn:xmpp:spoiler:0'/><feature var='urn:xmpp:fallback:0'/><feature var='urn:xmpp:reactions:0'/><feature var='http://jabber.org/protocol/disco#info'/></query></iq>");
}

QTEST_MAIN(tst_QXmppDiscoveryManager)

#include "tst_qxmppdiscoverymanager.moc"
