/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Olivier Goffart
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

#include "QXmppStreamInitiationIq_p.h"
#include "QXmppTransferManager.h"
#include "util.h"

class tst_QXmppStreamInitiationIq : public QObject
{
    Q_OBJECT

private slots:
    void testFileInfo_data();
    void testFileInfo();
    void testOffer();
    void testResult();
};

void tst_QXmppStreamInitiationIq::testFileInfo_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QDateTime>("date");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QByteArray>("hash");
    QTest::addColumn<QString>("name");
    QTest::addColumn<qint64>("size");

    QTest::newRow("normal")
        << QByteArray("<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" name=\"test.txt\" size=\"1022\"/>")
        << QDateTime()
        << QString()
        << QByteArray()
        << QString("test.txt")
        << qint64(1022);

    QTest::newRow("full")
        << QByteArray("<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" "
            "date=\"1969-07-21T02:56:15Z\" "
            "hash=\"552da749930852c69ae5d2141d3766b1\" "
            "name=\"test.txt\" "
            "size=\"1022\">"
                "<desc>This is a test. If this were a real file...</desc>"
            "</file>")
        << QDateTime(QDate(1969, 7, 21), QTime(2, 56, 15), Qt::UTC)
        << QString("This is a test. If this were a real file...")
        << QByteArray::fromHex("552da749930852c69ae5d2141d3766b1")
        << QString("test.txt")
        << qint64(1022);
}

void tst_QXmppStreamInitiationIq::testFileInfo()
{
    QFETCH(QByteArray, xml);
    QFETCH(QDateTime, date);
    QFETCH(QString, description);
    QFETCH(QByteArray, hash);
    QFETCH(QString, name);
    QFETCH(qint64, size);

    QXmppTransferFileInfo info;
    parsePacket(info, xml);
    QCOMPARE(info.date(), date);
    QCOMPARE(info.description(), description);
    QCOMPARE(info.hash(), hash);
    QCOMPARE(info.name(), name);
    QCOMPARE(info.size(), size);
    serializePacket(info, xml);
}

void tst_QXmppStreamInitiationIq::testOffer()
{
    QByteArray xml(
        "<iq id=\"offer1\" to=\"receiver@jabber.org/resource\" type=\"set\">"
          "<si xmlns=\"http://jabber.org/protocol/si\" id=\"a0\" mime-type=\"text/plain\" profile=\"http://jabber.org/protocol/si/profile/file-transfer\">"
            "<file xmlns=\"http://jabber.org/protocol/si/profile/file-transfer\" name=\"test.txt\" size=\"1022\"/>"
            "<feature xmlns=\"http://jabber.org/protocol/feature-neg\">"
              "<x xmlns=\"jabber:x:data\" type=\"form\">"
                "<field type=\"list-single\" var=\"stream-method\">"
                  "<option><value>http://jabber.org/protocol/bytestreams</value></option>"
                  "<option><value>http://jabber.org/protocol/ibb</value></option>"
                "</field>"
              "</x>"
            "</feature>"
          "</si>"
        "</iq>");

    QXmppStreamInitiationIq iq;
    parsePacket(iq, xml);
    QVERIFY(!iq.fileInfo().isNull());
    QCOMPARE(iq.fileInfo().name(), QString("test.txt"));
    QCOMPARE(iq.fileInfo().size(), qint64(1022));
    serializePacket(iq, xml);
}

void tst_QXmppStreamInitiationIq::testResult()
{
    QByteArray xml(
        "<iq id=\"offer1\" to=\"sender@jabber.org/resource\" type=\"result\">"
          "<si xmlns=\"http://jabber.org/protocol/si\">"
            "<feature xmlns=\"http://jabber.org/protocol/feature-neg\">"
              "<x xmlns=\"jabber:x:data\" type=\"submit\">"
                "<field type=\"list-single\" var=\"stream-method\">"
                  "<value>http://jabber.org/protocol/bytestreams</value>"
                "</field>"
              "</x>"
            "</feature>"
          "</si>"
        "</iq>");

    QXmppStreamInitiationIq iq;
    parsePacket(iq, xml);
    QVERIFY(iq.fileInfo().isNull());
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppStreamInitiationIq)
#include "tst_qxmppstreaminitiationiq.moc"
