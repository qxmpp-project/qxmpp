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

#include "QXmppSessionIq.h"
#include "QXmppStreamFeatures.h"
#include "util.h"

class TestPackets : public QObject
{
    Q_OBJECT

private slots:
    void testSession();
    void testStreamFeatures();
};

void TestPackets::testSession()
{
    const QByteArray xml(
        "<iq id=\"session_1\" to=\"example.com\" type=\"set\">"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "</iq>");

    QXmppSessionIq session;
    parsePacket(session, xml);
    QCOMPARE(session.id(), QString("session_1"));
    QCOMPARE(session.to(), QString("example.com"));
    QCOMPARE(session.type(), QXmppIq::Set);
    serializePacket(session, xml);
}

void TestPackets::testStreamFeatures()
{
    const QByteArray xml("<stream:features/>");
    QXmppStreamFeatures features;
    parsePacket(features, xml);
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.authMechanisms(), QStringList());
    QCOMPARE(features.compressionMethods(), QStringList());
    serializePacket(features, xml);

    const QByteArray xml2("<stream:features>"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "<auth xmlns=\"http://jabber.org/features/iq-auth\"/>"
        "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>"
        "<compression xmlns=\"http://jabber.org/features/compress\"><method>zlib</method></compression>"
        "<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><mechanism>PLAIN</mechanism></mechanisms>"
        "</stream:features>");
    QXmppStreamFeatures features2;
    parsePacket(features2, xml2);
    QCOMPARE(features2.bindMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.sessionMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.nonSaslAuthMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.tlsMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features2.authMechanisms(), QStringList() << "PLAIN");
    QCOMPARE(features2.compressionMethods(), QStringList() << "zlib");
    serializePacket(features2, xml2);
}

QTEST_MAIN(TestPackets)
#include "tests.moc"
