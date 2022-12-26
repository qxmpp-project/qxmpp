// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStreamFeatures.h"

#include "util.h"

template<class T>
static void parsePacketWithStream(T &packet, const QByteArray &xml)
{
    QDomDocument doc;
    const auto wrappedXml =
        QByteArrayLiteral("<stream:stream xmlns:stream='http://etherx.jabber.org/streams'>") +
        xml + QByteArrayLiteral("</stream:stream>");

    QString err;
    bool parsingSuccess = doc.setContent(wrappedXml, true, &err);
    if (!err.isNull()) {
        qDebug() << err;
    }
    QVERIFY(parsingSuccess);

    packet.parse(doc.documentElement().firstChildElement());
}

class tst_QXmppStreamFeatures : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testEmpty();
    Q_SLOT void testRequired();
    Q_SLOT void testFull();
    Q_SLOT void testSetters();
};

void tst_QXmppStreamFeatures::testEmpty()
{
    const QByteArray xml("<stream:features/>");

    QXmppStreamFeatures features;
    parsePacketWithStream(features, xml);
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.clientStateIndicationMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.registerMode(), QXmppStreamFeatures::Disabled);
    QCOMPARE(features.preApprovedSubscriptionsSupported(), false);
    QCOMPARE(features.rosterVersioningSupported(), false);
    QCOMPARE(features.authMechanisms(), QStringList());
    QCOMPARE(features.compressionMethods(), QStringList());
    serializePacket(features, xml);
}

void tst_QXmppStreamFeatures::testRequired()
{
    const QByteArray xml(
        "<stream:features>"
        "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\">"
        "<required/>"
        "</starttls>"
        "</stream:features>");

    QXmppStreamFeatures features;
    parsePacketWithStream(features, xml);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Required);
    serializePacket(features, xml);
}

void tst_QXmppStreamFeatures::testFull()
{
    const QByteArray xml("<stream:features>"
                         "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
                         "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
                         "<auth xmlns=\"http://jabber.org/features/iq-auth\"/>"
                         "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>"
                         "<csi xmlns=\"urn:xmpp:csi:0\"/>"
                         "<register xmlns=\"http://jabber.org/features/iq-register\"/>"
                         "<sub xmlns=\"urn:xmpp:features:pre-approval\"/>"
                         "<ver xmlns=\"urn:xmpp:features:rosterver\"/>"
                         "<compression xmlns=\"http://jabber.org/features/compress\"><method>zlib</method></compression>"
                         "<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"><mechanism>PLAIN</mechanism></mechanisms>"
                         "</stream:features>");

    QXmppStreamFeatures features;
    parsePacketWithStream(features, xml);
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.clientStateIndicationMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.registerMode(), QXmppStreamFeatures::Enabled);
    QCOMPARE(features.preApprovedSubscriptionsSupported(), true);
    QCOMPARE(features.authMechanisms(), QStringList() << "PLAIN");
    QCOMPARE(features.compressionMethods(), QStringList() << "zlib");
    serializePacket(features, xml);

    features = QXmppStreamFeatures();
    features.setBindMode(QXmppStreamFeatures::Enabled);
    features.setSessionMode(QXmppStreamFeatures::Enabled);
    features.setNonSaslAuthMode(QXmppStreamFeatures::Enabled);
    features.setTlsMode(QXmppStreamFeatures::Enabled);
    features.setClientStateIndicationMode(QXmppStreamFeatures::Enabled);
    features.setRegisterMode(QXmppStreamFeatures::Enabled);
    features.setPreApprovedSubscriptionsSupported(true);
    features.setRosterVersioningSupported(true);
    features.setAuthMechanisms(QStringList { QStringLiteral("PLAIN") });
    features.setCompressionMethods(QStringList { QStringLiteral("zlib") });
    serializePacket(features, xml);
}

void tst_QXmppStreamFeatures::testSetters()
{
    QXmppStreamFeatures features;
    features.setBindMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.bindMode(), QXmppStreamFeatures::Enabled);
    features.setSessionMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.sessionMode(), QXmppStreamFeatures::Enabled);
    features.setNonSaslAuthMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.nonSaslAuthMode(), QXmppStreamFeatures::Enabled);
    features.setTlsMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.tlsMode(), QXmppStreamFeatures::Enabled);
    features.setClientStateIndicationMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.clientStateIndicationMode(), QXmppStreamFeatures::Enabled);
    features.setClientStateIndicationMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.clientStateIndicationMode(), QXmppStreamFeatures::Enabled);
    features.setRegisterMode(QXmppStreamFeatures::Enabled);
    QCOMPARE(features.registerMode(), QXmppStreamFeatures::Enabled);

    features.setAuthMechanisms(QStringList() << "custom-mechanism");
    QCOMPARE(features.authMechanisms(), QStringList() << "custom-mechanism");
    features.setCompressionMethods(QStringList() << "compression-methods");
    QCOMPARE(features.compressionMethods(), QStringList() << "compression-methods");
}

QTEST_MAIN(tst_QXmppStreamFeatures)
#include "tst_qxmppstreamfeatures.moc"
