/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
 *  Melvin Keskin
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

#include "QXmppTrustMessageElement.h"
#include "QXmppTrustMessageKeyOwner.h"

#include "util.h"
#include <QObject>

class tst_QXmppTrustMessages : public QObject
{
    Q_OBJECT

private slots:
    void testIsTrustMessageKeyOwner_data();
    void testIsTrustMessageKeyOwner();
    void testTrustMessageKeyOwner_data();
    void testTrustMessageKeyOwner();
};

void tst_QXmppTrustMessages::testIsTrustMessageKeyOwner_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("valid")
        << QByteArrayLiteral("<key-owner xmlns=\"urn:xmpp:tm:0\"/>")
        << true;
    QTest::newRow("invalidTag")
        << QByteArrayLiteral("<invalid xmlns=\"urn:xmpp:tm:0\"/>")
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
    QTest::addColumn<QList<QString>>("trustedKeys");
    QTest::addColumn<QList<QString>>("distrustedKeys");

    QTest::newRow("trustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"alice@example.org\">"
               "<trust>6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4</trust>"
               "<trust>221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020</trust>"
               "</key-owner>")
        << QStringLiteral("alice@example.org")
        << QList<QString>({ QStringLiteral("6850019d7ed0feb6d3823072498ceb4f616c6025586f8f666dc6b9c81ef7e0a4"),
                            QStringLiteral("221a4f8e228b72182b006e5ca527d3bddccf8d9e6feaf4ce96e1c451e8648020") })
        << QList<QString>();
    QTest::newRow("distrustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"bob@example.com\">"
               "<distrust>b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413</distrust>"
               "<distrust>d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e</distrust>"
               "</key-owner>")
        << QStringLiteral("bob@example.com")
        << QList<QString>()
        << QList<QString>({ QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
                            QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") });
    QTest::newRow("trustedAndDistrustedKeys")
        << QByteArrayLiteral(
               "<key-owner jid=\"bob@example.com\">"
               "<trust>623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f</trust>"
               "<distrust>b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413</distrust>"
               "<distrust>d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e</distrust>"
               "</key-owner>")
        << QStringLiteral("bob@example.com")
        << QList<QString>({ QStringLiteral("623548d3835c6d33ef5cb680f7944ef381cf712bf23a0119dabe5c4f252cd02f") })
        << QList<QString>({ QStringLiteral("b423f5088de9a924d51b31581723d850c7cc67d0a4fe6b267c3d301ff56d2413"),
                            QStringLiteral("d9f849b6b828309c5f2c8df4f38fd891887da5aaa24a22c50d52f69b4a80817e") });
}

void tst_QXmppTrustMessages::testTrustMessageKeyOwner()
{
    QFETCH(QByteArray, xml);
    QFETCH(QString, keyOwnerJid);
    QFETCH(QList<QString>, trustedKeys);
    QFETCH(QList<QString>, distrustedKeys);

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

QTEST_MAIN(tst_QXmppTrustMessages)
#include "tst_qxmpptrustmessages.moc"
