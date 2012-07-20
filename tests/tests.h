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

#include <QDomDocument>
#include <QObject>
#include <QtTest/QtTest>

template <class T>
static void parsePacket(T &packet, const QByteArray &xml)
{
    //qDebug() << "parsing" << xml;
    QDomDocument doc;
    QCOMPARE(doc.setContent(xml, true), true);
    QDomElement element = doc.documentElement();
    packet.parse(element);
}

template <class T>
static void serializePacket(T &packet, const QByteArray &xml)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QXmlStreamWriter writer(&buffer);
    packet.toXml(&writer);
    qDebug() << "expect " << xml;
    qDebug() << "writing" << buffer.data();
    QCOMPARE(buffer.data(), xml);
}

class TestUtils : public QObject
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

class TestPackets : public QObject
{
    Q_OBJECT

private slots:
    void testArchiveList_data();
    void testArchiveList();
    void testArchiveChat_data();
    void testArchiveChat();
    void testArchiveRemove();
    void testArchiveRetrieve_data();
    void testArchiveRetrieve();
    void testBindNoResource();
    void testBindResource();
    void testBindResult();
    void testDiscovery();
    void testDiscoveryWithForm();
    void testNonSaslAuth();
    void testSession();
    void testStreamFeatures();
    void testVCard();
    void testVersionGet();
    void testVersionResult();
    void testEntityTimeGet();
    void testEntityTimeResult();
};

class TestJingle : public QObject
{
    Q_OBJECT

private slots:
    void testSession();
    void testTerminate();
    void testAudioPayloadType();
    void testVideoPayloadType();
    void testRinging();
};

class TestPubSub : public QObject
{
    Q_OBJECT

private slots:
    void testItems();
    void testItemsResponse();
    void testPublish();
    void testSubscribe();
    void testSubscription();
    void testSubscriptions();
};

class TestServer : public QObject
{
    Q_OBJECT

private slots:
    void testConnect_data();
    void testConnect();
};

class TestStun : public QObject
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

class TestXmlRpc : public QObject
{
    Q_OBJECT

private slots:
    void testBase64();
    void testBool();
    void testDateTime();
    void testDouble();
    void testInt();
    void testNil();
    void testString();

    void testArray();
    void testStruct();

    void testInvoke();
    void testResponse();
    void testResponseFault();
};
