// SPDX-FileCopyrightText: 2012 Oliver Goffart <ogoffart@woboq.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPresence.h"

#include "util.h"

#include <QDateTime>
#include <QObject>

class tst_QXmppPresence : public QObject
{
    Q_OBJECT

private:
    Q_SLOT void testPresence();
    Q_SLOT void testPresence_data();
    Q_SLOT void testPresenceWithCapability();
    Q_SLOT void testPresenceWithExtendedAddresses();
    Q_SLOT void testPresenceWithMucItem();
    Q_SLOT void testPresenceWithMucPassword();
    Q_SLOT void testPresenceWithMucSupport();
    Q_SLOT void testPresenceWithMuji();
    Q_SLOT void testPresenceWithLastUserInteraction();
    Q_SLOT void testPresenceWithMix();
    Q_SLOT void testPresenceWithVCard();
};

void tst_QXmppPresence::testPresence_data()
{
    QXmppPresence foo;

    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("priority");
    QTest::addColumn<int>("statusType");
    QTest::addColumn<QString>("statusText");
    QTest::addColumn<int>("vcardUpdate");
    QTest::addColumn<QByteArray>("photoHash");

    // presence type
    QTest::newRow("available") << QByteArray("<presence/>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("unavailable") << QByteArray("<presence type=\"unavailable\"/>") << int(QXmppPresence::Unavailable) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("error") << QByteArray("<presence type=\"error\"/>") << int(QXmppPresence::Error) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("subscribe") << QByteArray("<presence type=\"subscribe\"/>") << int(QXmppPresence::Subscribe) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("unsubscribe") << QByteArray("<presence type=\"unsubscribe\"/>") << int(QXmppPresence::Unsubscribe) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("subscribed") << QByteArray("<presence type=\"subscribed\"/>") << int(QXmppPresence::Subscribed) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("unsubscribed") << QByteArray("<presence type=\"unsubscribed\"/>") << int(QXmppPresence::Unsubscribed) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("probe") << QByteArray("<presence type=\"probe\"/>") << int(QXmppPresence::Probe) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();

    // status text + priority
    QTest::newRow("full") << QByteArray("<presence><show>away</show><status>In a meeting</status><priority>5</priority></presence>") << int(QXmppPresence::Available) << 5 << int(QXmppPresence::Away) << "In a meeting" << int(QXmppPresence::VCardUpdateNone) << QByteArray();

    // status type
    QTest::newRow("away") << QByteArray("<presence><show>away</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Away) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("dnd") << QByteArray("<presence><show>dnd</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::DND) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("chat") << QByteArray("<presence><show>chat</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Chat) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("xa") << QByteArray("<presence><show>xa</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::XA) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();
    QTest::newRow("invisible") << QByteArray("<presence><show>invisible</show></presence>") << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Invisible) << "" << int(QXmppPresence::VCardUpdateNone) << QByteArray();

    // photo
    QTest::newRow("vcard-photo") << QByteArray(
                                        "<presence>"
                                        "<x xmlns=\"vcard-temp:x:update\">"
                                        "<photo>73b908bc</photo>"
                                        "</x>"
                                        "</presence>")
                                 << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateValidPhoto) << QByteArray::fromHex("73b908bc");

    QTest::newRow("vard-not-ready") << QByteArray(
                                           "<presence>"
                                           "<x xmlns=\"vcard-temp:x:update\"/>"
                                           "</presence>")
                                    << int(QXmppPresence::Available) << 0 << int(QXmppPresence::Online) << "" << int(QXmppPresence::VCardUpdateNotReady) << QByteArray();
}

void tst_QXmppPresence::testPresence()
{
    QFETCH(QByteArray, xml);
    QFETCH(int, type);
    QFETCH(int, priority);
    QFETCH(int, statusType);
    QFETCH(QString, statusText);
    QFETCH(int, vcardUpdate);
    QFETCH(QByteArray, photoHash);

    // test parsing and serialization after parsing
    QXmppPresence parsedPresence;
    parsePacket(parsedPresence, xml);
    QCOMPARE(int(parsedPresence.type()), type);
    QCOMPARE(parsedPresence.priority(), priority);
    QCOMPARE(int(parsedPresence.availableStatusType()), statusType);
    QCOMPARE(parsedPresence.statusText(), statusText);
    QCOMPARE(int(parsedPresence.vCardUpdateType()), vcardUpdate);
    QCOMPARE(parsedPresence.photoHash(), photoHash);

    serializePacket(parsedPresence, xml);

    // test serialization from setters
    QXmppPresence presence;
    presence.setType(static_cast<QXmppPresence::Type>(type));
    presence.setPriority(priority);
    presence.setAvailableStatusType(static_cast<QXmppPresence::AvailableStatusType>(statusType));
    presence.setStatusText(statusText);
    presence.setVCardUpdateType(static_cast<QXmppPresence::VCardUpdateType>(vcardUpdate));
    presence.setPhotoHash(photoHash);

    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithCapability()
{
    const QByteArray xml(
        "<presence to=\"foo@example.com/QXmpp\" from=\"bar@example.com/QXmpp\">"
        "<show>away</show>"
        "<status>In a meeting</status>"
        "<priority>5</priority>"
        "<c xmlns=\"http://jabber.org/protocol/caps\" hash=\"sha-1\" node=\"https://github.com/qxmpp-project/qxmpp\" ver=\"QgayPKawpkPSDYmwT/WM94uAlu0=\"/>"
        "<x xmlns=\"vcard-temp:x:update\">"
        "<photo>73b908bc</photo>"
        "</x>"
        "<x xmlns=\"urn:other:namespace\"/>"
        "</presence>");

    // test parsing and serialization after parsing
    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), u"foo@example.com/QXmpp"_s);
    QCOMPARE(presence.from(), u"bar@example.com/QXmpp"_s);
    QCOMPARE(presence.availableStatusType(), QXmppPresence::Away);
    QCOMPARE(presence.statusText(), u"In a meeting"_s);
    QCOMPARE(presence.priority(), 5);
    QCOMPARE(presence.photoHash(), QByteArray::fromHex("73b908bc"));
    QCOMPARE(presence.vCardUpdateType(), QXmppPresence::VCardUpdateValidPhoto);
    QCOMPARE(presence.capabilityHash(), u"sha-1"_s);
    QCOMPARE(presence.capabilityNode(), u"https://github.com/qxmpp-project/qxmpp"_s);
    QCOMPARE(presence.capabilityVer(), QByteArray::fromBase64("QgayPKawpkPSDYmwT/WM94uAlu0="));
    QCOMPARE(presence.extensions().first().tagName(), u"x"_s);
    QCOMPARE(presence.extensions().first().attribute(u"xmlns"_s), u"urn:other:namespace");

    serializePacket(presence, xml);

    // test serialization from setters
    QXmppPresence presence2;
    presence2.setTo(u"foo@example.com/QXmpp"_s);
    presence2.setFrom(u"bar@example.com/QXmpp"_s);
    presence2.setAvailableStatusType(QXmppPresence::Away);
    presence2.setStatusText(u"In a meeting"_s);
    presence2.setPriority(5);
    presence2.setPhotoHash(QByteArray::fromHex("73b908bc"));
    presence2.setVCardUpdateType(QXmppPresence::VCardUpdateValidPhoto);
    presence2.setCapabilityHash(u"sha-1"_s);
    presence2.setCapabilityNode(u"https://github.com/qxmpp-project/qxmpp"_s);
    presence2.setCapabilityVer(QByteArray::fromBase64("QgayPKawpkPSDYmwT/WM94uAlu0="));

    QXmppElement unknownExtension;
    unknownExtension.setTagName(u"x"_s);
    unknownExtension.setAttribute(u"xmlns"_s, u"urn:other:namespace"_s);
    presence2.setExtensions(QXmppElementList() << unknownExtension);

    serializePacket(presence2, xml);
}

void tst_QXmppPresence::testPresenceWithExtendedAddresses()
{
    const QByteArray xml(
        "<presence to=\"multicast.jabber.org\" from=\"hildjj@jabber.com\" type=\"unavailable\">"
        "<addresses xmlns=\"http://jabber.org/protocol/address\">"
        "<address jid=\"temas@jabber.org\" type=\"bcc\"/>"
        "<address jid=\"jer@jabber.org\" type=\"bcc\"/>"
        "</addresses>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.extendedAddresses().size(), 2);
    QCOMPARE(presence.extendedAddresses()[0].description(), QString());
    QCOMPARE(presence.extendedAddresses()[0].jid(), QLatin1String("temas@jabber.org"));
    QCOMPARE(presence.extendedAddresses()[0].type(), QLatin1String("bcc"));
    QCOMPARE(presence.extendedAddresses()[1].description(), QString());
    QCOMPARE(presence.extendedAddresses()[1].jid(), QLatin1String("jer@jabber.org"));
    QCOMPARE(presence.extendedAddresses()[1].type(), QLatin1String("bcc"));
    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithMucItem()
{
    const QByteArray xml(
        "<presence to=\"pistol@shakespeare.lit/harfleur\" "
        "from=\"harfleur@henryv.shakespeare.lit/pistol\" "
        "type=\"unavailable\">"
        "<x xmlns=\"http://jabber.org/protocol/muc#user\">"
        "<item affiliation=\"none\" role=\"none\">"
        "<actor jid=\"fluellen@shakespeare.lit\"/>"
        "<reason>Avaunt, you cullion!</reason>"
        "</item>"
        "<status code=\"307\"/>"
        "</x>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QLatin1String("pistol@shakespeare.lit/harfleur"));
    QCOMPARE(presence.from(), QLatin1String("harfleur@henryv.shakespeare.lit/pistol"));
    QCOMPARE(presence.type(), QXmppPresence::Unavailable);
    QCOMPARE(presence.mucItem().actor(), QLatin1String("fluellen@shakespeare.lit"));
    QCOMPARE(presence.mucItem().affiliation(), QXmppMucItem::NoAffiliation);
    QCOMPARE(presence.mucItem().jid(), QString());
    QCOMPARE(presence.mucItem().reason(), QLatin1String("Avaunt, you cullion!"));
    QCOMPARE(presence.mucItem().role(), QXmppMucItem::NoRole);
    QCOMPARE(presence.mucStatusCodes(), QList<int>() << 307);
    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithMucPassword()
{
    const QByteArray xml(
        "<presence to=\"coven@chat.shakespeare.lit/thirdwitch\" "
        "from=\"hag66@shakespeare.lit/pda\">"
        "<x xmlns=\"http://jabber.org/protocol/muc\">"
        "<password>pass</password>"
        "</x>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QLatin1String("coven@chat.shakespeare.lit/thirdwitch"));
    QCOMPARE(presence.from(), QLatin1String("hag66@shakespeare.lit/pda"));
    QCOMPARE(presence.type(), QXmppPresence::Available);
    QCOMPARE(presence.isMucSupported(), true);
    QCOMPARE(presence.mucPassword(), QLatin1String("pass"));
    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithMucSupport()
{
    const QByteArray xml(
        "<presence to=\"coven@chat.shakespeare.lit/thirdwitch\" "
        "from=\"hag66@shakespeare.lit/pda\">"
        "<x xmlns=\"http://jabber.org/protocol/muc\"/>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QCOMPARE(presence.to(), QLatin1String("coven@chat.shakespeare.lit/thirdwitch"));
    QCOMPARE(presence.from(), QLatin1String("hag66@shakespeare.lit/pda"));
    QCOMPARE(presence.type(), QXmppPresence::Available);
    QCOMPARE(presence.isMucSupported(), true);
    QVERIFY(presence.mucPassword().isEmpty());
    serializePacket(presence, xml);
}

void tst_QXmppPresence::testPresenceWithMuji()
{
    const QByteArray xml(
        "<presence>"
        "<muji xmlns=\"urn:xmpp:jingle:muji:0\">"
        "<preparing/>"
        "<content creator=\"initiator\" name=\"video\"/>"
        "<content creator=\"initiator\" name=\"voice\"/>"
        "</muji>"
        "</presence>");

    QXmppPresence presence1;
    QVERIFY(!presence1.isPreparingMujiSession());
    QVERIFY(presence1.mujiContents().isEmpty());
    parsePacket(presence1, xml);

    QVERIFY(presence1.isPreparingMujiSession());
    QCOMPARE(presence1.mujiContents().size(), 2);
    QCOMPARE(presence1.mujiContents().at(0).name(), u"video"_s);
    QCOMPARE(presence1.mujiContents().at(1).name(), u"voice"_s);
    serializePacket(presence1, xml);

    QXmppPresence presence2;
    presence2.setIsPreparingMujiSession(true);
    QXmppJingleIq::Content mujiContent1;
    mujiContent1.setCreator(u"initiator"_s);
    mujiContent1.setName(u"video"_s);
    QXmppJingleIq::Content mujiContent2;
    mujiContent2.setCreator(u"initiator"_s);
    mujiContent2.setName(u"voice"_s);
    presence2.setMujiContents({ mujiContent1, mujiContent2 });

    QVERIFY(presence2.isPreparingMujiSession());
    QCOMPARE(presence2.mujiContents().size(), 2);
    QCOMPARE(presence2.mujiContents().at(0).name(), u"video"_s);
    QCOMPARE(presence2.mujiContents().at(1).name(), u"voice"_s);
    serializePacket(presence2, xml);
}

void tst_QXmppPresence::testPresenceWithLastUserInteraction()
{
    const QByteArray xml(
        "<presence to=\"coven@chat.shakespeare.lit/thirdwitch\" "
        "from=\"hag66@shakespeare.lit/pda\">"
        "<idle xmlns=\"urn:xmpp:idle:1\" since=\"1969-07-21T02:56:15Z\"/>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);
    QVERIFY(!presence.lastUserInteraction().isNull());
    QVERIFY(presence.lastUserInteraction().isValid());
    QCOMPARE(presence.lastUserInteraction(), QDateTime(QDate(1969, 7, 21), QTime(2, 56, 15), TimeZoneUTC));
    serializePacket(presence, xml);

    QDateTime another(QDate(2025, 2, 5), QTime(15, 32, 8), TimeZoneUTC);
    presence.setLastUserInteraction(another);
    QCOMPARE(presence.lastUserInteraction(), another);
}

void tst_QXmppPresence::testPresenceWithMix()
{
    const QByteArray xml(
        "<presence to=\"hag99@shakespeare.example\" "
        "from=\"123435#coven@mix.shakespeare.example/UUID-a1j/7533\">"
        "<show>dnd</show>"
        "<status>Making a Brew</status>"
        "<mix xmlns=\"urn:xmpp:presence:0\">"
        "<jid>hecate@shakespeare.example/UUID-x4r/2491</jid>"
        "<nick>thirdwitch</nick>"
        "</mix>"
        "</presence>");

    QXmppPresence presence;
    parsePacket(presence, xml);

    QCOMPARE(presence.mixUserJid(), u"hecate@shakespeare.example/UUID-x4r/2491"_s);
    QCOMPARE(presence.mixUserNick(), u"thirdwitch"_s);
    serializePacket(presence, xml);

    presence.setMixUserJid("alexander@example.org");
    QCOMPARE(presence.mixUserJid(), u"alexander@example.org"_s);
    presence.setMixUserNick("erik");
    QCOMPARE(presence.mixUserNick(), u"erik"_s);
}

void tst_QXmppPresence::testPresenceWithVCard()
{
}

QTEST_MAIN(tst_QXmppPresence)
#include "tst_qxmpppresence.moc"
