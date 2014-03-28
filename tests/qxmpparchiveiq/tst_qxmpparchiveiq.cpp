/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#include "QXmppArchiveIq.h"
#include "util.h"

class tst_QXmppArchiveIq : public QObject
{
    Q_OBJECT

private slots:
    void testArchiveList_data();
    void testArchiveList();
    void testArchiveChat_data();
    void testArchiveChat();
    void testArchiveRemove();
    void testArchiveRetrieve_data();
    void testArchiveRetrieve();
};

void tst_QXmppArchiveIq::testArchiveList_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("max");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"list_1\" type=\"get\">"
        "<list xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\"/>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"list_1\" type=\"get\">"
        "<list xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</list>"
        "</iq>") << 30;
}

void tst_QXmppArchiveIq::testArchiveList()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, max);

    QXmppArchiveListIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("list_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.end(), QDateTime(QDate(1479, 7, 21), QTime(4, 0, 0), Qt::UTC));
    QCOMPARE(iq.resultSetQuery().max(), max);
    serializePacket(iq, xml);
}

void tst_QXmppArchiveIq::testArchiveChat_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("count");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"chat_1\" type=\"result\">"
        "<chat xmlns=\"urn:xmpp:archive\""
        " with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:56:15Z\""
        " subject=\"She speaks!\""
        " version=\"4\""
        ">"
        "<from secs=\"0\"><body>Art thou not Romeo, and a Montague?</body></from>"
        "<to secs=\"11\"><body>Neither, fair saint, if either thee dislike.</body></to>"
        "<from secs=\"7\"><body>How cam'st thou hither, tell me, and wherefore?</body></from>"
        "</chat>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"chat_1\" type=\"result\">"
        "<chat xmlns=\"urn:xmpp:archive\""
        " with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:56:15Z\""
        " subject=\"She speaks!\""
        " version=\"4\""
        ">"
        "<from secs=\"0\"><body>Art thou not Romeo, and a Montague?</body></from>"
        "<to secs=\"11\"><body>Neither, fair saint, if either thee dislike.</body></to>"
        "<from secs=\"7\"><body>How cam'st thou hither, tell me, and wherefore?</body></from>"
        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
        "<count>3</count>"
        "</set>"
        "</chat>"
        "</iq>") << 3;
}

void tst_QXmppArchiveIq::testArchiveChat()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, count);

    QXmppArchiveChatIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Result);
    QCOMPARE(iq.id(), QLatin1String("chat_1"));
    QCOMPARE(iq.chat().with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.chat().messages().size(), 3);
    QCOMPARE(iq.chat().messages()[0].isReceived(), true);
    QCOMPARE(iq.chat().messages()[0].body(), QLatin1String("Art thou not Romeo, and a Montague?"));
    QCOMPARE(iq.chat().messages()[0].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 15), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].isReceived(), false);
    QCOMPARE(iq.chat().messages()[1].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 26), Qt::UTC));
    QCOMPARE(iq.chat().messages()[1].body(), QLatin1String("Neither, fair saint, if either thee dislike."));
    QCOMPARE(iq.chat().messages()[2].isReceived(), true);
    QCOMPARE(iq.chat().messages()[2].date(), QDateTime(QDate(1469, 7, 21), QTime(2, 56, 33), Qt::UTC));
    QCOMPARE(iq.chat().messages()[2].body(), QLatin1String("How cam'st thou hither, tell me, and wherefore?"));
    QCOMPARE(iq.resultSetReply().count(), count);
    serializePacket(iq, xml);
}

void tst_QXmppArchiveIq::testArchiveRemove()
{
    const QByteArray xml(
        "<iq id=\"remove_1\" type=\"set\">"
        "<remove xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\" end=\"1479-07-21T04:00:00Z\"/>"
        "</iq>");

    QXmppArchiveRemoveIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Set);
    QCOMPARE(iq.id(), QLatin1String("remove_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.end(), QDateTime(QDate(1479, 7, 21), QTime(4, 0, 0), Qt::UTC));
    serializePacket(iq, xml);
}

void tst_QXmppArchiveIq::testArchiveRetrieve_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("max");

    QTest::newRow("no rsm") <<
        QByteArray(
        "<iq id=\"retrieve_1\" type=\"get\">"
        "<retrieve xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\"/>"
        "</iq>") << -1;

    QTest::newRow("with rsm") <<
        QByteArray(
        "<iq id=\"retrieve_1\" type=\"get\">"
        "<retrieve xmlns=\"urn:xmpp:archive\" with=\"juliet@capulet.com\""
        " start=\"1469-07-21T02:00:00Z\">"
            "<set xmlns=\"http://jabber.org/protocol/rsm\">"
            "<max>30</max>"
            "</set>"
        "</retrieve>"
        "</iq>") << 30;
}

void tst_QXmppArchiveIq::testArchiveRetrieve()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, max);

    QXmppArchiveRetrieveIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.id(), QLatin1String("retrieve_1"));
    QCOMPARE(iq.with(), QLatin1String("juliet@capulet.com"));
    QCOMPARE(iq.start(), QDateTime(QDate(1469, 7, 21), QTime(2, 0, 0), Qt::UTC));
    QCOMPARE(iq.resultSetQuery().max(), max);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppArchiveIq)
#include "tst_qxmpparchiveiq.moc"
