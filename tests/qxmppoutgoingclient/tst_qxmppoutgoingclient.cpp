// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutgoingClient.h"

#include "util.h"

class tst_QXmppOutgoingClient : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testParseHostAddress_data();
    Q_SLOT void testParseHostAddress();
};

void tst_QXmppOutgoingClient::testParseHostAddress_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("resultHost");
    QTest::addColumn<int>("resultPort");

    QTest::newRow("host-and-port")
        << QStringLiteral("qxmpp.org:443")
        << QStringLiteral("qxmpp.org")
        << 443;

    QTest::newRow("no-port")
        << QStringLiteral("qxmpp.org")
        << QStringLiteral("qxmpp.org")
        << -1;

    QTest::newRow("ipv4-with-port")
        << QStringLiteral("127.0.0.1:443")
        << QStringLiteral("127.0.0.1")
        << 443;

    QTest::newRow("ipv4-no-port")
        << QStringLiteral("127.0.0.1")
        << QStringLiteral("127.0.0.1")
        << -1;

    QTest::newRow("ipv6-with-port")
        << QStringLiteral("[2001:41D0:1:A49b::1]:9222")
        << QStringLiteral("2001:41d0:1:a49b::1")
        << 9222;

    QTest::newRow("ipv6-no-port")
        << QStringLiteral("[2001:41D0:1:A49b::1]")
        << QStringLiteral("2001:41d0:1:a49b::1")
        << -1;
}

void tst_QXmppOutgoingClient::testParseHostAddress()
{
    QFETCH(QString, input);
    QFETCH(QString, resultHost);
    QFETCH(int, resultPort);

    const auto address = QXmppOutgoingClient::parseHostAddress(input);
    QCOMPARE(address.first, resultHost);
    QCOMPARE(address.second, resultPort);
}

QTEST_MAIN(tst_QXmppOutgoingClient)
#include "tst_qxmppoutgoingclient.moc"
