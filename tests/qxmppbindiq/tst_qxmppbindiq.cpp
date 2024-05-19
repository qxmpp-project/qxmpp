// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBindIq.h"

#include "compat/QXmppSessionIq.h"
#include "util.h"

#include <QObject>

class tst_QXmppBindIq : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testNoResource();
    Q_SLOT void testResource();
    Q_SLOT void testResult();

    Q_SLOT void testSessionIq();
};

void tst_QXmppBindIq::testNoResource()
{
    const QByteArray xml(
        "<iq id=\"bind_1\" type=\"set\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"/>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Set);
    QCOMPARE(bind.id(), u"bind_1"_s);
    QCOMPARE(bind.jid(), QString());
    QCOMPARE(bind.resource(), QString());
    serializePacket(bind, xml);
}

void tst_QXmppBindIq::testResource()
{
    const QByteArray xml(
        "<iq id=\"bind_2\" type=\"set\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\">"
        "<resource>someresource</resource>"
        "</bind>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Set);
    QCOMPARE(bind.id(), u"bind_2"_s);
    QCOMPARE(bind.jid(), QString());
    QCOMPARE(bind.resource(), u"someresource"_s);
    serializePacket(bind, xml);
}

void tst_QXmppBindIq::testResult()
{
    const QByteArray xml(
        "<iq id=\"bind_2\" type=\"result\">"
        "<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\">"
        "<jid>somenode@example.com/someresource</jid>"
        "</bind>"
        "</iq>");

    QXmppBindIq bind;
    parsePacket(bind, xml);
    QCOMPARE(bind.type(), QXmppIq::Result);
    QCOMPARE(bind.id(), u"bind_2"_s);
    QCOMPARE(bind.jid(), u"somenode@example.com/someresource"_s);
    QCOMPARE(bind.resource(), QString());
    serializePacket(bind, xml);
}

void tst_QXmppBindIq::testSessionIq()
{
    const QByteArray xml(
        "<iq id=\"session_1\" to=\"example.com\" type=\"set\">"
        "<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/>"
        "</iq>");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    QXmppSessionIq session;
    QT_WARNING_POP

    parsePacket(session, xml);
    serializePacket(session, xml);
}

QTEST_MAIN(tst_QXmppBindIq)
#include "tst_qxmppbindiq.moc"
