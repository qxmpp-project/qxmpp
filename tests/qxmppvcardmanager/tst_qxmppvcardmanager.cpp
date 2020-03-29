/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include <QObject>
#include "QXmppClient.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"
#include "util.h"

Q_DECLARE_METATYPE(QXmppVCardIq);

class tst_QXmppVCardManager : public QObject
{
    Q_OBJECT

private slots:
    void testHandleStanza_data();
    void testHandleStanza();

private:
    QXmppClient m_client;
};

void tst_QXmppVCardManager::testHandleStanza_data()
{
    QTest::addColumn<QXmppVCardIq>("expectedIq");
    QTest::addColumn<bool>("accept");
    QTest::addColumn<bool>("success");
    QTest::addColumn<bool>("clientVCard");

    QXmppVCardIq iqForVCard;
    iqForVCard.setType(QXmppIq::Result);
    iqForVCard.setFrom("jer@jabber.org");
    iqForVCard.setTo("stpeter@jabber.org/roundabout");
    iqForVCard.setFullName("JeremieMiller");

    QTest::newRow("vCardReceived")
        << iqForVCard
        << true
        << true
        << false;

    QXmppVCardIq iqForNoVCard;
    iqForNoVCard.setType(QXmppIq::Error);
    iqForNoVCard.setTo("stpeter@jabber.org/roundabout");
    iqForNoVCard.setError({QXmppStanza::Error::Cancel, QXmppStanza::Error::ServiceUnavailable});

    QTest::newRow("noVCard")
        << iqForNoVCard
        << true
        << false
        << false;

    QXmppVCardIq iqForClientVCardWithoutFromAttribute;
    iqForClientVCardWithoutFromAttribute.setType(QXmppIq::Result);
    iqForClientVCardWithoutFromAttribute.setTo("stpeter@jabber.org/roundabout");
    iqForClientVCardWithoutFromAttribute.setFullName("Peter Saint-Andre");

    QTest::newRow("clientVCardReceivedWithoutFromAttribute")
        << iqForClientVCardWithoutFromAttribute
        << true
        << true
        << true;

    QXmppVCardIq iqForClientVCardWithFromAttribute;
    iqForClientVCardWithFromAttribute.setType(QXmppIq::Result);
    iqForClientVCardWithFromAttribute.setFrom("stpeter@jabber.org");
    iqForClientVCardWithFromAttribute.setTo("stpeter@jabber.org/roundabout");
    iqForClientVCardWithFromAttribute.setFullName("Peter Saint-Andre");

    QTest::newRow("clientVCardReceivedWithFromAttribute")
        << iqForClientVCardWithFromAttribute
        << true
        << true
        << true;

    QXmppVCardIq iqForNoClientVCard;
    iqForNoClientVCard.setType(QXmppIq::Error);
    iqForNoClientVCard.setTo("stpeter@jabber.org/roundabout");
    iqForNoClientVCard.setError({QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound});

    QTest::newRow("noClientVCard")
        << iqForNoClientVCard
        << true
        << false
        << true;

    QXmppVCardIq iqForEmptyClientVCard;
    iqForEmptyClientVCard.setType(QXmppIq::Result);
    iqForEmptyClientVCard.setTo("stpeter@jabber.org/roundabout");

    QTest::newRow("emptyClientVCard")
        << iqForEmptyClientVCard
        << true
        << true
        << true;
}

void tst_QXmppVCardManager::testHandleStanza()
{
    QXmppVCardManager *manager = new QXmppVCardManager();
    m_client.addExtension(manager);

    QFETCH(QXmppVCardIq, expectedIq);
    QFETCH(bool, accept);
    QFETCH(bool, success);
    QFETCH(bool, clientVCard);

    bool signalVCardReceivedTriggered = false;
    bool signalClientVCardReceivedTriggered = false;
    bool signalNoVCardFoundTriggered = false;
    bool signalNoClientVCardFoundTriggered = false;

    QObject context;
    connect(manager, &QXmppVCardManager::vCardReceived, &context, [&](QXmppVCardIq iq) {
        signalVCardReceivedTriggered = true;
        QCOMPARE(iq, expectedIq);
    });
    connect(manager, &QXmppVCardManager::clientVCardReceived, &context, [&]{
        signalClientVCardReceivedTriggered = true;
    });
    connect(manager, &QXmppVCardManager::noVCardFound, &context, [&]{
        signalNoVCardFoundTriggered = true;
    });
    connect(manager, &QXmppVCardManager::noClientVCardFound, &context, [&]{
        signalNoClientVCardFoundTriggered = true;
    });

    m_client.connectToServer("stpeter@jabber.org", {});
    m_client.disconnectFromServer();

    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    expectedIq.toXml(&xmlStream);

    QDomDocument doc;
    QCOMPARE(doc.setContent(data, true), true);
    QDomElement domElement = doc.documentElement();
    bool accepted = manager->handleStanza(domElement);

    QCOMPARE(accepted, accept);

    QCOMPARE(manager->clientVCard(), accept && success && clientVCard ? expectedIq : QXmppVCardIq());
    QCOMPARE(manager->isClientVCardReceived(), accept && success && clientVCard);

    QCOMPARE(signalVCardReceivedTriggered, accept && success && !clientVCard);
    QCOMPARE(signalClientVCardReceivedTriggered, accept && success && clientVCard);
    QCOMPARE(signalNoVCardFoundTriggered, accept && !success && !clientVCard);
    QCOMPARE(signalNoClientVCardFoundTriggered, accept && !success && clientVCard);

    // Reset the manager because the next one does not have a set client VCard.
    m_client.removeExtension(manager);
}

QTEST_MAIN(tst_QXmppVCardManager)
#include "tst_qxmppvcardmanager.moc"
