/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Jonah Brüchert
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

#include <QObject>
#include "util.h"
#include <QXmppPushEnableIq.h>
#include <QXmppDataForm.h>

class tst_QXmppPushEnableIq : public QObject
{
    Q_OBJECT

private slots:
    void testPushEnable();
    void testPushDisable();
    void testXmlNs();
    void testDataForm();
    void testIsEnableIq();
};

void tst_QXmppPushEnableIq::testPushEnable()
{
    const QByteArray xml(
        R"(<iq id="x42" type="set">)"
        R"(<enable xmlns="urn:xmpp:push:0" jid="push-5.client.example" node="yxs32uqsflafdk3iuqo"/>)"
        "</iq>");

    QXmppPushEnableIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.mode(), QXmppPushEnableIq::Enable);
    QCOMPARE(iq.jid(), QStringLiteral("push-5.client.example"));
    QCOMPARE(iq.node(), QStringLiteral("yxs32uqsflafdk3iuqo"));

    serializePacket(iq, xml);

    QXmppPushEnableIq sIq;
    sIq.setJid("push-5.client.example");
    sIq.setMode(QXmppPushEnableIq::Enable);
    sIq.setNode("yxs32uqsflafdk3iuqo");
    sIq.setType(QXmppIq::Set);
    sIq.setId("x42");

    serializePacket(sIq, xml);
}

void tst_QXmppPushEnableIq::testPushDisable()
{
    const QByteArray xml(
        R"(<iq id="x97" type="set">)"
        R"(<disable xmlns="urn:xmpp:push:0" jid="push-5.client.example" node="yxs32uqsflafdk3iuqo"/>)"
        "</iq>");

    QXmppPushEnableIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.mode(), QXmppPushEnableIq::Disable);
    QCOMPARE(iq.jid(), QStringLiteral("push-5.client.example"));

    serializePacket(iq, xml);

    QXmppPushEnableIq sIq;
    sIq.setJid("push-5.client.example");
    sIq.setMode(QXmppPushEnableIq::Disable);
    sIq.setNode("yxs32uqsflafdk3iuqo");
    sIq.setType(QXmppIq::Set);
    sIq.setId("x97");

    serializePacket(sIq, xml);
}

void tst_QXmppPushEnableIq::testXmlNs()
{
    const QByteArray xml(
        R"(<iq type="set" id="x97">)"
        R"(<disable xmlns="urn:ympp:wrongns:0" jid="push-5.client.example"/>)"
        "</iq>");

    QXmppPushEnableIq iq;
    parsePacket(iq, xml);
    QVERIFY(iq.jid().isEmpty());
}

void tst_QXmppPushEnableIq::testDataForm()
{
    const QByteArray xml(
        R"(<iq id="x43" type="set">)"
        R"(<enable xmlns="urn:xmpp:push:0" jid="push-5.client.example" node="yxs32uqsflafdk3iuqo">)"
        R"(<x xmlns="jabber:x:data" type="submit">)"
        R"(<field type="hidden" var="FORM_TYPE"><value>http://jabber.org/protocol/pubsub#publish-options</value></field>)"
        R"(<field type="text-single" var="secret"><value>eruio234vzxc2kla-91</value></field>)"
        "</x>"
        "</enable>"
        "</iq>");

    QXmppPushEnableIq iq;
    parsePacket(iq, xml);
    QVERIFY(!iq.dataForm().isNull());
    QCOMPARE(iq.dataForm().fields().size(), 2);

    serializePacket(iq, xml);

    QXmppPushEnableIq sIq;

    QXmppDataForm::Field field0;
    field0.setKey("FORM_TYPE");
    field0.setType(QXmppDataForm::Field::HiddenField);
    field0.setValue("http://jabber.org/protocol/pubsub#publish-options");

    QXmppDataForm::Field field1;
    field1.setKey("secret");
    field1.setValue("eruio234vzxc2kla-91");

    QXmppDataForm form;
    form.setType(QXmppDataForm::Submit);
    form.setFields({field0, field1});

    sIq.setDataForm(form);

    sIq.setType(QXmppIq::Set);
    sIq.setMode(QXmppPushEnableIq::Enable);
    sIq.setId("x43");
    sIq.setJid("push-5.client.example");
    sIq.setNode("yxs32uqsflafdk3iuqo");

    serializePacket(sIq, xml);
}

void tst_QXmppPushEnableIq::testIsEnableIq()
{
    const QByteArray xml(
        R"(<iq id="x42" type="set">)"
        R"(<enable xmlns="urn:xmpp:push:0" jid="push-5.client.example" node="yxs32uqsflafdk3iuqo"/>)"
        "</iq>");

    QDomDocument doc;
    doc.setContent(xml, true);
    bool isPushEnable = QXmppPushEnableIq::isPushEnableIq(doc.documentElement());
    QCOMPARE(isPushEnable, true);

    // reset
    isPushEnable = false;

    const QByteArray xml2(
        R"(<iq id="x97" type="set">)"
        R"(<disable xmlns="urn:xmpp:push:0" jid="push-5.client.example" node="yxs32uqsflafdk3iuqo"/>)"
        "</iq>");

    doc.setContent(xml2, true);
    isPushEnable = QXmppPushEnableIq::isPushEnableIq(doc.documentElement());
    QCOMPARE(isPushEnable, true);
}

QTEST_MAIN(tst_QXmppPushEnableIq);
#include "tst_qxmpppushenableiq.moc"
