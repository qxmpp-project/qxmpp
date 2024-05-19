// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVersionIq.h"

#include "util.h"

#include <QObject>

class tst_QXmppVersionIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testVersionGet();
    Q_SLOT void testVersionResult();
};

void tst_QXmppVersionIq::testVersionGet()
{
    const QByteArray xmlGet(
        "<iq id=\"version_1\" to=\"juliet@capulet.com/balcony\" "
        "from=\"romeo@montague.net/orchard\" type=\"get\">"
        "<query xmlns=\"jabber:iq:version\"/></iq>");

    QXmppVersionIq verIqGet;
    parsePacket(verIqGet, xmlGet);
    QCOMPARE(verIqGet.id(), QLatin1String("version_1"));
    QCOMPARE(verIqGet.to(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(verIqGet.from(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(verIqGet.type(), QXmppIq::Get);
    serializePacket(verIqGet, xmlGet);
}

void tst_QXmppVersionIq::testVersionResult()
{
    const QByteArray xmlResult(
        "<iq id=\"version_1\" to=\"romeo@montague.net/orchard\" "
        "from=\"juliet@capulet.com/balcony\" type=\"result\">"
        "<query xmlns=\"jabber:iq:version\">"
        "<name>qxmpp</name>"
        "<os>Windows-XP</os>"
        "<version>0.2.0</version>"
        "</query></iq>");

    QXmppVersionIq verIqResult;
    parsePacket(verIqResult, xmlResult);
    QCOMPARE(verIqResult.id(), QLatin1String("version_1"));
    QCOMPARE(verIqResult.to(), QLatin1String("romeo@montague.net/orchard"));
    QCOMPARE(verIqResult.from(), QLatin1String("juliet@capulet.com/balcony"));
    QCOMPARE(verIqResult.type(), QXmppIq::Result);
    QCOMPARE(verIqResult.name(), u"qxmpp"_s);
    QCOMPARE(verIqResult.version(), u"0.2.0"_s);
    QCOMPARE(verIqResult.os(), u"Windows-XP"_s);

    serializePacket(verIqResult, xmlResult);
}

QTEST_MAIN(tst_QXmppVersionIq)
#include "tst_qxmppversioniq.moc"
