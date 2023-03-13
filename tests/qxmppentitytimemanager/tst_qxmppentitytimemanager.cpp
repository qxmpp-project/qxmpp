// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEntityTimeIq.h"
#include "QXmppEntityTimeManager.h"

#include "TestClient.h"

Q_DECLARE_METATYPE(QXmppEntityTimeIq)

class tst_QXmppEntityTimeManager : public QObject
{
    Q_OBJECT
    Q_SLOT void initTestCase();
    Q_SLOT void testSendRequest();
    Q_SLOT void testHandleRequest();
};

void tst_QXmppEntityTimeManager::initTestCase()
{
    qRegisterMetaType<QXmppEntityTimeIq>();
}

void tst_QXmppEntityTimeManager::testSendRequest()
{
    TestClient test;
    auto *manager = test.addNewExtension<QXmppEntityTimeManager>();

    QSignalSpy spy(manager, &QXmppEntityTimeManager::timeReceived);

    manager->requestTime("juliet@capulet.com/balcony");
    test.expect("<iq id='qxmpp1' to='juliet@capulet.com/balcony' type='get'><time xmlns='urn:xmpp:time'/></iq>");
    manager->handleStanza(xmlToDom(R"(<iq id='qxmpp1' to='romeo@montague.net/orchard' from='juliet@capulet.com/balcony' type='result'>
  <time xmlns='urn:xmpp:time'>
    <tzo>-06:00</tzo>
    <utc>2006-12-19T17:58:35Z</utc>
  </time>
</iq>)"));

    QCOMPARE(spy.size(), 1);
    auto time = spy.at(0).at(0).value<QXmppEntityTimeIq>();
    QCOMPARE(time.utc(), QDateTime({2006, 12, 19}, {17, 58, 35}, Qt::UTC));
    QCOMPARE(time.tzo(), -6 * 60 * 60);
}

void tst_QXmppEntityTimeManager::testHandleRequest()
{
    TestClient test;
    test.configuration().setJid("juliet@capulet.com/balcony");

    auto *manager = test.addNewExtension<QXmppEntityTimeManager>();

    manager->handleStanza(xmlToDom(R"(<iq type='get' from='romeo@montague.net/orchard' to='juliet@capulet.com/balcony' id='time_1'>
  <time xmlns='urn:xmpp:time'/>
</iq>)"));

    auto packet = xmlToDom(test.takePacket());
    QVERIFY(QXmppEntityTimeIq::isEntityTimeIq(packet));
    QXmppEntityTimeIq resp;
    resp.parse(packet);
    
    QCOMPARE(resp.id(), QStringLiteral("time_1"));
    QCOMPARE(resp.type(), QXmppIq::Result);
}

QTEST_MAIN(tst_QXmppEntityTimeManager)
#include "tst_qxmppentitytimemanager.moc"
