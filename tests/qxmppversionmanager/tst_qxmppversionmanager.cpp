// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVersionIq.h"
#include "QXmppVersionManager.h"

#include "TestClient.h"

Q_DECLARE_METATYPE(QXmppVersionIq);

class tst_QXmppVersionManager : public QObject
{
    Q_OBJECT
    Q_SLOT void initTestCase();
    Q_SLOT void testSendRequest();
    Q_SLOT void testHandleRequest();
};

void tst_QXmppVersionManager::initTestCase()
{
    qRegisterMetaType<QXmppVersionIq>();
}

void tst_QXmppVersionManager::testSendRequest()
{
    TestClient test;
    auto *verManager = test.addNewExtension<QXmppVersionManager>();

    QSignalSpy spy(verManager, &QXmppVersionManager::versionReceived);

    auto id = verManager->requestVersion("juliet@capulet.com/balcony");
    test.expect("<iq id='qxmpp1' to='juliet@capulet.com/balcony' type='get'><query xmlns='jabber:iq:version'/></iq>");
    verManager->handleStanza(xmlToDom(R"(<iq type='result' from='juliet@capulet.com/balcony' id='qxmpp1'>
  <query xmlns='jabber:iq:version'>
    <name>Exodus</name>
    <version>0.7.0.4</version>
    <os>Windows-XP 5.01.2600</os>
  </query>
</iq>)"));

    QCOMPARE(spy.size(), 1);
    auto version = spy.at(0).at(0).value<QXmppVersionIq>();
    QCOMPARE(version.name(), u"Exodus"_s);
    QCOMPARE(version.version(), u"0.7.0.4"_s);
    QCOMPARE(version.os(), u"Windows-XP 5.01.2600"_s);
}

void tst_QXmppVersionManager::testHandleRequest()
{
    TestClient test;
    test.configuration().setJid("juliet@capulet.com/balcony");

    auto *verManager = test.addNewExtension<QXmppVersionManager>();
    verManager->setClientName("Exodus");
    verManager->setClientVersion("0.7.0.4");
    verManager->setClientOs("Windows-XP 5.01.2600");

    verManager->handleStanza(xmlToDom(R"(<iq type='get' from='romeo@montague.net/orchard' to='juliet@capulet.com/balcony' id='version_1'>
  <query xmlns='jabber:iq:version'/>
</iq>)"));
    test.expect(R"(<iq id='version_1' to='romeo@montague.net/orchard' type='result'>)"
                "<query xmlns='jabber:iq:version'><name>Exodus</name><os>Windows-XP 5.01.2600</os><version>0.7.0.4</version>"
                "</query></iq>");
}

QTEST_MAIN(tst_QXmppVersionManager)
#include "tst_qxmppversionmanager.moc"
