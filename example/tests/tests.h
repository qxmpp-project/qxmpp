/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void testHmac();
};

class TestPackets : public QObject
{
    Q_OBJECT

private slots:
    void testBindNoResource();
    void testBindResource();
    void testBindResult();
    void testMessage();
    void testMessageFull();
    void testMessageDelay();
    void testMessageLegacyDelay();
    void testPresence();
    void testPresenceFull();
    void testSession();
};

class TestJingle : public QObject
{
    Q_OBJECT

private slots:
    void testSession();
    void testTerminate();
    void testPayloadType();
    void testRinging();
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
