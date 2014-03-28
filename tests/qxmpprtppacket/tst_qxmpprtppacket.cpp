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
#include "QXmppRtpChannel.h"

class tst_QXmppRtpPacket : public QObject
{
    Q_OBJECT

private slots:
    void testBad();
    void testSimple();
    void testWithCsrc();
};

void tst_QXmppRtpPacket::testBad()
{
    QXmppRtpPacket packet;

    // too short
    QCOMPARE(packet.decode(QByteArray()), false);
    QCOMPARE(packet.decode(QByteArray("\x80\x00\x3e", 3)), false);
    QCOMPARE(packet.decode(QByteArray("\x84\x00\x3e\xd2\x00\x00\x00\x90\x5f\xbd\x16\x9e", 12)), false);

    // wrong RTP version
    QCOMPARE(packet.decode(QByteArray("\x40\x00\x3e\xd2\x00\x00\x00\x90\x5f\xbd\x16\x9e", 12)), false);
}

void tst_QXmppRtpPacket::testSimple()
{
    QByteArray data("\x80\x00\x3e\xd2\x00\x00\x00\x90\x5f\xbd\x16\x9e\x12\x34\x56", 15);
    QXmppRtpPacket packet;
    QCOMPARE(packet.decode(data), true);
    QCOMPARE(packet.version, quint8(2));
    QCOMPARE(packet.marker, false);
    QCOMPARE(packet.type, quint8(0));
    QCOMPARE(packet.sequence, quint16(16082));
    QCOMPARE(packet.stamp, quint32(144));
    QCOMPARE(packet.ssrc, quint32(1606227614));
    QCOMPARE(packet.csrc, QList<quint32>());
    QCOMPARE(packet.payload, QByteArray("\x12\x34\x56", 3));
    QCOMPARE(packet.encode(), data);
}

void tst_QXmppRtpPacket::testWithCsrc()
{
    QByteArray data("\x84\x00\x3e\xd2\x00\x00\x00\x90\x5f\xbd\x16\x9e\xab\xcd\xef\x01\xde\xad\xbe\xef\x12\x34\x56", 23);
    QXmppRtpPacket packet;
    QCOMPARE(packet.decode(data), true);
    QCOMPARE(packet.version, quint8(2));
    QCOMPARE(packet.marker, false);
    QCOMPARE(packet.type, quint8(0));
    QCOMPARE(packet.sequence, quint16(16082));
    QCOMPARE(packet.stamp, quint32(144));
    QCOMPARE(packet.ssrc, quint32(1606227614));
    qDebug() << packet.csrc;
    QCOMPARE(packet.csrc, QList<quint32>() << quint32(0xabcdef01) << quint32(0xdeadbeef));
    QCOMPARE(packet.payload, QByteArray("\x12\x34\x56", 3));
    QCOMPARE(packet.encode(), data);
}

QTEST_MAIN(tst_QXmppRtpPacket)
#include "tst_qxmpprtppacket.moc"
