// SPDX-FileCopyrightText: 2012 Oliver Goffart <ogoffart@woboq.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppResultSet.h"

#include "util.h"

#include <QObject>

class tst_QXmppResultSet : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testQuery_data();
    Q_SLOT void testQuery();
    Q_SLOT void testReply_data();
    Q_SLOT void testReply();
};

void tst_QXmppResultSet::testQuery_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("max");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("after");

    QTest::newRow("Example 3") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<max>10</max>"
                                             "</set>")
                               << 10 << -1 << QString() << QString();

    QTest::newRow("Example 5") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<max>10</max>"
                                             "<after>peterpan@neverland.lit</after>"
                                             "</set>")
                               << 10 << -1 << QString() << u"peterpan@neverland.lit"_s;

    QTest::newRow("Example 5") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<max>10</max>"
                                             "<before>peter@pixyland.org</before>"
                                             "</set>")
                               << 10 << -1 << u"peter@pixyland.org"_s << QString();

    QTest::newRow("Example 11") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                              "<max>10</max>"
                                              "<before/>"
                                              "</set>")
                                << 10 << -1 << u""_s << QString();

    QTest::newRow("Example 12") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                              "<max>10</max>"
                                              "<index>371</index>"
                                              "</set>")
                                << 10 << 371 << QString() << QString();

    QTest::newRow("Example 15") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                              "<max>0</max>"
                                              "</set>")
                                << 0 << -1 << QString() << QString();
}

void tst_QXmppResultSet::testQuery()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, max);
    QFETCH(int, index);
    QFETCH(QString, before);
    QFETCH(QString, after);

    QXmppResultSetQuery iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.max(), max);
    QCOMPARE(iq.index(), index);
    QCOMPARE(iq.before(), before);
    QCOMPARE(iq.before().isNull(), before.isNull());
    QCOMPARE(iq.after(), after);
    QCOMPARE(iq.after().isNull(), after.isNull());
    serializePacket(iq, xml);
}

void tst_QXmppResultSet::testReply_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("last");

    QTest::newRow("Example 4") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<first index=\"0\">stpeter@jabber.org</first>"
                                             "<last>peterpan@neverland.lit</last>"
                                             "<count>800</count>"
                                             "</set>")
                               << 800 << 0 << u"stpeter@jabber.org"_s << u"peterpan@neverland.lit"_s;

    QTest::newRow("Example 6") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<first index=\"0\">stpeter@jabber.org</first>"
                                             "<last>peterpan@neverland.lit</last>"
                                             "<count>800</count>"
                                             "</set>")
                               << 800 << 0 << u"stpeter@jabber.org"_s << u"peterpan@neverland.lit"_s;
    QTest::newRow("Example 4") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<first index=\"10\">peter@pixyland.org</first>"
                                             "<last>peter@rabbit.lit</last>"
                                             "<count>800</count>"
                                             "</set>")
                               << 800 << 10 << u"peter@pixyland.org"_s << u"peter@rabbit.lit"_s;

    QTest::newRow("Example 7") << QByteArray("<set xmlns=\"http://jabber.org/protocol/rsm\">"
                                             "<count>790</count>"
                                             "</set>")
                               << 790 << -1 << QString() << QString();
}

void tst_QXmppResultSet::testReply()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, count);
    QFETCH(int, index);
    QFETCH(QString, first);
    QFETCH(QString, last);

    QXmppResultSetReply iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.count(), count);
    QCOMPARE(iq.index(), index);
    QCOMPARE(iq.first(), first);
    QCOMPARE(iq.first().isNull(), first.isNull());
    QCOMPARE(iq.last(), last);
    QCOMPARE(iq.last().isNull(), last.isNull());
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppResultSet)
#include "tst_qxmppresultset.moc"
