// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTrustMessageElement.h"
#include "QXmppTrustMessageKeyOwner.h"

#include "util.h"

#include <QObject>

class tst_QXmppTrustMessages : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testIsTrustMessageKeyOwner_data();
    Q_SLOT void testIsTrustMessageKeyOwner();
    Q_SLOT void testTrustMessageKeyOwner_data();
    Q_SLOT void testTrustMessageKeyOwner();
    Q_SLOT void testIsTrustMessageElement_data();
    Q_SLOT void testIsTrustMessageElement();
    Q_SLOT void testTrustMessageElement();
};

void tst_QXmppTrustMessages::testIsTrustMessageKeyOwner_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<key-owner xmlns=\"urn:xmpp:tm:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:tm:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<key-owner xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppTrustMessages::testIsTrustMessageKeyOwner()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppTrustMessageKeyOwner::isTrustMessageKeyOwner(xmlToDom(xml)), isValid);
}

void tst_QXmppTrustMessages::testTrustMessageKeyOwner_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QString>("keyOwnerJid");
    QTest::addColumn<QList<QByteArray>>("trustedKeys");
    QTest::addColumn<QList<QByteArray>>("distrustedKeys");

    QTest::newRow("trustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"alice@example.org\">"
               "<trust>aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=</trust>"
               "<trust>IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=</trust>"
               "</key-owner>")
        << u"alice@example.org"_s
        << QList<QByteArray>({ QByteArray::fromBase64(QByteArrayLiteral("aFABnX7Q/rbTgjBySYzrT2FsYCVYb49mbca5yB734KQ=")),
                               QByteArray::fromBase64(QByteArrayLiteral("IhpPjiKLchgrAG5cpSfTvdzPjZ5v6vTOluHEUehkgCA=")) })
        << QList<QByteArray>();
    QTest::newRow("distrustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"bob@example.com\">"
               "<distrust>tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=</distrust>"
               "<distrust>2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=</distrust>"
               "</key-owner>")
        << u"bob@example.com"_s
        << QList<QByteArray>()
        << QList<QByteArray>({ QByteArray::fromBase64(QByteArrayLiteral("tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=")),
                               QByteArray::fromBase64(QByteArrayLiteral("2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=")) });
    QTest::newRow("trustedAndDistrustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"bob@example.com\">"
               "<trust>YjVI04NcbTPvXLaA95RO84HPcSvyOgEZ2r5cTyUs0C8=</trust>"
               "<distrust>tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=</distrust>"
               "<distrust>2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=</distrust>"
               "</key-owner>")
        << u"bob@example.com"_s
        << QList<QByteArray>({ QByteArray::fromBase64(QByteArrayLiteral("YjVI04NcbTPvXLaA95RO84HPcSvyOgEZ2r5cTyUs0C8=")) })
        << QList<QByteArray>({ QByteArray::fromBase64(QByteArrayLiteral("tCP1CI3pqSTVGzFYFyPYUMfMZ9Ck/msmfD0wH/VtJBM=")),
                               QByteArray::fromBase64(QByteArrayLiteral("2fhJtrgoMJxfLI3084/YkYh9paqiSiLFDVL2m0qAgX4=")) });
}

void tst_QXmppTrustMessages::testTrustMessageKeyOwner()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, keyOwnerJid);
    QFETCH(QList<QByteArray>, trustedKeys);
    QFETCH(QList<QByteArray>, distrustedKeys);

    QXmppTrustMessageKeyOwner keyOwner1;
    parsePacket(keyOwner1, xml);
    QCOMPARE(keyOwner1.jid(), keyOwnerJid);
    QCOMPARE(keyOwner1.trustedKeys(), trustedKeys);
    QCOMPARE(keyOwner1.distrustedKeys(), distrustedKeys);
    serializePacket(keyOwner1, xml);

    QXmppTrustMessageKeyOwner keyOwner2;
    keyOwner2.setJid(keyOwnerJid);
    keyOwner2.setTrustedKeys(trustedKeys);
    keyOwner2.setDistrustedKeys(distrustedKeys);
    QCOMPARE(keyOwner2.jid(), keyOwnerJid);
    QCOMPARE(keyOwner2.trustedKeys(), trustedKeys);
    QCOMPARE(keyOwner2.distrustedKeys(), distrustedKeys);
    serializePacket(keyOwner2, xml);
}

void tst_QXmppTrustMessages::testIsTrustMessageElement_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<trust-message xmlns=\"urn:xmpp:tm:1\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:tm:1\"/>")
        << false;
    QTest::newRow("invalidNamespace")
        << QByteArrayLiteral("<trust-message xmlns=\"invalid\"/>")
        << false;
}

void tst_QXmppTrustMessages::testIsTrustMessageElement()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, isValid);

    QCOMPARE(QXmppTrustMessageElement::isTrustMessageElement(xmlToDom(xml)), isValid);
}

void tst_QXmppTrustMessages::testTrustMessageElement()
{
    const QByteArray xml(QByteArrayLiteral(
        "<trust-message xmlns=\"urn:xmpp:tm:1\" usage=\"urn:xmpp:atm:1\" encryption=\"urn:xmpp:omemo:2\">"
        "<key-owner jid=\"alice@example.org\"/>"
        "<key-owner jid=\"bob@example.com\"/>"
        "</trust-message>"));

    QXmppTrustMessageElement trustMessageElement1;
    parsePacket(trustMessageElement1, xml);
    QCOMPARE(trustMessageElement1.usage(), u"urn:xmpp:atm:1"_s);
    QCOMPARE(trustMessageElement1.encryption(), u"urn:xmpp:omemo:2"_s);
    QCOMPARE(trustMessageElement1.keyOwners().at(0).jid(), u"alice@example.org"_s);
    QCOMPARE(trustMessageElement1.keyOwners().at(1).jid(), u"bob@example.com"_s);
    serializePacket(trustMessageElement1, xml);

    QXmppTrustMessageKeyOwner keyOwner1;
    keyOwner1.setJid(u"alice@example.org"_s);
    QXmppTrustMessageKeyOwner keyOwner2;
    keyOwner2.setJid(u"bob@example.com"_s);

    QXmppTrustMessageElement trustMessageElement2;
    trustMessageElement2.setUsage(u"urn:xmpp:atm:1"_s);
    trustMessageElement2.setEncryption(u"urn:xmpp:omemo:2"_s);
    trustMessageElement2.setKeyOwners({ keyOwner1, keyOwner2 });
    QCOMPARE(trustMessageElement2.usage(), u"urn:xmpp:atm:1"_s);
    QCOMPARE(trustMessageElement2.encryption(), u"urn:xmpp:omemo:2"_s);
    QCOMPARE(trustMessageElement2.keyOwners().at(0).jid(), u"alice@example.org"_s);
    QCOMPARE(trustMessageElement2.keyOwners().at(1).jid(), u"bob@example.com"_s);
    serializePacket(trustMessageElement2, xml);

    QXmppTrustMessageElement trustMessageElement3;
    trustMessageElement3.setUsage(u"urn:xmpp:atm:1"_s);
    trustMessageElement3.setEncryption(u"urn:xmpp:omemo:2"_s);
    trustMessageElement3.addKeyOwner(keyOwner1);
    trustMessageElement3.addKeyOwner(keyOwner2);
    QCOMPARE(trustMessageElement3.usage(), u"urn:xmpp:atm:1"_s);
    QCOMPARE(trustMessageElement3.encryption(), u"urn:xmpp:omemo:2"_s);
    QCOMPARE(trustMessageElement3.keyOwners().at(0).jid(), u"alice@example.org"_s);
    QCOMPARE(trustMessageElement3.keyOwners().at(1).jid(), u"bob@example.com"_s);
    serializePacket(trustMessageElement3, xml);
}

QTEST_MAIN(tst_QXmppTrustMessages)
#include "tst_qxmpptrustmessages.moc"
