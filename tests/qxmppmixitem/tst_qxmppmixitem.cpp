// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixItem.h"
#include "QXmppPubSubIq.h"

#include "util.h"
#include <QDomDocument>
#include <QObject>

class tst_QXmppMixItem : public QObject
{
    Q_OBJECT

private slots:
    void testInfo();
    void testIsInfoItem();
    void testParticipant();
    void testIsParticipantItem();
};

void tst_QXmppMixItem::testInfo()
{
    const QByteArray xml(
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:core:1</value>"
        "</field>"
        "<field type=\"text-single\" var=\"Name\">"
        "<value>Witches Coven</value>"
        "</field>"
        "<field type=\"text-single\" var=\"Description\">"
        "<value>A location not far from the blasted heath where the "
        "three witches meet</value>"
        "</field>"
        "<field type=\"jid-multi\" var=\"Contact\">"
        "<value>greymalkin@shakespeare.example</value>"
        "<value>joan@shakespeare.example</value>"
        "</field>"
        "</x>");

    QXmppMixInfoItem item;
    parsePacket(item, xml);

    QCOMPARE(item.name(), QString("Witches Coven"));
    QCOMPARE(item.description(), QString("A location not far from the blasted "
                                         "heath where the three witches meet"));
    QCOMPARE(item.contactJids(), QStringList() << "greymalkin@shakespeare.example"
                                               << "joan@shakespeare.example");

    QXmppElement element = item.toElement();
    serializePacket(element, xml);

    // test setters
    item.setName("Skynet Development");
    QCOMPARE(item.name(), QString("Skynet Development"));
    item.setDescription("Very cool development group.");
    QCOMPARE(item.description(), QString("Very cool development group."));
    item.setContactJids(QStringList() << "somebody@example.org");
    QCOMPARE(item.contactJids(), QStringList() << "somebody@example.org");
}

void tst_QXmppMixItem::testIsInfoItem()
{
    QDomDocument doc;
    QDomElement element;

    const QByteArray xmlCorrect(
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>urn:xmpp:mix:core:1</value>"
        "</field>"
        "</x>");
    QCOMPARE(doc.setContent(xmlCorrect, true), true);
    element = doc.documentElement();
    QVERIFY(QXmppMixInfoItem::isMixChannelInfo(element));

    const QByteArray xmlWrong(
        "<x xmlns=\"jabber:x:data\" type=\"result\">"
        "<field type=\"hidden\" var=\"FORM_TYPE\">"
        "<value>other:namespace</value>"
        "</field>"
        "</x>");
    QCOMPARE(doc.setContent(xmlWrong, true), true);
    element = doc.documentElement();
    QVERIFY(!QXmppMixInfoItem::isMixChannelInfo(element));
}

void tst_QXmppMixItem::testParticipant()
{
    const QByteArray xml(
        "<participant xmlns=\"urn:xmpp:mix:core:1\">"
        "<jid>hag66@shakespeare.example</jid>"
        "<nick>thirdwitch</nick>"
        "</participant>");

    QXmppMixParticipantItem item;
    parsePacket(item, xml);

    QCOMPARE(item.nick(), QString("thirdwitch"));
    QCOMPARE(item.jid(), QString("hag66@shakespeare.example"));

    QXmppElement element = item.toElement();
    serializePacket(element, xml);

    // test setters
    item.setNick("thomasd");
    QCOMPARE(item.nick(), QString("thomasd"));
    item.setJid("thomas@d.example");
    QCOMPARE(item.jid(), QString("thomas@d.example"));
}

void tst_QXmppMixItem::testIsParticipantItem()
{
    QDomDocument doc;
    QDomElement element;

    const QByteArray xmlCorrect(
        "<participant xmlns=\"urn:xmpp:mix:core:1\">"
        "</participant>");
    QCOMPARE(doc.setContent(xmlCorrect, true), true);
    element = doc.documentElement();
    QVERIFY(QXmppMixParticipantItem::isMixParticipantItem(element));

    const QByteArray xmlWrong(
        "<participant xmlns=\"other:namespace:1\">"
        "</participant>");
    QCOMPARE(doc.setContent(xmlWrong, true), true);
    element = doc.documentElement();
    QVERIFY(!QXmppMixParticipantItem::isMixParticipantItem(element));
}

QTEST_MAIN(tst_QXmppMixItem)
#include "tst_qxmppmixitem.moc"
