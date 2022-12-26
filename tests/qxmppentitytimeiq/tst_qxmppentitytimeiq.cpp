// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEntityTimeIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppEntityTimeIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testEntityTimeGet();
    Q_SLOT void testEntityTimeResult();
};

void tst_QXmppEntityTimeIq::testEntityTimeGet()
{
    const QByteArray xml("<iq id=\"time_1\" "
                         "to=\"juliet@capulet.com/balcony\" "
                         "from=\"romeo@montague.net/orchard\" type=\"get\">"
                         "<time xmlns=\"urn:xmpp:time\"/>"
                         "</iq>");

    QXmppEntityTimeIq entityTime;
    parsePacket(entityTime, xml);
    QCOMPARE(entityTime.id(), QLatin1String("time_1"));
    QCOMPARE(entityTime.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(entityTime.from(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(entityTime.type(), QXmppIq::Get);
    serializePacket(entityTime, xml);
}

void tst_QXmppEntityTimeIq::testEntityTimeResult()
{
    const QByteArray xml(
        "<iq id=\"time_1\" to=\"romeo@montague.net/orchard\" from=\"juliet@capulet.com/balcony\" type=\"result\">"
        "<time xmlns=\"urn:xmpp:time\">"
        "<tzo>-06:00</tzo>"
        "<utc>2006-12-19T17:58:35Z</utc>"
        "</time>"
        "</iq>");

    QXmppEntityTimeIq entityTime;
    parsePacket(entityTime, xml);
    QCOMPARE(entityTime.id(), QLatin1String("time_1"));
    QCOMPARE(entityTime.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(entityTime.to(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(entityTime.type(), QXmppIq::Result);
    QCOMPARE(entityTime.tzo(), -21600);
    QCOMPARE(entityTime.utc(), QDateTime(QDate(2006, 12, 19), QTime(17, 58, 35), Qt::UTC));
    serializePacket(entityTime, xml);
}

QTEST_MAIN(tst_QXmppEntityTimeIq)
#include "tst_qxmppentitytimeiq.moc"
