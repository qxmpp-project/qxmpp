// SPDX-FileCopyrightText: 2012 Oliver Goffart <ogoffart@woboq.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStreamInitiationIq_p.h"
#include "QXmppTransferManager.h"

#include "util.h"

class tst_QXmppStreamInitiationIq : public QObject
{
    Q_OBJECT

    Q_SLOT void testFileInfo_data();
    Q_SLOT void testFileInfo();
    Q_SLOT void testOffer();
    Q_SLOT void testResult();
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
        << QDateTime().toUTC()
        << QString()
        << QByteArray()
        << u"test.txt"_s
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
        << u"This is a test. If this were a real file..."_s
        << QByteArray::fromHex("552da749930852c69ae5d2141d3766b1")
        << u"test.txt"_s
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
    QCOMPARE(iq.fileInfo().name(), u"test.txt"_s);
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
