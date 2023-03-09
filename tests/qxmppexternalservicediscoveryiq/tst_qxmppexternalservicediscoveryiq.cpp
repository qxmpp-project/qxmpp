// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppExternalServiceDiscoveryIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppExternalServiceDiscoveryIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testIsExternalService_data();
    Q_SLOT void testIsExternalService();

    Q_SLOT void testExternalService();

    Q_SLOT void testIsExternalServiceDiscoveryIq_data();
    Q_SLOT void testIsExternalServiceDiscoveryIq();

    Q_SLOT void testExternalServiceDiscoveryIq();
};

void tst_QXmppExternalServiceDiscoveryIq::testIsExternalService_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<service host='stun.shakespeare.lit' type='stun'/>")
        << true;
    QTest::newRow("invalidHost")
        << QByteArrayLiteral("<service type='stun'/>")
        << false;
    QTest::newRow("invalidHostEmpty")
        << QByteArrayLiteral("<service type='stun' host=''/>")
        << false;
    QTest::newRow("invalidType")
        << QByteArrayLiteral("<service host='stun.shakespeare.lit'/>")
        << false;
    QTest::newRow("invalidTypeEmpty")
        << QByteArrayLiteral("<service host='stun.shakespeare.lit' type=''/>")
        << false;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid host='stun.shakespeare.lit' type='stun'/>")
        << false;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid/>")
        << false;
}

void tst_QXmppExternalServiceDiscoveryIq::testIsExternalService()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppExternalService::isExternalService(xmlToDom(xml)), isValid);
}

void tst_QXmppExternalServiceDiscoveryIq::testExternalService()
{
    QByteArray xml { QByteArrayLiteral(
        "<service host='stun.shakespeare.lit'"
        " type='stun'"
        " port='9998'"
        " transport='udp'/>") };

    QXmppExternalService service;
    parsePacket(service, xml);
    QCOMPARE(service.host(), "stun.shakespeare.lit");
    QCOMPARE(service.port(), 9998);
    QCOMPARE(service.transport().has_value(), true);
    QCOMPARE(service.transport().value(), QXmppExternalService::Transport::Udp);
    QCOMPARE(service.type(), "stun");
    serializePacket(service, xml);
}

void tst_QXmppExternalServiceDiscoveryIq::testIsExternalServiceDiscoveryIq_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral(
               "<iq from='shakespeare.lit'"
               " id='ul2bc7y6'"
               " to='bard@shakespeare.lit/globe'"
               " type='result'>"
               "<services xmlns='urn:xmpp:extdisco:2'>"
               "<service host='stun.shakespeare.lit'"
               " type='stun'"
               " port='9998'"
               " transport='udp'/>"
               "</services>"
               "</iq>")
        << true;

    QTest::newRow("invalidTag")
        << QByteArrayLiteral(
               "<iq from='shakespeare.lit'"
               " id='ul2bc7y6'"
               " to='bard@shakespeare.lit/globe'"
               " type='result'>"
               "<invalid xmlns='urn:xmpp:extdisco:2'>"
               "<service host='stun.shakespeare.lit'"
               " type='stun'"
               " port='9998'"
               " transport='udp'/>"
               "</invalid>"
               "</iq>")
        << false;

    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral(
               "<iq from='shakespeare.lit'"
               " id='ul2bc7y6'"
               " to='bard@shakespeare.lit/globe'"
               " type='result'>"
               "<services xmlns='invalid'>"
               "<service host='stun.shakespeare.lit'"
               " type='stun'"
               " port='9998'"
               " transport='udp'/>"
               "</services>"
               "</iq>")
        << false;
}

void tst_QXmppExternalServiceDiscoveryIq::testIsExternalServiceDiscoveryIq()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppExternalServiceDiscoveryIq::isExternalServiceDiscoveryIq(xmlToDom(xml)), isValid);
}

void tst_QXmppExternalServiceDiscoveryIq::testExternalServiceDiscoveryIq()
{
    const QByteArray xml { QByteArrayLiteral(
        "<iq"
        " id='qxmpp2'"
        " type='result'>"
        "<services xmlns='urn:xmpp:extdisco:2'>"
        "<service host='stun.shakespeare.lit'"
        " type='stun'"
        " port='9998'"
        " transport='udp'/>"
        "<service host='relay.shakespeare.lit'"
        " type='turn'"
        " password='jj929jkj5sadjfj93v3n'"
        " port='9999'"
        " transport='udp'"
        " username='nb78932lkjlskjfdb7g8'/>"
        "<service host='192.0.2.1'"
        " type='stun'"
        " port='8888'"
        " transport='udp'/>"
        "<service host='192.0.2.1'"
        " type='turn'"
        " password='93jn3bakj9s832lrjbbz'"
        " port='8889'"
        " transport='udp'"
        " username='auu98sjl2wk3e9fjdsl7'/>"
        "<service host='ftp.shakespeare.lit'"
        " type='ftp'"
        " name='Shakespearean File Server'"
        " password='guest'"
        " port='20'"
        " transport='tcp'"
        " username='guest'/>"
        "</services>"
        "</iq>") };

    QXmppExternalServiceDiscoveryIq iq1;
    iq1.setType(QXmppIq::Result);

    parsePacket(iq1, xml);
    QCOMPARE(iq1.externalServices().length(), 5);
    serializePacket(iq1, xml);

    QXmppExternalService service1;
    service1.setHost("127.0.0.1");
    service1.setType("ftp");

    iq1.addExternalService(service1);

    QXmppExternalService service2;
    service2.setHost("127.0.0.1");
    service2.setType("ftp");

    iq1.addExternalService(service2);

    QCOMPARE(iq1.externalServices().length(), 7);

    const QByteArray xml2 { QByteArrayLiteral(
        "<iq"
        " id='qxmpp2'"
        " type='result'>"
        "<services xmlns='urn:xmpp:extdisco:2'>"
        "<service host='193.169.1.256'"
        " type='turn'/>"
        "<service host='194.170.2.257'"
        " type='stun'/>"
        "<service host='195.171.3.258'"
        " type='ftp'/>"
        "</services>"
        "</iq>") };

    QXmppExternalServiceDiscoveryIq iq2;
    iq2.setType(QXmppIq::Result);

    QXmppExternalService service3;
    service3.setHost("193.169.1.256");
    service3.setType("turn");
    QXmppExternalService service4;
    service4.setHost("194.170.2.257");
    service4.setType("stun");
    QXmppExternalService service5;
    service5.setHost("195.171.3.258");
    service5.setType("ftp");

    iq2.setExternalServices({ service3, service4, service5 });

    QCOMPARE(iq2.externalServices().length(), 3);
    serializePacket(iq2, xml2);
}

QTEST_MAIN(tst_QXmppExternalServiceDiscoveryIq)
#include "tst_qxmppexternalservicediscoveryiq.moc"
