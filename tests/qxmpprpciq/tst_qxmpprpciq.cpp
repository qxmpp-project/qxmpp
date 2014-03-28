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
#include "QXmppRpcIq.h"
#include "util.h"

static void checkVariant(const QVariant &value, const QByteArray &xml)
{
    // serialise
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    QXmppRpcMarshaller::marshall(&writer, value);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);

    // parse
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    QStringList errors;
    QVariant test = QXmppRpcMarshaller::demarshall(element, errors);
    if (!errors.isEmpty())
        qDebug() << errors;
    QCOMPARE(errors, QStringList());
    QCOMPARE(test, value);
}

class tst_QXmppRpcIq : public QObject
{
    Q_OBJECT

private slots:
    void testBase64();
    void testBool();
    void testDateTime();
    void testDouble();
    void testInt();
    void testNil();
    void testString();

    void testArray();
    void testStruct();

    void testInvoke();
    void testResponse();
    void testResponseFault();
};
void tst_QXmppRpcIq::testBase64()
{
    checkVariant(QByteArray("\0\1\2\3", 4),
                 QByteArray("<value><base64>AAECAw==</base64></value>"));
}

void tst_QXmppRpcIq::testBool()
{
    checkVariant(false,
                 QByteArray("<value><boolean>0</boolean></value>"));
    checkVariant(true,
                 QByteArray("<value><boolean>1</boolean></value>"));
}

void tst_QXmppRpcIq::testDateTime()
{
    checkVariant(QDateTime(QDate(1998, 7, 17), QTime(14, 8, 55)),
                 QByteArray("<value><dateTime.iso8601>1998-07-17T14:08:55</dateTime.iso8601></value>"));
}

void tst_QXmppRpcIq::testDouble()
{
    checkVariant(double(-12.214),
                 QByteArray("<value><double>-12.214</double></value>"));
}

void tst_QXmppRpcIq::testInt()
{
    checkVariant(int(-12),
                 QByteArray("<value><i4>-12</i4></value>"));
}

void tst_QXmppRpcIq::testNil()
{
    checkVariant(QVariant(),
                 QByteArray("<value><nil/></value>"));
}

void tst_QXmppRpcIq::testString()
{
    checkVariant(QString("hello world"),
                 QByteArray("<value><string>hello world</string></value>"));
}

void tst_QXmppRpcIq::testArray()
{
    checkVariant(QVariantList() << QString("hello world") << double(-12.214),
                 QByteArray("<value><array><data>"
                            "<value><string>hello world</string></value>"
                            "<value><double>-12.214</double></value>"
                            "</data></array></value>"));
}

void tst_QXmppRpcIq::testStruct()
{
    QMap<QString, QVariant> map;
    map["bar"] = QString("hello world");
    map["foo"] = double(-12.214);
    checkVariant(map,
                 QByteArray("<value><struct>"
                            "<member>"
                                "<name>bar</name>"
                                "<value><string>hello world</string></value>"
                            "</member>"
                            "<member>"
                                "<name>foo</name>"
                                "<value><double>-12.214</double></value>"
                            "</member>"
                            "</struct></value>"));
}

void tst_QXmppRpcIq::testInvoke()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"responder@company-a.com/jrpc-server\""
        " from=\"requester@company-b.com/jrpc-client\""
        " type=\"set\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodCall>"
        "<methodName>examples.getStateName</methodName>"
        "<params>"
        "<param>"
        "<value><i4>6</i4></value>"
        "</param>"
        "</params>"
        "</methodCall>"
        "</query>"
        "</iq>");

    QXmppRpcInvokeIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.method(), QLatin1String("examples.getStateName"));
    QCOMPARE(iq.arguments(), QVariantList() << int(6));
    serializePacket(iq, xml);
}

void tst_QXmppRpcIq::testResponse()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"requester@company-b.com/jrpc-client\""
        " from=\"responder@company-a.com/jrpc-server\""
        " type=\"result\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodResponse>"
        "<params>"
        "<param>"
        "<value><string>Colorado</string></value>"
        "</param>"
        "</params>"
        "</methodResponse>"
        "</query>"
        "</iq>");

    QXmppRpcResponseIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.faultCode(), 0);
    QCOMPARE(iq.faultString(), QString());
    QCOMPARE(iq.values(), QVariantList() << QString("Colorado"));
    serializePacket(iq, xml);
}

void tst_QXmppRpcIq::testResponseFault()
{
    const QByteArray xml(
        "<iq"
        " id=\"rpc1\""
        " to=\"requester@company-b.com/jrpc-client\""
        " from=\"responder@company-a.com/jrpc-server\""
        " type=\"result\">"
        "<query xmlns=\"jabber:iq:rpc\">"
        "<methodResponse>"
        "<fault>"
        "<value>"
            "<struct>"
                "<member>"
                    "<name>faultCode</name>"
                    "<value><i4>404</i4></value>"
                "</member>"
                "<member>"
                    "<name>faultString</name>"
                    "<value><string>Not found</string></value>"
                "</member>"
            "</struct>"
        "</value>"
        "</fault>"
        "</methodResponse>"
        "</query>"
        "</iq>");

    QXmppRpcResponseIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.faultCode(), 404);
    QCOMPARE(iq.faultString(), QLatin1String("Not found"));
    QCOMPARE(iq.values(), QVariantList());
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppRpcIq)
#include "tst_qxmpprpciq.moc"
