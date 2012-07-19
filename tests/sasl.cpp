/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include "QXmppSaslAuth.h"

#include "sasl.h"
#include "tests.h"

void tst_QXmppSaslClient::testAnonymous()
{
    QXmppSaslClientAnonymous client;
    QCOMPARE(client.mechanism(), QLatin1String("ANONYMOUS"));

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client.respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());
 
    // any further step is an error
    QVERIFY(!client.respond(QByteArray(), response));
}

void tst_QXmppSaslClient::testDigestMd5()
{
    qsrand(0);
    QXmppSaslClientDigestMd5 client;
    client.setUsername("qxmpp1");
    client.setPassword("qxmpp123");
    client.setServer("jabber.ru");
    QCOMPARE(client.mechanism(), QLatin1String("DIGEST-MD5"));

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client.respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());

    QVERIFY(client.respond(QByteArray("nonce=\"2530347127\",qop=\"auth\",charset=utf-8,algorithm=md5-sess"), response));
    QCOMPARE(response, QByteArray("charset=utf-8,cnonce=\"AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=\",digest-uri=\"xmpp/jabber.ru\",nc=00000001,nonce=2530347127,output=a61fbf4320577d74038b71a8546bc7ae,qop=auth,username=qxmpp1"));

    QVERIFY(client.respond(QByteArray("rspauth=d92bf7f4331700c24799cbab364a14b7"), response));
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QVERIFY(!client.respond(QByteArray(), response));
}

void tst_QXmppSaslClient::testFacebook()
{
    QXmppSaslClientFacebook client;
    client.setUsername("123456789012345");
    client.setPassword("abcdefghijlkmno");
    QCOMPARE(client.mechanism(), QLatin1String("X-FACEBOOK-PLATFORM"));

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client.respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());
   
    // challenge response 
    QVERIFY(client.respond(QByteArray("version=1&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8"), response));
    QCOMPARE(response, QByteArray("access_token=123456789012345&api_key=abcdefghijlkmno&call_id=&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8&v=1.0"));

    // any further step is an error
    QVERIFY(!client.respond(QByteArray(), response));
}

void tst_QXmppSaslClient::testPlain()
{
    QXmppSaslClientPlain client;
    client.setUsername("foo");
    client.setPassword("bar");
    QCOMPARE(client.mechanism(), QLatin1String("PLAIN"));

    // initial step returns data
    QByteArray response;
    QVERIFY(client.respond(QByteArray(), response));
    QCOMPARE(response, QByteArray("\0foo\0bar", 8));

    // any further step is an error
    QVERIFY(!client.respond(QByteArray(), response));
}


