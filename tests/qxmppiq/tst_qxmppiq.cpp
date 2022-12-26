// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIq.h"

#include "util.h"
#include <QObject>

class tst_QXmppIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic_data();
    Q_SLOT void testBasic();
};

void tst_QXmppIq::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");

    QTest::newRow("get")
        << QByteArray(R"(<iq to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="get"/>)")
        << int(QXmppIq::Get);

    QTest::newRow("set")
        << QByteArray(R"(<iq to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="set"/>)")
        << int(QXmppIq::Set);

    QTest::newRow("result")
        << QByteArray(R"(<iq to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="result"/>)")
        << int(QXmppIq::Result);

    QTest::newRow("error")
        << QByteArray(R"(<iq to="foo@example.com/QXmpp" from="bar@example.com/QXmpp" type="error"/>)")
        << int(QXmppIq::Error);
}

void tst_QXmppIq::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);

    QXmppIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.to(), QString("foo@example.com/QXmpp"));
    QCOMPARE(iq.from(), QString("bar@example.com/QXmpp"));
    QCOMPARE(int(iq.type()), type);
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppIq)
#include "tst_qxmppiq.moc"
