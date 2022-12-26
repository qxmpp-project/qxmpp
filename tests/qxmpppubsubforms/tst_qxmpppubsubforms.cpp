// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDataFormBase.h"
#include "QXmppPubSubSubAuthorization.h"

#include "util.h"
#include <QObject>

class tst_QXmppPubSubForms : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void subAuthorization();
};

void tst_QXmppPubSubForms::subAuthorization()
{
    QByteArray xml(R"(
<x xmlns="jabber:x:data" type="form">
<field type="hidden" var="FORM_TYPE">
<value>http://jabber.org/protocol/pubsub#subscribe_authorization</value>
</field>
<field type="boolean" var="pubsub#allow"><value>0</value></field>
<field type="text-single" var="pubsub#node"><value>princely_musings</value></field>
<field type="text-single" var="pubsub#subid"><value>123-abc</value></field>
<field type="jid-single" var="pubsub#subscriber_jid"><value>horatio@denmark.lit</value></field>
</x>)");

    QXmppDataForm form;
    parsePacket(form, xml);

    auto subAuthForm = QXmppPubSubSubAuthorization::fromDataForm(form);
    QVERIFY(subAuthForm.has_value());
    QCOMPARE(subAuthForm->subid(), QStringLiteral("123-abc"));
    QCOMPARE(subAuthForm->node(), QStringLiteral("princely_musings"));
    QCOMPARE(subAuthForm->subscriberJid(), QStringLiteral("horatio@denmark.lit"));
    QVERIFY(subAuthForm->allowSubscription().has_value());
    QCOMPARE(subAuthForm->allowSubscription().value(), false);

    form = subAuthForm->toDataForm();
    QVERIFY(!form.isNull());
    xml = QString::fromUtf8(xml).remove(QChar('\n')).toUtf8();
    serializePacket(form, xml);
}

QTEST_MAIN(tst_QXmppPubSubForms)
#include "tst_qxmpppubsubforms.moc"
