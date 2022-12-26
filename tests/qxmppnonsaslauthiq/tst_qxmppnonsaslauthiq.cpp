// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppNonSASLAuth.h"

#include "util.h"

class tst_QXmppNonSASLAuthIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testGet();
    Q_SLOT void testSetPlain();
    Q_SLOT void testSetDigest();
};

void tst_QXmppNonSASLAuthIq::testGet()
{
    // Client requests authentication fields from server
    const QByteArray xml(
        "<iq id=\"auth1\" to=\"shakespeare.lit\" type=\"get\">"
        "<query xmlns=\"jabber:iq:auth\"/>"
        "</iq>");

    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    serializePacket(iq, xml);
}

void tst_QXmppNonSASLAuthIq::testSetPlain()
{
    // Client provides required information (plain)
    const QByteArray xml(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<password>Calli0pe</password>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.digest(), QByteArray());
    QCOMPARE(iq.password(), QLatin1String("Calli0pe"));
    QCOMPARE(iq.resource(), QLatin1String("globe"));
    serializePacket(iq, xml);
}

void tst_QXmppNonSASLAuthIq::testSetDigest()
{
    // Client provides required information (digest)
    const QByteArray xml(
        "<iq id=\"auth2\" type=\"set\">"
        "<query xmlns=\"jabber:iq:auth\">"
        "<username>bill</username>"
        "<digest>48fc78be9ec8f86d8ce1c39c320c97c21d62334d</digest>"
        "<resource>globe</resource>"
        "</query>"
        "</iq>");
    QXmppNonSASLAuthIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.username(), QLatin1String("bill"));
    QCOMPARE(iq.digest(), QByteArray("\x48\xfc\x78\xbe\x9e\xc8\xf8\x6d\x8c\xe1\xc3\x9c\x32\x0c\x97\xc2\x1d\x62\x33\x4d"));
    QCOMPARE(iq.password(), QString());
    QCOMPARE(iq.resource(), QLatin1String("globe"));
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppNonSASLAuthIq)
#include "tst_qxmppnonsaslauthiq.moc"
