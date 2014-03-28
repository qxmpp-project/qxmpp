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
#include "QXmppStun.h"
#include "util.h"

class tst_QXmppStunMessage : public QObject
{
    Q_OBJECT

private slots:
    void testFingerprint();
    void testIntegrity();
    void testIPv4Address();
    void testIPv6Address();
    void testXorIPv4Address();
    void testXorIPv6Address();
};

void tst_QXmppStunMessage::testFingerprint()
{
    // without fingerprint
    QXmppStunMessage msg;
    msg.setType(0x0001);
    QCOMPARE(msg.encode(QByteArray(), false),
             QByteArray("\x00\x01\x00\x00\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20));

    // with fingerprint
    QCOMPARE(msg.encode(QByteArray(), true),
             QByteArray("\x00\x01\x00\x08\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x28\x00\x04\xB2\xAA\xF9\xF6", 28));
}

void tst_QXmppStunMessage::testIntegrity()
{
    QXmppStunMessage msg;
    msg.setType(0x0001);
    QCOMPARE(msg.encode(QByteArray("somesecret"), false),
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x14\x96\x4B\x40\xD1\x84\x67\x6A\xFD\xB5\xE0\x7C\xC5\x1F\xFB\xBD\xA2\x61\xAF\xB1\x26", 44));
}

void tst_QXmppStunMessage::testIPv4Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.mappedHost = QHostAddress("127.0.0.1");
    msg.mappedPort = 12345;
    QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x0C\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x08\x00\x01\x30\x39\x7F\x00\x00\x01", 32));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.mappedHost, QHostAddress("127.0.0.1"));
    QCOMPARE(msg2.mappedPort, quint16(12345));
}

void tst_QXmppStunMessage::testIPv6Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.mappedHost = QHostAddress("::1");
    msg.mappedPort = 12345;
    const QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x14\x00\x02\x30\x39\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 44));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.mappedHost, QHostAddress("::1"));
    QCOMPARE(msg2.mappedPort, quint16(12345));
}

void tst_QXmppStunMessage::testXorIPv4Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.xorMappedHost = QHostAddress("127.0.0.1");
    msg.xorMappedPort = 12345;
    QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x0C\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x08\x00\x01\x11\x2B\x5E\x12\xA4\x43", 32));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.xorMappedHost, QHostAddress("127.0.0.1"));
    QCOMPARE(msg2.xorMappedPort, quint16(12345));
}

void tst_QXmppStunMessage::testXorIPv6Address()
{
    // encode
    QXmppStunMessage msg;
    msg.setType(0x0001);
    msg.xorMappedHost = QHostAddress("::1");
    msg.xorMappedPort = 12345;
    const QByteArray packet = msg.encode(QByteArray(), false);
    QCOMPARE(packet,
             QByteArray("\x00\x01\x00\x18\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x14\x00\x02\x11\x2B\x21\x12\xA4\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 44));

    // decode
    QXmppStunMessage msg2;
    msg2.decode(packet);
    QCOMPARE(msg2.xorMappedHost, QHostAddress("::1"));
    QCOMPARE(msg2.xorMappedPort, quint16(12345));
}

QTEST_MAIN(tst_QXmppStunMessage)
#include "tst_qxmppstunmessage.moc"
