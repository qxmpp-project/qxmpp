/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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
#include "QXmppRegisterIq.h"
#include "util.h"

class tst_QXmppRegisterIq : public QObject
{
    Q_OBJECT

private slots:
    void testGet();
    void testResult();
    void testResultWithForm();
    void testSet();
    void testSetWithForm();
};

void tst_QXmppRegisterIq::testGet()
{
    const QByteArray xml(
        "<iq id=\"reg1\" to=\"shakespeare.lit\" type=\"get\">"
        "<query xmlns=\"jabber:iq:register\"/>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg1"));
    QCOMPARE(iq.to(), QLatin1String("shakespeare.lit"));
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.instructions(), QString());
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(iq.form().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResult()
{
    const QByteArray xml(
        "<iq id=\"reg1\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>Choose a username and password for use with this service. Please also provide your email address.</instructions>"
        "<username/>"
        "<password/>"
        "<email/>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg1"));
    QCOMPARE(iq.to(), QString());
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("Choose a username and password for use with this service. Please also provide your email address."));
    QVERIFY(!iq.username().isNull());
    QVERIFY(iq.username().isEmpty());
    QVERIFY(!iq.password().isNull());
    QVERIFY(iq.password().isEmpty());
    QVERIFY(!iq.email().isNull());
    QVERIFY(iq.email().isEmpty());
    QVERIFY(iq.form().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testResultWithForm()
{
    const QByteArray xml(
        "<iq id=\"reg3\" to=\"juliet@capulet.com/balcony\" from=\"contests.shakespeare.lit\" type=\"result\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<instructions>Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php</instructions>"
        "<x xmlns=\"jabber:x:data\" type=\"form\">"
        "<title>Contest Registration</title>"
        "<instructions>"
        "Please provide the following information"
        "to sign up for our special contests!"
        "</instructions>"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>jabber:iq:register</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Given Name\" var=\"first\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Family Name\" var=\"last\">"
        "<required/>"
        "</field>"
        "<field type=\"text-single\" label=\"Email Address\" var=\"email\">"
        "<required/>"
        "</field>"
        "<field type=\"list-single\" label=\"Gender\" var=\"x-gender\">"
        "<option label=\"Male\"><value>M</value></option>"
        "<option label=\"Female\"><value>F</value></option>"
        "</field>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg3"));
    QCOMPARE(iq.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(iq.from(), QLatin1String("contests.shakespeare.lit"));
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.instructions(), QLatin1String("Use the enclosed form to register. If your Jabber client does not support Data Forms, visit http://www.shakespeare.lit/contests.php"));
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(!iq.form().isNull());
    QCOMPARE(iq.form().title(), QLatin1String("Contest Registration"));
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testSet()
{
    const QByteArray xml(
        "<iq id=\"reg2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<username>bill</username>"
        "<password>Calliope</password>"
        "<email>bard@shakespeare.lit</email>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg2"));
    QCOMPARE(iq.to(), QString());
    QCOMPARE(iq.from(), QString());
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.password(), QLatin1String("Calliope"));
    QCOMPARE(iq.email(), QLatin1String("bard@shakespeare.lit"));
    QVERIFY(iq.form().isNull());
    serializePacket(iq, xml);
}

void tst_QXmppRegisterIq::testSetWithForm()
{
    const QByteArray xml(
        "<iq id=\"reg4\" to=\"contests.shakespeare.lit\" from=\"juliet@capulet.com/balcony\" type=\"set\">"
        "<query xmlns=\"jabber:iq:register\">"
        "<x xmlns=\"jabber:x:data\" type=\"submit\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>jabber:iq:register</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Given Name\" var=\"first\">"
        "<value>Juliet</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Family Name\" var=\"last\">"
        "<value>Capulet</value>"
        "</field>"
        "<field type=\"text-single\" label=\"Email Address\" var=\"email\">"
        "<value>juliet@capulet.com</value>"
        "</field>"
        "<field type=\"list-single\" label=\"Gender\" var=\"x-gender\">"
        "<value>F</value>"
        "</field>"
        "</x>"
        "</query>"
        "</iq>");

    QXmppRegisterIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.id(), QLatin1String("reg4"));
    QCOMPARE(iq.to(), QLatin1String("contests.shakespeare.lit"));
    QCOMPARE(iq.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(iq.type(), QXmppIq::Set);
    QVERIFY(iq.username().isNull());
    QVERIFY(iq.password().isNull());
    QVERIFY(iq.email().isNull());
    QVERIFY(!iq.form().isNull());
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppRegisterIq)
#include "tst_qxmppregisteriq.moc"
