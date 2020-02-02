/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppStanza.h"

#include "util.h"
#include <QObject>

class tst_QXmppStanza : public QObject
{
    Q_OBJECT

private slots:
    void testExtendedAddress_data();
    void testExtendedAddress();

    void testErrorFileTooLarge();
    void testErrorRetry();
};

void tst_QXmppStanza::testExtendedAddress_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("delivered");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("jid");
    QTest::addColumn<QString>("type");

    QTest::newRow("simple")
        << QByteArray(R"(<address jid="foo@example.com/QXmpp" type="bcc"/>)")
        << false
        << QString()
        << QString("foo@example.com/QXmpp")
        << QString("bcc");

    QTest::newRow("full")
        << QByteArray(R"(<address delivered="true" desc="some description" jid="foo@example.com/QXmpp" type="bcc"/>)")
        << true
        << QString("some description")
        << QString("foo@example.com/QXmpp")
        << QString("bcc");
}

void tst_QXmppStanza::testExtendedAddress()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, delivered);
    QFETCH(QString, description);
    QFETCH(QString, jid);
    QFETCH(QString, type);

    QXmppExtendedAddress address;
    parsePacket(address, xml);
    QCOMPARE(address.isDelivered(), delivered);
    QCOMPARE(address.description(), description);
    QCOMPARE(address.jid(), jid);
    QCOMPARE(address.type(), type);
    serializePacket(address, xml);
}

void tst_QXmppStanza::testErrorFileTooLarge()
{
    const QByteArray xml(
        "<error type=\"modify\">"
          "<not-acceptable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
          "<text xml:lang=\"en\" "
                "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
            "File too large. The maximum file size is 20000 bytes"
          "</text>"
          "<file-too-large xmlns=\"urn:xmpp:http:upload:0\">"
            "<max-file-size>20000</max-file-size>"
          "</file-too-large>"
        "</error>"
    );

    QXmppStanza::Error error;
    parsePacket(error, xml);
    QCOMPARE(error.type(), QXmppStanza::Error::Modify);
    QCOMPARE(error.text(), QString("File too large. The maximum file size is "
                                   "20000 bytes"));
    QCOMPARE(error.condition(), QXmppStanza::Error::NotAcceptable);
    QVERIFY(error.fileTooLarge());
    QCOMPARE(error.maxFileSize(), 20000);
    serializePacket(error, xml);

    // test setters
    error.setMaxFileSize(60000);
    QCOMPARE(error.maxFileSize(), 60000);
    error.setFileTooLarge(false);
    QVERIFY(!error.fileTooLarge());

    QXmppStanza::Error e2;
    e2.setMaxFileSize(123000);
    QVERIFY(e2.fileTooLarge());
}

void tst_QXmppStanza::testErrorRetry()
{
    const QByteArray xml(
        "<error type=\"wait\">"
          "<resource-constraint "
            "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
          "<text xml:lang=\"en\" "
                "xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\">"
            "Quota reached. You can only upload 5 files in 5 minutes"
          "</text>"
          "<retry xmlns=\"urn:xmpp:http:upload:0\" "
                 "stamp=\"2017-12-03T23:42:05Z\"/>"
        "</error>"
    );

    QXmppStanza::Error error;
    parsePacket(error, xml);
    QCOMPARE(error.type(), QXmppStanza::Error::Wait);
    QCOMPARE(error.text(), QString("Quota reached. You can only upload 5 "
                                   "files in 5 minutes"));
    QCOMPARE(error.condition(), QXmppStanza::Error::ResourceConstraint);
    QCOMPARE(error.retryDate(), QDateTime(QDate(2017, 12, 03),
                                          QTime(23, 42, 05), Qt::UTC));
    serializePacket(error, xml);

    // test setter
    error.setRetryDate(QDateTime(QDate(1985, 10, 26), QTime(1, 35)));
    QCOMPARE(error.retryDate(), QDateTime(QDate(1985, 10, 26), QTime(1, 35)));
}

QTEST_MAIN(tst_QXmppStanza)
#include "tst_qxmppstanza.moc"
