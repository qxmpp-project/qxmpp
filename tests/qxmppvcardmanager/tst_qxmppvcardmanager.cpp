// SPDX-FileCopyrightText: 2020 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

#include "IntegrationTesting.h"
#include "TestClient.h"
#include "util.h"

#include <memory>

#include <QObject>

Q_DECLARE_METATYPE(QXmppVCardIq);

using namespace QXmpp;

class tst_QXmppVCardManager : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testHandleStanza_data();
    Q_SLOT void testHandleStanza();
    Q_SLOT void fetchVCard();
    Q_SLOT void setVCard();

    // integration tests
    Q_SLOT void testSetClientVCard();

    QXmppClient m_client;
};

void tst_QXmppVCardManager::testHandleStanza_data()
{
    QTest::addColumn<QXmppVCardIq>("expectedIq");
    QTest::addColumn<bool>("isClientVCard");

#define ROW(name, iq, clientVCard) \
    QTest::newRow(QT_STRINGIFY(name)) << iq << clientVCard

    QXmppVCardIq iq;
    iq.setType(QXmppIq::Result);
    iq.setTo("stpeter@jabber.org/roundabout");
    iq.setFullName("Jeremie Miller");

    auto iqFromBare = iq;
    iqFromBare.setFrom("stpeter@jabber.org");

    auto iqFromFull = iq;
    iqFromFull.setFrom("stpeter@jabber.org/roundabout");

    ROW(client - vcard - from - empty, iq, true);
    ROW(client - vcard - from - bare, iqFromBare, true);
    ROW(client - vcard - from - full, iqFromFull, false);

#undef ROW
}

void tst_QXmppVCardManager::testHandleStanza()
{
    QFETCH(QXmppVCardIq, expectedIq);
    QFETCH(bool, isClientVCard);

    // initialize new manager to clear internal values
    QXmppVCardManager *manager = new QXmppVCardManager();
    m_client.addExtension(manager);

    // sets own jid internally
    m_client.connectToServer("stpeter@jabber.org", {});
    m_client.disconnectFromServer();

    bool vCardReceived = false;
    bool clientVCardReceived = false;

    QObject context;
    connect(manager, &QXmppVCardManager::vCardReceived, &context, [&](QXmppVCardIq iq) {
        vCardReceived = true;
        QCOMPARE(iq, expectedIq);
    });
    connect(manager, &QXmppVCardManager::clientVCardReceived, &context, [&]() {
        clientVCardReceived = true;
        QCOMPARE(manager->clientVCard(), expectedIq);
    });

    bool accepted = manager->handleStanza(writePacketToDom(expectedIq));

    QVERIFY(accepted);
    QVERIFY(vCardReceived);
    QCOMPARE(clientVCardReceived, isClientVCard);

    // clean up (client deletes manager)
    m_client.removeExtension(manager);
}

void tst_QXmppVCardManager::fetchVCard()
{
    TestClient test;
    auto *manager = test.addNewExtension<QXmppVCardManager>();
    auto task = manager->fetchVCard("stpeter@jabber.org");
    QVERIFY(!task.isFinished());

    test.expect("<iq id='qxmpp2' to='stpeter@jabber.org' type='get'><vCard xmlns='vcard-temp'><TITLE/><ROLE/></vCard></iq>");
    test.inject("<iq id='qxmpp2' type='result'>"
                "<vCard xmlns='vcard-temp'>"
                "<FN>Peter Saint-Andre</FN>"
                "<N>"
                "<FAMILY>Saint-Andre</FAMILY>"
                "<GIVEN>Peter</GIVEN>"
                "<MIDDLE/>"
                "</N>"
                "<NICKNAME>stpeter</NICKNAME>"
                "<URL>http://www.xmpp.org/xsf/people/stpeter.shtml</URL>"
                "<BDAY>1966-08-06</BDAY>"
                "<ORG>"
                "<ORGNAME>XMPP Standards Foundation</ORGNAME>"
                "<ORGUNIT/>"
                "</ORG>"
                "<TITLE>Executive Director</TITLE>"
                "<ROLE>Patron Saint</ROLE>"
                "<TEL><WORK/><VOICE/><NUMBER>303-308-3282</NUMBER></TEL>"
                "<TEL><WORK/><FAX/><NUMBER/></TEL>"
                "<TEL><WORK/><MSG/><NUMBER/></TEL>"
                "<ADR>"
                "<WORK/>"
                "<EXTADD>Suite 600</EXTADD>"
                "<STREET>1899 Wynkoop Street</STREET>"
                "<LOCALITY>Denver</LOCALITY>"
                "<REGION>CO</REGION>"
                "<PCODE>80202</PCODE>"
                "<CTRY>USA</CTRY>"
                "</ADR>"
                "<TEL><HOME/><VOICE/><NUMBER>303-555-1212</NUMBER></TEL>"
                "<TEL><HOME/><FAX/><NUMBER/></TEL>"
                "<TEL><HOME/><MSG/><NUMBER/></TEL>"
                "<ADR>"
                "<HOME/>"
                "<EXTADD/>"
                "<STREET/>"
                "<LOCALITY>Denver</LOCALITY>"
                "<REGION>CO</REGION>"
                "<PCODE>80209</PCODE>"
                "<CTRY>USA</CTRY>"
                "</ADR>"
                "<EMAIL><INTERNET/><PREF/><USERID>stpeter@jabber.org</USERID></EMAIL>"
                "<JABBERID>stpeter@jabber.org</JABBERID>"
                "<DESC>More information about me is located on my personal website: http://www.saint-andre.com/</DESC>"
                "</vCard>"
                "</iq>");

    auto vCardIq = expectFutureVariant<QXmppVCardIq>(task);
    QCOMPARE(vCardIq.birthday(), QDate(1966, 8, 6));
}

void tst_QXmppVCardManager::setVCard()
{
    TestClient test;
    test.configuration().setJid("stpeter@jabber.org");
    auto *manager = test.addNewExtension<QXmppVCardManager>();

    QXmppVCardIq v;
    v.setFirstName("Peter");
    v.setLastName("Saint-Andre");
    v.setFullName("Peter Saint-Andre");

    auto task = manager->setVCard(v);
    QVERIFY(!task.isFinished());
    test.expect("<iq id='qxmpp2' to='stpeter@jabber.org' type='set'>"
                "<vCard xmlns='vcard-temp'>"
                "<FN>Peter Saint-Andre</FN>"
                "<N>"
                "<GIVEN>Peter</GIVEN>"
                "<FAMILY>Saint-Andre</FAMILY>"
                "</N>"
                "<TITLE/><ROLE/>"
                "</vCard>"
                "</iq>");
    test.inject("<iq id='qxmpp2' type='result'/>");

    expectFutureVariant<Success>(task);
}

void tst_QXmppVCardManager::testSetClientVCard()
{
    SKIP_IF_INTEGRATION_TESTS_DISABLED();

    auto client = std::make_unique<QXmppClient>();
    auto *vCardManager = client->findExtension<QXmppVCardManager>();
    auto config = IntegrationTests::clientConfiguration();

    QSignalSpy connectSpy(client.get(), &QXmppClient::connected);
    QSignalSpy disconnectSpy(client.get(), &QXmppClient::disconnected);
    QSignalSpy vCardSpy(vCardManager, &QXmppVCardManager::clientVCardReceived);

    // connect to server
    client->connectToServer(config);
    QVERIFY2(connectSpy.wait(), "Could not connect to server!");

    // request own vcard
    vCardManager->requestClientVCard();
    QVERIFY(vCardSpy.wait());

    // check our vcard has the correct address
    QCOMPARE(vCardManager->clientVCard().from(), client->configuration().jidBare());

    // set a new vcard
    QXmppVCardIq newVCard;
    newVCard.setFirstName(u"Bob"_s);
    newVCard.setBirthday(QDate(1, 2, 2000));
    newVCard.setEmail(u"bob@qxmpp.org"_s);
    vCardManager->setClientVCard(newVCard);

    // there's currently no signal to see whether the change was successful...

    QCoreApplication::processEvents();

    // reconnect
    client->disconnectFromServer();
    QVERIFY(disconnectSpy.wait());

    client->connectToServer(config);
    QVERIFY2(connectSpy.wait(), "Could not connect to server!");

    // request own vcard
    vCardManager->requestClientVCard();
    QVERIFY(vCardSpy.wait());

    // check our vcard has been changed successfully
    QCOMPARE(vCardManager->clientVCard().from(), client->configuration().jidBare());
    QCOMPARE(vCardManager->clientVCard().firstName(), u"Bob"_s);
    QCOMPARE(vCardManager->clientVCard().birthday(), QDate(01, 02, 2000));
    QCOMPARE(vCardManager->clientVCard().email(), u"bob@qxmpp.org"_s);

    // reset the vcard for future tests
    vCardManager->setClientVCard(QXmppVCardIq());

    // disconnect
    client->disconnectFromServer();
    QVERIFY(disconnectSpy.wait());
}

QTEST_MAIN(tst_QXmppVCardManager)
#include "tst_qxmppvcardmanager.moc"
