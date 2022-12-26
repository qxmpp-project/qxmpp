// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStartTlsPacket.h"

#include "util.h"
#include <QObject>

class tst_QXmppStartTlsPacket : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testBasic_data();
    Q_SLOT void testBasic();
};

void tst_QXmppStartTlsPacket::testBasic_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QXmppStartTlsPacket::Type>("type");

#define ROW(name, xml, valid, type) \
    QTest::newRow(name)             \
        << QByteArrayLiteral(xml)   \
        << valid                    \
        << type

    ROW("starttls", R"(<starttls xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>)", true, QXmppStartTlsPacket::StartTls);
    ROW("proceed", R"(<proceed xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>)", true, QXmppStartTlsPacket::Proceed);
    ROW("failure", R"(<failure xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>)", true, QXmppStartTlsPacket::Failure);

    ROW("invalid-tag", R"(<invalid-tag-name xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>)", false, QXmppStartTlsPacket::StartTls);

#undef ROW
}

void tst_QXmppStartTlsPacket::testBasic()
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, valid);
    QFETCH(QXmppStartTlsPacket::Type, type);

    QDomDocument doc;
    QVERIFY(doc.setContent(xml, true));
    QCOMPARE(QXmppStartTlsPacket::isStartTlsPacket(doc.documentElement()), valid);
    QCOMPARE(QXmppStartTlsPacket::isStartTlsPacket(doc.documentElement(), type), valid);

    // test other types return false
    for (auto testValue : { QXmppStartTlsPacket::StartTls,
                            QXmppStartTlsPacket::Proceed,
                            QXmppStartTlsPacket::Failure }) {
        QCOMPARE(QXmppStartTlsPacket::isStartTlsPacket(doc.documentElement(), testValue), testValue == type && valid);
    }

    if (valid) {
        QXmppStartTlsPacket packet;
        parsePacket(packet, xml);
        QCOMPARE(packet.type(), type);
        serializePacket(packet, xml);

        QXmppStartTlsPacket packet2(type);
        serializePacket(packet2, xml);

        QXmppStartTlsPacket packet3;
        packet3.setType(type);
        serializePacket(packet2, xml);
    }
}

QTEST_MAIN(tst_QXmppStartTlsPacket)
#include "tst_qxmppstarttlspacket.moc"
