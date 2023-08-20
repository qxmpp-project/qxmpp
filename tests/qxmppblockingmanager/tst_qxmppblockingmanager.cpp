// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBlockingManager.h"
#include "QXmppE2eeMetadata.h"

#include "TestClient.h"

using namespace QXmpp;

class tst_QXmppBlockingManager : public QObject
{
    Q_OBJECT
private:
    Q_SLOT void basic();
    Q_SLOT void fetch();
    Q_SLOT void block();
    Q_SLOT void unblock();
    Q_SLOT void pushBlocked();
    Q_SLOT void blockedState();
};

void tst_QXmppBlockingManager::basic()
{
    QXmppBlockingManager m;
    QVERIFY(!m.isSubscribed());
}

void tst_QXmppBlockingManager::fetch()
{
    TestClient t;
    t.configuration().setJid("juliet@capulet.com");
    auto *m = t.addNewExtension<QXmppBlockingManager>();

    QVERIFY(!m->isSubscribed());

    auto task = m->fetchBlocklist();
    t.expect("<iq id='qxmpp1' type='get'><blocklist xmlns='urn:xmpp:blocking'/></iq>");
    t.inject("<iq type='result' id='qxmpp1'><blocklist xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/><item jid='iago@shakespeare.lit'/></blocklist></iq>");

    QVERIFY(m->isSubscribed());
    auto blocklist = expectFutureVariant<QXmppBlocklist>(task);
    QVector<QString> expected { "romeo@montague.net", "iago@shakespeare.lit" };
    QCOMPARE(blocklist.entries(), expected);

    // now the blocklist is cached
    task = m->fetchBlocklist();
    blocklist = expectFutureVariant<QXmppBlocklist>(task);
    QCOMPARE(blocklist.entries(), expected);

    QVERIFY(m->isSubscribed());
}

void tst_QXmppBlockingManager::block()
{
    TestClient t;
    t.configuration().setJid("juliet@capulet.com");
    auto *m = t.addNewExtension<QXmppBlockingManager>();

    auto task = m->block("romeo@montague.net");
    t.expect("<iq id='qxmpp1' type='set'><block xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/></block></iq>");
    t.inject("<iq type='result' id='qxmpp1'/>");
    expectFutureVariant<Success>(task);
}

void tst_QXmppBlockingManager::unblock()
{
    TestClient t;
    t.configuration().setJid("juliet@capulet.com");
    auto *m = t.addNewExtension<QXmppBlockingManager>();

    auto task = m->unblock("romeo@montague.net");
    t.expect("<iq id='qxmpp1' type='set'><unblock xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/></unblock></iq>");
    t.inject("<iq type='result' id='qxmpp1'/>");
    expectFutureVariant<Success>(task);
}

void tst_QXmppBlockingManager::pushBlocked()
{
    TestClient t;
    t.configuration().setJid("juliet@capulet.com/balcony");
    auto *m = t.addNewExtension<QXmppBlockingManager>();

    m->fetchBlocklist();
    t.expect("<iq id='qxmpp1' type='get'><blocklist xmlns='urn:xmpp:blocking'/></iq>");
    t.inject("<iq type='result' id='qxmpp1'><blocklist xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/><item jid='iago@shakespeare.lit'/></blocklist></iq>");

    QVERIFY(m->isSubscribed());

    QSignalSpy blockedSpy(m, &QXmppBlockingManager::blocked);
    QSignalSpy unblockedSpy(m, &QXmppBlockingManager::unblocked);

    auto dom = xmlToDom("<iq to='juliet@capulet.com/balcony' type='set' id='push4'><unblock xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/></unblock></iq>");
    QVERIFY(m->handleStanza(dom, {}));

    QCOMPARE(blockedSpy.size(), 0);
    QCOMPARE(unblockedSpy.size(), 1);
    QCOMPARE(unblockedSpy[0][0].value<QVector<QString>>(), QVector<QString> { "romeo@montague.net" });

    auto blocklist = std::get<QXmppBlocklist>(m->fetchBlocklist().result()).entries();
    QCOMPARE(blocklist, QVector<QString> { "iago@shakespeare.lit" });

    dom = xmlToDom("<iq to='juliet@capulet.com/balcony' type='set' id='push3'><block xmlns='urn:xmpp:blocking'><item jid='romeo@montague.net'/></block></iq>");
    QVERIFY(m->handleStanza(dom, {}));

    QCOMPARE(blockedSpy.size(), 1);
    QCOMPARE(blockedSpy[0][0].value<QVector<QString>>(), QVector<QString> { "romeo@montague.net" });
    QCOMPARE(unblockedSpy.size(), 1);

    blocklist = std::get<QXmppBlocklist>(m->fetchBlocklist().result()).entries();
    auto expected = QVector<QString> { "iago@shakespeare.lit", "romeo@montague.net" };
    QCOMPARE(blocklist, expected);
}

void tst_QXmppBlockingManager::blockedState()
{
    using L = QXmppBlocklist;
    auto entries = QVector<QString> {
        "iago@shakespeare.lit", "romeo@montague.net"
    };
    QXmppBlocklist l(entries);

    QVERIFY(l.containsEntry(u"iago@shakespeare.lit"));
    QVERIFY(!l.containsEntry(u"shakespeare.lit"));
    QCOMPARE(l.entries(), entries);

    auto state = l.blockingState("iago@shakespeare.lit");
    auto blocked = expectVariant<L::Blocked>(state);
    QCOMPARE(blocked.blockingEntries, QVector<QString> { "iago@shakespeare.lit" });
    QVERIFY(blocked.partiallyBlockingEntries.isEmpty());

    state = l.blockingState("iago@shakespeare.lit/res");
    blocked = expectVariant<L::Blocked>(state);
    QCOMPARE(blocked.blockingEntries, QVector<QString> { "iago@shakespeare.lit" });
    QVERIFY(blocked.partiallyBlockingEntries.isEmpty());

    state = l.blockingState("shakespeare.lit");
    auto partially = expectVariant<L::PartiallyBlocked>(state);
    QCOMPARE(partially.partiallyBlockingEntries, QVector<QString> { "iago@shakespeare.lit" });

    state = l.blockingState("qxmpp.org");
    expectVariant<L::NotBlocked>(state);
}

QTEST_MAIN(tst_QXmppBlockingManager)
#include "tst_qxmppblockingmanager.moc"
