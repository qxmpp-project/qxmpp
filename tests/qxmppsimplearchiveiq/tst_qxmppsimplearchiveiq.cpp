/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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
#include "QXmppSimpleArchiveIq.h"
#include "QXmppSimpleArchivePreferenceIq.h"
#include "QXmppSimpleArchiveManager.h"
#include "util.h"

class tst_QXmppSimpleArchiveIq : public QObject
{
    Q_OBJECT

private slots:
    void testRequestAll();
    void testRequestAllByJID();
    void testRequestAllByStartDate();
    void testRequestAllBetweenDates();
    void testRequestWithLimit();
    void testRequestWithLimitAfter();
    void testRequestWithLimitBefore();
    void testPreferenceSet();
    void testForwardedMessage();
};

// XEP-0313: 4. Querying the archive
void tst_QXmppSimpleArchiveIq::testRequestAll()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\"/>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
}

// XEP-0313: 4.1.1. Filtering by JID
void tst_QXmppSimpleArchiveIq::testRequestAllByJID()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<with>juliet@capulet.lit</with>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setWith(QString("juliet@capulet.lit"));
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.with(), QString("juliet@capulet.lit"));
}

// XEP-0313: 4.1.2. Filtering by time received
void tst_QXmppSimpleArchiveIq::testRequestAllByStartDate()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<start>2010-06-07T01:02:03Z</start>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setStart(QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.start(), QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
}

// XEP-0313: 4.1.2. Filtering by time received
void tst_QXmppSimpleArchiveIq::testRequestAllBetweenDates()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<start>2010-06-07T01:02:03Z</start>"
                        "<end>2011-07-08T01:02:03Z</end>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setStart(QDateTime(QDate(2010, 6, 7), QTime(2,2,3)));
    serialisationMessage.setEnd(QDateTime(QDate(2011, 7, 8), QTime(2,2,3)));
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.start(), QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
    QCOMPARE(parsedIq.end(), QDateTime(QDate(2011, 7, 8), QTime(2,2,3)));
}

// XEP-0313: 4.1.3. Limiting results
void tst_QXmppSimpleArchiveIq::testRequestWithLimit()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<start>2010-06-07T01:02:03Z</start>"
                        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
                            "<max>10</max>"
                        "</set>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setStart(QDateTime(QDate(2010, 6, 7), QTime(2,2,3)));
    QXmppResultSetQuery rsm;
    rsm.setMax(10);
    serialisationMessage.setResultSetQuery(rsm);
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.start(), QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
    QCOMPARE(parsedIq.resultSetQuery().max(), 10);
}

// XEP-0313: 4.1.3. Limiting results
void tst_QXmppSimpleArchiveIq::testRequestWithLimitAfter()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<start>2010-06-07T01:02:03Z</start>"
                        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
                            "<max>10</max>"
                            "<after>09af3-cc343-b409f</after>"
                        "</set>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setStart(QDateTime(QDate(2010, 6, 7), QTime(2,2,3)));
    QXmppResultSetQuery rsm;
    rsm.setMax(10);
    rsm.setAfter(QString("09af3-cc343-b409f"));
    serialisationMessage.setResultSetQuery(rsm);
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.start(), QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
    QCOMPARE(parsedIq.resultSetQuery().max(), 10);
    QCOMPARE(parsedIq.resultSetQuery().after(), QString("09af3-cc343-b409f"));
}

// XEP-0313: 4.1.3. Limiting results
void tst_QXmppSimpleArchiveIq::testRequestWithLimitBefore()
{
    QXmppSimpleArchiveQueryIq serialisationMessage;

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"get\">"
                        "<query xmlns=\"urn:xmpp:mam:tmp\" "
                            "queryid=\"f27\">"
                        "<start>2010-06-07T01:02:03Z</start>"
                        "<set xmlns=\"http://jabber.org/protocol/rsm\">"
                            "<max>10</max>"
                            "<before>09af3-cc343-b409f</before>"
                        "</set>"
                        "</query>"
                "</iq>");

    // tests
    serialisationMessage.setType(QXmppIq::Get);
    serialisationMessage.setId(QString("juliet1"));
    serialisationMessage.setQueryId(QString("f27"));
    serialisationMessage.setStart(QDateTime(QDate(2010, 6, 7), QTime(2,2,3)));
    QXmppResultSetQuery rsm;
    rsm.setMax(10);
    rsm.setBefore(QString("09af3-cc343-b409f"));
    serialisationMessage.setResultSetQuery(rsm);
    serializePacket(serialisationMessage, expectedXml);

    QXmppSimpleArchiveQueryIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.queryId(), QString("f27"));
    QCOMPARE(parsedIq.start(), QDateTime(QDate(2010, 06, 07), QTime(2,2,3)));
    QCOMPARE(parsedIq.resultSetQuery().max(), 10);
    QCOMPARE(parsedIq.resultSetQuery().before(), QString("09af3-cc343-b409f"));
}

// XEP-0313: 5.1 preferences
void tst_QXmppSimpleArchiveIq::testPreferenceSet()
{
    QXmppSimpleArchivePreferenceIq prefIq(QXmppSimpleArchivePreferenceIq::Roster);

    // expected
    const QByteArray expectedXml(
                "<iq "
                    "id=\"juliet1\" "
                    "type=\"set\">"
                        "<prefs xmlns=\"urn:xmpp:mam:tmp\" "
                            "default=\"roster\">"
                        "<always>"
                            "<jid>romeo@montague.lit</jid>"
                            "<jid>susan@montague.lit</jid>"
                        "</always>"
                        "<never>"
                            "<jid>montague@montague.lit</jid>"
                            "<jid>bridget@montague.lit</jid>"
                        "</never>"
                        "</prefs>"
                "</iq>");

    // tests
    prefIq.setType(QXmppIq::Set);
    prefIq.setId(QString("juliet1"));
    prefIq.addAlwaysArchive(QString("romeo@montague.lit"));
    prefIq.addAlwaysArchive(QString("susan@montague.lit"));
    prefIq.addNeverArchive(QString("montague@montague.lit"));
    prefIq.addNeverArchive(QString("bridget@montague.lit"));
    serializePacket(prefIq, expectedXml);

    QXmppSimpleArchivePreferenceIq parsedIq;
    parsePacket(parsedIq, expectedXml);
    QCOMPARE(parsedIq.id(), QString("juliet1"));
    QCOMPARE(parsedIq.archiveDefault(), QXmppSimpleArchivePreferenceIq::Roster);
    QCOMPARE(parsedIq.alwaysArchive().size(), 2);
    QCOMPARE(parsedIq.neverArchive().size(), 2);
}

void tst_QXmppSimpleArchiveIq::testForwardedMessage()
{
    const QByteArray inputXml(
                "<message "
                    "from=\"juliet1\" "
                    "to=\"romeo1\">"
                    "<result "
                        "xmlns=\"urn:xmpp:mam:tmp\" "
                        "queryid=\"query_query1\" "
                        "id=\"42073\">"
                        "<forwarded xmlns=\"urn:xmpp:forward:0\">"
                            "<delay xmlns=\"urn:xmpp:delay\" "
                                "from=\"juliet1\" "
                                "stamp=\"2013-09-11T15:04:47.524933Z\">"
                            "</delay>"
                            "<message type=\"chat\" "
                                "to=\"romeo1\" "
                                "id=\"C1945F99-2304-4E6E-8A9E-4CDC1F274C02\">"
                                "<body>k tnx bye</body>"
                                "<active xmlns=\"http://jabber.org/protocol/chatstates\"/>"
                                "<markable xmlns=\"urn:xmpp:chat-markers:0\"/>"
                                "<allow-permanent-storage xmlns=\"urn:xmpp:hints\"/>"
                            "</message>"
                        "</forwarded>"
                    "</result>"
                "</message>");

    QXmppSimpleArchiveManager manager;

    QDomDocument doc;
    QCOMPARE(doc.setContent(inputXml, true), true);
    QDomElement element = doc.documentElement();

    manager.retrieveMessages("query1");
    const bool parsedOk = manager.handleStanza(element);
    QCOMPARE(parsedOk, true);

    QXmppMessage message;
    parsePacket(message, inputXml);
    QCOMPARE(message.hasMaMMessage(), true);
    QCOMPARE(message.from(), QString("juliet1"));
    QCOMPARE(message.mamMessage().to(), QString("romeo1"));
    QCOMPARE(message.mamMessage().body(), QString("k tnx bye"));
}

QTEST_MAIN(tst_QXmppSimpleArchiveIq)
#include "tst_qxmppsimplearchiveiq.moc"
