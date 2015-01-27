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

#include <QObject>
#include "QXmppUtils.h"
#include "util.h"

class tst_QXmppUtils : public QObject
{
    Q_OBJECT

private slots:
    void testCrc32();
    void testHmac();
    void testJid();
    void testMime();
    void testLibVersion();
    void testTimezoneOffset();
};

void tst_QXmppUtils::testCrc32()
{
    quint32 crc = QXmppUtils::generateCrc32(QByteArray());
    QCOMPARE(crc, 0u);

    crc = QXmppUtils::generateCrc32(QByteArray("Hi There"));
    QCOMPARE(crc, 0xDB143BBEu);
}

void tst_QXmppUtils::testHmac()
{
    QByteArray hmac = QXmppUtils::generateHmacMd5(QByteArray(16, 0x0b), QByteArray("Hi There"));
    QCOMPARE(hmac, QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray("Jefe"), QByteArray("what do ya want for nothing?"));
    QCOMPARE(hmac, QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738"));

    hmac = QXmppUtils::generateHmacMd5(QByteArray(16, 0xaa), QByteArray(50, 0xdd));
    QCOMPARE(hmac, QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6"));
}

void tst_QXmppUtils::testJid()
{
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com/resource"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("foo@example.com"), QLatin1String("foo@example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToBareJid(QString()), QString());

    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com/resource"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("foo@example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain("example.com"), QLatin1String("example.com"));
    QCOMPARE(QXmppUtils::jidToDomain(QString()), QString());

    QCOMPARE(QXmppUtils::jidToResource("foo@example.com/resource"), QLatin1String("resource"));
    QCOMPARE(QXmppUtils::jidToResource("foo@example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToResource(QString()), QString());

    QCOMPARE(QXmppUtils::jidToUser("foo@example.com/resource"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("foo@example.com"), QLatin1String("foo"));
    QCOMPARE(QXmppUtils::jidToUser("example.com"), QString());
    QCOMPARE(QXmppUtils::jidToUser(QString()), QString());
}

// FIXME: how should we test MIME detection without expose getImageType?
#if 0
QString getImageType(const QByteArray &contents);

static void testMimeType(const QString &fileName, const QString fileType)
{
    // load file from resources
    QFile file(":/" + fileName);
    QCOMPARE(file.open(QIODevice::ReadOnly), true);
    QCOMPARE(getImageType(file.readAll()), fileType);
    file.close();
}

void tst_QXmppUtils::testMime()
{
    testMimeType("test.bmp", "image/bmp");
    testMimeType("test.gif", "image/gif");
    testMimeType("test.jpg", "image/jpeg");
    testMimeType("test.mng", "video/x-mng");
    testMimeType("test.png", "image/png");
    testMimeType("test.svg", "image/svg+xml");
    testMimeType("test.xpm", "image/x-xpm");
}
#else
void tst_QXmppUtils::testMime()
{
}
#endif

void tst_QXmppUtils::testLibVersion()
{
    QCOMPARE(QXmppVersion(), QString("0.8.3"));
}

void tst_QXmppUtils::testTimezoneOffset()
{
    // parsing
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("Z"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-00:00"), 0);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("+01:30"), 5400);
    QCOMPARE(QXmppUtils::timezoneOffsetFromString("-01:30"), -5400);

    // serialization
    QCOMPARE(QXmppUtils::timezoneOffsetToString(0), QLatin1String("Z"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(5400), QLatin1String("+01:30"));
    QCOMPARE(QXmppUtils::timezoneOffsetToString(-5400), QLatin1String("-01:30"));
}

QTEST_MAIN(tst_QXmppUtils)
#include "tst_qxmpputils.moc"
