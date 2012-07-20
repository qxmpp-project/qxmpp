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
#include "QXmppSaslAuth_p.h"

#include "sasl.h"
#include "tests.h"

void tst_QXmppSasl::testParsing()
{
    // empty
    QMap<QByteArray, QByteArray> empty = QXmppSaslDigestMd5::parseMessage(QByteArray());
    QCOMPARE(empty.size(), 0);
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(empty), QByteArray());

    // non-empty
    const QByteArray bytes("number=12345,quoted_plain=\"quoted string\",quoted_quote=\"quoted\\\\slash\\\"quote\",string=string");

    QMap<QByteArray, QByteArray> map = QXmppSaslDigestMd5::parseMessage(bytes);
    QCOMPARE(map.size(), 4);
    QCOMPARE(map["number"], QByteArray("12345"));
    QCOMPARE(map["quoted_plain"], QByteArray("quoted string"));
    QCOMPARE(map["quoted_quote"], QByteArray("quoted\\slash\"quote"));
    QCOMPARE(map["string"], QByteArray("string"));
    QCOMPARE(QXmppSaslDigestMd5::serializeMessage(map), bytes);
}

void tst_QXmppSaslClient::testAvailableMechanisms()
{
    QCOMPARE(QXmppSaslClient::availableMechanisms(), QStringList() << "PLAIN" << "DIGEST-MD5" << "ANONYMOUS" << "X-FACEBOOK-PLATFORM");
}

void tst_QXmppSaslClient::testBadMechanism()
{
    QXmppSaslClient *client = QXmppSaslClient::create("BAD-MECH");
    QVERIFY(client == 0);
}

void tst_QXmppSaslClient::testAnonymous()
{
    QXmppSaslClient *client = QXmppSaslClient::create("ANONYMOUS");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism(), QLatin1String("ANONYMOUS"));

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client->respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray(), response));

    delete client;
}

void tst_QXmppSaslClient::testDigestMd5_data()
{
    QTest::addColumn<QByteArray>("qop");
    QTest::newRow("qop-none") << QByteArray();
    QTest::newRow("qop-auth") << QByteArray(",qop=\"auth\"");
    QTest::newRow("qop-multi") << QByteArray(",qop=\"auth,auth-int\"");
}

void tst_QXmppSaslClient::testDigestMd5()
{
    QFETCH(QByteArray, qop);

    QXmppSaslDigestMd5::setNonce("AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=");

    QXmppSaslClient *client = QXmppSaslClient::create("DIGEST-MD5");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism(), QLatin1String("DIGEST-MD5"));

    client->setUsername("qxmpp1");
    client->setPassword("qxmpp123");
    client->setHost("jabber.ru");
    client->setServiceType("xmpp");

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client->respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());

    QVERIFY(client->respond(QByteArray("nonce=\"2530347127\"") + qop + QByteArray("charset=utf-8,algorithm=md5-sess"), response));
    QCOMPARE(response, QByteArray("charset=utf-8,cnonce=\"AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=\",digest-uri=\"xmpp/jabber.ru\",nc=00000001,nonce=2530347127,qop=auth,response=a61fbf4320577d74038b71a8546bc7ae,username=qxmpp1"));

    QVERIFY(client->respond(QByteArray("rspauth=d92bf7f4331700c24799cbab364a14b7"), response));
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QVERIFY(!client->respond(QByteArray(), response));

    delete client;
}

void tst_QXmppSaslClient::testFacebook()
{
    QXmppSaslClient *client = QXmppSaslClient::create("X-FACEBOOK-PLATFORM");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism(), QLatin1String("X-FACEBOOK-PLATFORM"));

    client->setUsername("123456789012345");
    client->setPassword("abcdefghijlkmno");

    // initial step returns nothing
    QByteArray response;
    QVERIFY(client->respond(QByteArray(), response));
    QCOMPARE(response, QByteArray());
   
    // challenge response 
    QVERIFY(client->respond(QByteArray("version=1&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8"), response));
    QCOMPARE(response, QByteArray("access_token=123456789012345&api_key=abcdefghijlkmno&call_id=&method=auth.xmpp_login&nonce=AA4EFEE16F2AB64B131EEFFE6EACDDB8&v=1.0"));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray(), response));

    delete client;
}

void tst_QXmppSaslClient::testPlain()
{
    QXmppSaslClient *client = QXmppSaslClient::create("PLAIN");
    QVERIFY(client != 0);
    QCOMPARE(client->mechanism(), QLatin1String("PLAIN"));

    client->setUsername("foo");
    client->setPassword("bar");

    // initial step returns data
    QByteArray response;
    QVERIFY(client->respond(QByteArray(), response));
    QCOMPARE(response, QByteArray("\0foo\0bar", 8));

    // any further step is an error
    QVERIFY(!client->respond(QByteArray(), response));

    delete client;
}

void tst_QXmppSaslServer::testBadMechanism()
{
    QXmppSaslServer *server = QXmppSaslServer::create("BAD-MECH");
    QVERIFY(server == 0);
}

void tst_QXmppSaslServer::testAnonymous()
{
    QXmppSaslServer *server = QXmppSaslServer::create("ANONYMOUS");
    QVERIFY(server != 0);
    QCOMPARE(server->mechanism(), QLatin1String("ANONYMOUS"));

    // initial step returns success
    QByteArray response;
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Succeeded);
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);

    delete server;
}

void tst_QXmppSaslServer::testDigestMd5()
{
    QXmppSaslDigestMd5::setNonce("OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=");

    QXmppSaslServer *server = QXmppSaslServer::create("DIGEST-MD5");
    QVERIFY(server != 0);
    QCOMPARE(server->mechanism(), QLatin1String("DIGEST-MD5"));

    // initial step returns challenge
    QByteArray response;
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Challenge);
    QCOMPARE(response, QByteArray("algorithm=md5-sess,charset=utf-8,nonce=\"OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=\",qop=auth"));

    // password needed
    const QByteArray request = QByteArray("charset=utf-8,cnonce=\"AMzVG8Oibf+sVUCPPlWLR8lZQvbbJtJB9vJd+u3c6dw=\",digest-uri=\"xmpp/jabber.ru\",nc=00000001,nonce=\"OI08/m+QRm6Ma+fKOjuqVXtz40sR5u9/u5GN6sSW0rs=\",qop=auth,response=70e9063257ee2bf6bfd108975b917410,username=qxmpp1");
    QCOMPARE(server->respond(request, response), QXmppSaslServer::InputNeeded);
    QCOMPARE(server->username(), QLatin1String("qxmpp1"));
    server->setPassword("qxmpp123");

    // second challenge
    QCOMPARE(server->respond(request, response), QXmppSaslServer::Challenge);
    QCOMPARE(response, QByteArray("rspauth=2821a3add271b9ae02b813bed57ec878"));

    // success
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Succeeded);
    QCOMPARE(response, QByteArray());

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);

    delete server;
}

void tst_QXmppSaslServer::testPlain()
{
    QXmppSaslServer *server = QXmppSaslServer::create("PLAIN");
    QVERIFY(server != 0);
    QCOMPARE(server->mechanism(), QLatin1String("PLAIN"));

    // initial step returns success
    QByteArray response;
    QCOMPARE(server->respond(QByteArray("\0foo\0bar", 8), response), QXmppSaslServer::InputNeeded);
    QCOMPARE(response, QByteArray());
    QCOMPARE(server->username(), QLatin1String("foo"));
    QCOMPARE(server->password(), QLatin1String("bar"));

    // any further step is an error
    QCOMPARE(server->respond(QByteArray(), response), QXmppSaslServer::Failed);

    delete server;
}
