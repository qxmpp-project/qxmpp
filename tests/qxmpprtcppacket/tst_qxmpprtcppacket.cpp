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
    void testGoodbye();
    void testGoodbyeWithReason();
    void testReceiverReport();
    void testSenderReport();
    void testSenderReportWithReceiverReport();
    void testSourceDescription();
};

void tst_QXmppRtcpPacket::testBad()
{
    QXmppRtcpPacket packet;

    // too short
    QCOMPARE(packet.decode(QByteArray()), false);
}

void tst_QXmppRtcpPacket::testGoodbye()
{
    const QByteArray data = QByteArray::fromHex("81cb000133425619");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QString());
    QCOMPARE(packet.goodbyeSsrcs().size(), 1);
    QCOMPARE(packet.goodbyeSsrcs()[0], quint32(859985433));
    QCOMPARE(packet.receiverReports().size(), 0);
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(0));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(0));
    QCOMPARE(packet.sourceDescriptions().size(), 0);
    QCOMPARE(packet.ssrc(), quint32(0));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::Goodbye));

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testGoodbyeWithReason()
{
    const QByteArray data = QByteArray::fromHex("81cb0003334256190462796521000000");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QLatin1String("bye!"));
    QCOMPARE(packet.goodbyeSsrcs().size(), 1);
    QCOMPARE(packet.goodbyeSsrcs()[0], quint32(859985433));
    QCOMPARE(packet.receiverReports().size(), 0);
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(0));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(0));
    QCOMPARE(packet.sourceDescriptions().size(), 0);
    QCOMPARE(packet.ssrc(), quint32(0));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::Goodbye));

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testReceiverReport()
{
    const QByteArray data = QByteArray::fromHex("81c9000741f3bca22886dfa00000000000005eb90000001000000000fffbdae2");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QString());
    QCOMPARE(packet.goodbyeSsrcs().size(), 0);
    QCOMPARE(packet.receiverReports().size(), 1);
    QCOMPARE(packet.receiverReports()[0].dlsr(), quint32(4294695650));
    QCOMPARE(packet.receiverReports()[0].fractionLost(), quint8(0));
    QCOMPARE(packet.receiverReports()[0].jitter(), quint32(16));
    QCOMPARE(packet.receiverReports()[0].lsr(), quint32(0));
    QCOMPARE(packet.receiverReports()[0].ssrc(), quint32(679927712));
    QCOMPARE(packet.receiverReports()[0].totalLost(), quint32(0));
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(0));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(0));
    QCOMPARE(packet.sourceDescriptions().size(), 0);
    QCOMPARE(packet.ssrc(), quint32(1106492578));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::ReceiverReport));

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testSenderReport()
{
    const QByteArray data = QByteArray::fromHex("80c8000627a6e4c1d97fec7d92acd9e8dd9e32570000007400004880");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QString());
    QCOMPARE(packet.goodbyeSsrcs().size(), 0);
    QCOMPARE(packet.receiverReports().size(), 0);
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(15672505252348484072ULL));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(18560));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(116));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(3718132311));
    QCOMPARE(packet.ssrc(), quint32(665248961));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::SenderReport));
    QCOMPARE(packet.sourceDescriptions().size(), 0);

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testSenderReportWithReceiverReport()
{
    const QByteArray data = QByteArray::fromHex("81c8000c3efeb4decf80b8156fd6542c0000014000000003000001e081bc22520000000000007db50000002500000000fffbd605");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QString());
    QCOMPARE(packet.goodbyeSsrcs().size(), 0);
    QCOMPARE(packet.receiverReports().size(), 1);
    QCOMPARE(packet.receiverReports()[0].dlsr(), quint32(4294694405));
    QCOMPARE(packet.receiverReports()[0].fractionLost(), quint8(0));
    QCOMPARE(packet.receiverReports()[0].jitter(), quint32(37));
    QCOMPARE(packet.receiverReports()[0].lsr(), quint32(0));
    QCOMPARE(packet.receiverReports()[0].ssrc(), quint32(2176590418));
    QCOMPARE(packet.receiverReports()[0].totalLost(), quint32(0));
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(14952153165080187948ULL));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(480));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(3));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(320));
    QCOMPARE(packet.sourceDescriptions().size(), 0);
    QCOMPARE(packet.ssrc(), quint32(1056879838));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::SenderReport));

    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtcpPacket::testSourceDescription()
{
    const QByteArray data = QByteArray::fromHex("81ca000c27a6e4c101267b64303361376334382d643930362d346239612d393832302d3131313830326463643537387d00000000");

    QXmppRtcpPacket packet;
    QVERIFY(packet.decode(data));

    QCOMPARE(packet.goodbyeReason(), QString());
    QCOMPARE(packet.goodbyeSsrcs().size(), 0);
    QCOMPARE(packet.receiverReports().size(), 0);
    QCOMPARE(packet.senderInfo().ntpStamp(), quint64(0));
    QCOMPARE(packet.senderInfo().octetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().packetCount(), quint32(0));
    QCOMPARE(packet.senderInfo().rtpStamp(), quint32(0));
    QCOMPARE(packet.sourceDescriptions().size(), 1);
    QCOMPARE(packet.sourceDescriptions()[0].cname(), QLatin1String("{d03a7c48-d906-4b9a-9820-111802dcd578}"));
    QCOMPARE(packet.sourceDescriptions()[0].name(), QString());
    QCOMPARE(packet.sourceDescriptions()[0].ssrc(), quint32(665248961));
    QCOMPARE(packet.ssrc(), quint32(0));
    QCOMPARE(packet.type(), quint8(QXmppRtcpPacket::SourceDescription));

    QCOMPARE(packet.encode(), data);
}

QTEST_MAIN(tst_QXmppRtcpPacket)
#include "tst_qxmpprtcppacket.moc"
