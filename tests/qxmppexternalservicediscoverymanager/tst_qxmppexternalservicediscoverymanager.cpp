// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppExternalServiceDiscoveryManager.h"

#include "TestClient.h"
#include "util.h"
#include <QObject>

const char *ns_external_service_discovery = "urn:xmpp:extdisco:2";

class tst_QXmppExternalServiceDiscoveryManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testRequestServices();
    Q_SLOT void testDiscoveryFeatures();
};

void tst_QXmppExternalServiceDiscoveryManager::testRequestServices()
{
    TestClient test;
    auto *extDiscoManager { test.addNewExtension<QXmppExternalServiceDiscoveryManager>() };

    auto future { extDiscoManager->requestServices("shakespeare.lit") };

    test.expect("<iq"
                " id='qxmpp1'"
                " to='shakespeare.lit'"
                " type='get'>"
                "<services xmlns='urn:xmpp:extdisco:2'/>"
                "</iq>");

    test.inject<QString>("<iq"
                         " id='qxmpp1'"
                         " from='shakespeare.lit'"
                         " type='result'>"
                         "<services xmlns='urn:xmpp:extdisco:2'>"
                         "<service host='stun.shakespeare.lit'"
                         " port='9998'"
                         " transport='udp'"
                         " type='stun'/>"
                         "<service host='relay.shakespeare.lit'"
                         " password='jj929jkj5sadjfj93v3n'"
                         " port='9999'"
                         " transport='udp'"
                         " type='turn'"
                         " username='nb78932lkjlskjfdb7g8'/>"
                         "<service host='192.0.2.1'"
                         " port='8888'"
                         " transport='udp'"
                         " type='stun'/>"
                         "<service host='192.0.2.1'"
                         " port='8889'"
                         " password='93jn3bakj9s832lrjbbz'"
                         " transport='udp'"
                         " type='turn'"
                         " username='auu98sjl2wk3e9fjdsl7'/>"
                         "<service host='ftp.shakespeare.lit'"
                         " name='Shakespearean File Server'"
                         " password='guest'"
                         " port='20'"
                         " transport='tcp'"
                         " type='ftp'"
                         " username='guest'/>"
                         "</services>"
                         "</iq>");

    const auto items { expectFutureVariant<QVector<QXmppExternalService>>(future.toFuture(this)) };

    QCOMPARE(items.size(), 5);
    QCOMPARE(items.at(0).host(), QStringLiteral("stun.shakespeare.lit"));
    QCOMPARE(items.at(4).host(), QStringLiteral("ftp.shakespeare.lit"));
}

void tst_QXmppExternalServiceDiscoveryManager::testDiscoveryFeatures()
{
    TestClient test;
    auto *m = test.addNewExtension<QXmppExternalServiceDiscoveryManager>();

    QCOMPARE(m->discoveryFeatures().contains(ns_external_service_discovery), true);
}

QTEST_MAIN(tst_QXmppExternalServiceDiscoveryManager)

#include "tst_qxmppexternalservicediscoverymanager.moc"
