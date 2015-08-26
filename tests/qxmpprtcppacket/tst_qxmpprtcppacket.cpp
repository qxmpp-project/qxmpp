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
#include <QtTest>
#include "QXmppRtcpPacket.h"

class tst_QXmppRtcpPacket : public QObject
{
    Q_OBJECT

private slots:
    void testBad();
    void testSenderReport();
    void testSourceDescription();
};

void tst_QXmppRtcpPacket::testBad()
{
    QXmppRtcpPacket packet;

    // too short
    QCOMPARE(packet.decode(QByteArray()), false);
}

void tst_QXmppRtcpPacket::testSenderReport()
{
    QByteArray data("\x80\xc8\x00\x06\x27\xa6\xe4\xc1\xd9\x7f\xec\x7d\x92\xac\xd9\xe8\xdd\x9e\x32\x57\x00\x00\x00\x74\x00\x00\x48\x80", 28);

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::SenderReport));

    QCOMPARE(packet.receiverReports().size(), 0);

    QCOMPARE(packet.senderReport().ntpStamp(), quint64(15672505252348484072));
    QCOMPARE(packet.senderReport().octetCount(), quint32(18560));
    QCOMPARE(packet.senderReport().packetCount(), quint32(116));
    QCOMPARE(packet.senderReport().rtpStamp(), quint32(3718132311));
    QCOMPARE(packet.senderReport().ssrc(), quint32(665248961));

    QCOMPARE(packet.sourceDescriptions().size(), 0);

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testSourceDescription()
{
    QByteArray data("\x81\xca\x00\x0c\x27\xa6\xe4\xc1\x01\x26\x7b\x64\x30\x33\x61\x37\x63\x34\x38\x2d\x64\x39\x30\x36\x2d\x34\x62\x39\x61\x2d\x39\x38\x32\x30\x2d\x31\x31\x31\x38\x30\x32\x64\x63\x64\x35\x37\x38\x7d\x00\x00\x00\x00", 52);

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::SourceDescription));

    QCOMPARE(packet.receiverReports().size(), 0);

    QCOMPARE(packet.senderReport().ntpStamp(), quint64(0));
    QCOMPARE(packet.senderReport().octetCount(), quint32(0));
    QCOMPARE(packet.senderReport().packetCount(), quint32(0));
    QCOMPARE(packet.senderReport().rtpStamp(), quint32(0));
    QCOMPARE(packet.senderReport().ssrc(), quint32(0));

    QCOMPARE(packet.sourceDescriptions().size(), 1);
    QCOMPARE(packet.sourceDescriptions()[0].cname(), QLatin1String("{d03a7c48-d906-4b9a-9820-111802dcd578}"));
    QCOMPARE(packet.sourceDescriptions()[0].name(), QString());
    QCOMPARE(packet.sourceDescriptions()[0].ssrc(), quint32(665248961));

    QCOMPARE(packet.encode(), data);
}

QTEST_MAIN(tst_QXmppRtcpPacket)
#include "tst_qxmpprtcppacket.moc"
