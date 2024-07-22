// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppUri.h"

#include "util.h"

#include <QObject>

namespace Uri = QXmpp::Uri;
using namespace QXmpp::Private;

class tst_QXmppUri : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void base();
    Q_SLOT void queryMessage();
    Q_SLOT void queryRoster();
    Q_SLOT void queryRemove();
    Q_SLOT void queryOther();
};

void tst_QXmppUri::base()
{
    auto str = u"xmpp:lnj@qxmpp.org"_s;
    auto uri = unwrap(QXmppUri::fromString(str));
    QCOMPARE(uri.jid(), u"lnj@qxmpp.org");
    QVERIFY(!uri.query().has_value());
}

void tst_QXmppUri::queryMessage()
{
    const auto string = u"xmpp:romeo@montague.net?message;subject=Test%20Message;body=Here's%20a%20test%20message"_s;
    auto uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(uri.jid(), u"romeo@montague.net");
    auto message = unwrap<Uri::Message>(uri.query());
    QCOMPARE(message, (Uri::Message { u"Test Message"_s, u"Here's a test message"_s, {}, {}, {}, {} }));

    QCOMPARE(uri.toString(), string);
}

void tst_QXmppUri::queryRoster()
{
    const auto string = u"xmpp:romeo@montague.net?roster;name=Romeo%20Montague;group=Friends"_s;
    auto uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(uri.jid(), u"romeo@montague.net");
    auto message = unwrap<Uri::Roster>(uri.query());
    QCOMPARE(message, (Uri::Roster { u"Romeo Montague"_s, u"Friends"_s }));

    QCOMPARE(uri.toString(), string);
}

void tst_QXmppUri::queryRemove()
{
    const auto string = u"xmpp:romeo@montague.net?remove"_s;
    auto uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(uri.jid(), u"romeo@montague.net");
    auto message = unwrap<Uri::Remove>(uri.query());
    QCOMPARE(message, Uri::Remove {});

    QCOMPARE(uri.toString(), string);
}

void tst_QXmppUri::queryOther()
{
    auto string = u"xmpp:lnj@qxmpp.org?command;node=test2;action=next"_s;
    auto uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Command>(uri.query()), (Uri::Command { u"test2"_s, u"next"_s }));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:xsf@muc.xmpp.org?invite;jid=lnj@qxmpp.org;password=1234"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Invite>(uri.query()), (Uri::Invite { u"lnj@qxmpp.org"_s, u"1234"_s }));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:xsf@muc.xmpp.org?join;password=1234"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Join>(uri.query()), (Uri::Join { u"1234"_s }));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?register"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Register>(uri.query()), (Uri::Register {}));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?remove"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Remove>(uri.query()), (Uri::Remove {}));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?subscribe"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Subscribe>(uri.query()), (Uri::Subscribe {}));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?unregister"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Unregister>(uri.query()), (Uri::Unregister {}));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?unsubscribe"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::Unsubscribe>(uri.query()), (Uri::Unsubscribe {}));
    QCOMPARE(uri.toString(), string);

    string = u"xmpp:qxmpp.org?x-new-query;a=b;action=add"_s;
    uri = unwrap(QXmppUri::fromString(string));
    QCOMPARE(unwrap<Uri::CustomQuery>(uri.query()), (Uri::CustomQuery { u"x-new-query"_s, { { u"a"_s, u"b"_s }, { u"action"_s, u"add"_s } } }));
    QCOMPARE(uri.toString(), string);
}

QTEST_MAIN(tst_QXmppUri)
#include "tst_qxmppuri.moc"
