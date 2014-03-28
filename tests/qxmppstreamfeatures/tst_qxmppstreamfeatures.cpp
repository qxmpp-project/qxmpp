/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
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

#include "QXmppStreamFeatures.h"
#include "util.h"

class tst_QXmppStreamFeatures : public QObject
{
    Q_OBJECT

private slots:
    void testEmpty();
    void testFull();
};

void tst_QXmppStreamFeatures::testEmpty()
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
}

void tst_QXmppStreamFeatures::testFull()
{
    const QByteArray xml("<stream:features>"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "<auth xmlns=\"http://jabber.org/features/iq-auth\"/>"
        "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>"
        "<compression xmlns=\"http://jabber.org/features/compress\"><method>zlib</method></compression>"
        "<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><mechanism>PLAIN</mechanism></mechanisms>"
        "</stream:features>");

    QXmppStreamFeatures features;
    parsePacket(features, xml);
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.authMechanisms(), QStringList() << "PLAIN");
    QCOMPARE(features.compressionMethods(), QStringList() << "zlib");
    serializePacket(features, xml);
}

QTEST_MAIN(tst_QXmppStreamFeatures)
#include "tst_qxmppstreamfeatures.moc"
