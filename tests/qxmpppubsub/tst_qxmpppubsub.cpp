/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
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

#include "QXmppPubSubAffiliation.h"

#include "util.h"
#include <QObject>

using Affiliation = QXmppPubSubAffiliation;
using AffiliationType = QXmppPubSubAffiliation::Affiliation;

class tst_QXmppPubSub : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testAffiliation_data();
    Q_SLOT void testAffiliation();
    Q_SLOT void testIsAffiliation_data();
    Q_SLOT void testIsAffiliation();
};

void tst_QXmppPubSub::testAffiliation_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<AffiliationType>("type");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("node");

#define ROW(name, xml, type, jid, node) \
    QTest::newRow(name) << QByteArrayLiteral(xml) << type << jid << node

    ROW("owner", "<affiliation affiliation='owner' node='node1'/>", AffiliationType::Owner, QString(), QString("node1"));
    ROW("publisher", "<affiliation affiliation='publisher' node='node2'/>", AffiliationType::Publisher, QString(), QString("node2"));
    ROW("outcast", "<affiliation affiliation='outcast' node='noise'/>", AffiliationType::Outcast, QString(), QString("noise"));
    ROW("none", "<affiliation affiliation='none' node='stuff'/>", AffiliationType::None, QString(), QString("stuff"));
    ROW("with-jid", "<affiliation affiliation='owner' jid='snob@qxmpp.org'/>", AffiliationType::Owner, QString("snob@qxmpp.org"), QString());

#undef ROW
}

void tst_QXmppPubSub::testAffiliation()
{
    QFETCH(QByteArray, xml);
    QFETCH(AffiliationType, type);
    QFETCH(QString, jid);
    QFETCH(QString, node);

    Affiliation affiliation;
    parsePacket(affiliation, xml);
    QCOMPARE(affiliation.jid(), jid);
    QCOMPARE(affiliation.node(), node);
    QCOMPARE(affiliation.type(), type);
    serializePacket(affiliation, xml);

    affiliation = {};
    affiliation.setJid(jid);
    affiliation.setNode(node);
    affiliation.setType(type);
    serializePacket(affiliation, xml);
}

void tst_QXmppPubSub::testIsAffiliation_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("accepted");

    QTest::newRow("ps-correct")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub'><affiliation affiliation=\"owner\" node=\"node1\"/></parent>")
        << true;
    QTest::newRow("ps-missing-node")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub'><affiliation affiliation=\"owner\"/></parent>")
        << false;
    QTest::newRow("ps-invalid-affiliation")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub'><affiliation affiliation=\"gigaowner\" node=\"node1\"/></parent>")
        << false;
    QTest::newRow("psowner-correct")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub#owner'><affiliation affiliation=\"owner\" jid=\"snob@qxmpp.org\"/></parent>")
        << true;
    QTest::newRow("psowner-missing-jid")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub#owner'><affiliation affiliation=\"owner\"/></parent>")
        << false;
    QTest::newRow("psowner-invalid-affiliation")
        << QByteArrayLiteral("<parent xmlns='http://jabber.org/protocol/pubsub#owner'><affiliation affiliation=\"superowner\" jid=\"snob@qxmpp.org\"/></parent>")
        << false;
    QTest::newRow("invalid-namespace")
        << QByteArrayLiteral("<parent xmlns='urn:xmpp:mix:0'><affiliation affiliation=\"owner\" node=\"node1\"/></parent>")
        << false;
}

void tst_QXmppPubSub::testIsAffiliation()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, accepted);

    auto dom = xmlToDom(xml).firstChildElement();
    QCOMPARE(Affiliation::isAffiliation(dom), accepted);
}

QTEST_MAIN(tst_QXmppPubSub)
#include "tst_qxmpppubsub.moc"
