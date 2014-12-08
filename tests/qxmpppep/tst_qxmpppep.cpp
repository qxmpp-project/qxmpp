/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Juan Aragon
 *
 * Source:
 *  http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <QObject>
#include "QXmppReachAddress.h"
#include "QXmppPEPManager.h"
#include "QXmppPubSubIq.h"
#include"QXmppGaming.h"

#include "util.h"
#include "QXmppConstants.h"


#include <QDebug>

class tst_QXmppPep : public QObject
{
    Q_OBJECT


private slots:
    void testReachabilityAdrressParser();
    void testCreateReachabiltiyAddressItem();
    void testHandleReachabilityAddressesEvent();
    void testReachabilityAddressReceivedSlot(const QString &jid, const QString &id, const QXmppReachAddress& reachAddres);
    void testPublishReachabilityAddress();
    void testCreateGamingItem();
private:
     QXmppPEPManager *m_PEPmanager;

};



void tst_QXmppPep::testCreateReachabiltiyAddressItem()
{

    // expected
    const QByteArray expectedXml(
                "<reach xmlns=\"urn:xmpp:reach:0\">"
                  "<addr uri=\"tel:+1-303-555-1212\">"
                    "<desc xml:lang=\"en\">Conference room phone</desc>"
                  "</addr>"
                  "<addr uri=\"sip:room123@example.com\">"
                    "<desc xml:lang=\"en\">In-room video system</desc>"
                  "</addr>"
                "</reach>");

    // tests
    QXmppAddress addr;
    QXmppReachAddress reachAddress;

    addr.setAddress("tel:+1-303-555-1212");
    addr.setDescription("Conference room phone");
    addr.setLanguage("en");

    reachAddress.addAddress(addr);

    addr.setAddress("sip:room123@example.com");
    addr.setDescription("In-room video system");
    addr.setLanguage("en");

    reachAddress.addAddress(addr);
    QXmppElement reachElement (reachAddress.toQXmppElement());

    serializePacket(reachElement, expectedXml);

}

void tst_QXmppPep::testHandleReachabilityAddressesEvent()
{
    const QByteArray inputXml(
                "<message from='pubsub.shakespeare.lit'"
                         "to='juliet@capulet.com'>"
                "<event xmlns='http://jabber.org/protocol/pubsub#event'>"
                "<items node='urn:xmpp:reach:0'>"
                      "<item id='a1s2d3f4g5h6bjeh936'>"
                        "<reach xmlns='urn:xmpp:reach:0'>"
                          "<addr uri='tel:+1-303-555-1212'>"
                            "<desc xml:lang='en'>Conference room phone</desc>"
                          "</addr>"
                          "<addr uri='sip:room123@example.com'>"
                            "<desc xml:lang='en'>In-room video system</desc>"
                          "</addr>"
                        "</reach>"
                      "</item>"
                    "</items>"
                  "</event>"
                "</message>");

    m_PEPmanager = new QXmppPEPManager(true);

    bool connection = false;
    connection = QObject::connect(m_PEPmanager, SIGNAL(reachabilityAddressReceived(QString,QString,QXmppReachAddress)), this, SLOT(testReachabilityAddressReceivedSlot(QString,QString,QXmppReachAddress)));
    QCOMPARE(connection, true);

    QDomDocument doc;
    QCOMPARE(doc.setContent(inputXml, true), true);
    QDomElement element = doc.documentElement();

    const bool parsedOk = m_PEPmanager->handleStanza(element);
    QCOMPARE(parsedOk, true);


}

void tst_QXmppPep::testReachabilityAdrressParser()
{
    const QByteArray inputXml(
                        "<reach xmlns='urn:xmpp:reach:0'>"
                          "<addr uri='tel:+1-303-555-1212'>"
                            "<desc xml:lang='en'>Conference room phone</desc>"
                          "</addr>"
                          "<addr uri='sip:room123@example.com'>"
                            "<desc xml:lang='en'>In-room video system</desc>"
                          "</addr>"
                        "</reach>"
                     );

    QDomDocument doc;
    QCOMPARE(doc.setContent(inputXml, true), true);
    QDomElement element = doc.documentElement();

    QXmppReachAddress reachAddres;
    reachAddres.parse(element);

    QCOMPARE(reachAddres.isNull(),false);
    if(!reachAddres.isNull())
    {
        QCOMPARE(reachAddres.getAddresses().size(),2);

        QCOMPARE(reachAddres.getAddresses().at(0).getAddress(), QString("tel:+1-303-555-1212"));
        QCOMPARE(reachAddres.getAddresses().at(0).getDescription(), QString("Conference room phone"));
        QCOMPARE(reachAddres.getAddresses().at(0).getLanguage(), QString("en"));

        QCOMPARE(reachAddres.getAddresses().at(1).getAddress(), QString("sip:room123@example.com"));
        QCOMPARE(reachAddres.getAddresses().at(1).getDescription(), QString("In-room video system"));
        QCOMPARE(reachAddres.getAddresses().at(1).getLanguage(), QString("en"));
    }
}


void tst_QXmppPep::testReachabilityAddressReceivedSlot(const QString &jid, const QString &id, const QXmppReachAddress& reachAddres)
{
    QCOMPARE(jid,QString("pubsub.shakespeare.lit"));
    QCOMPARE(id,QString("a1s2d3f4g5h6bjeh936"));
    QCOMPARE(reachAddres.isNull(),false);
    if(!reachAddres.isNull())
    {
        QCOMPARE(reachAddres.getAddresses().size(),2);

        QCOMPARE(reachAddres.getAddresses().at(0).getAddress(), QString("tel:+1-303-555-1212"));
        QCOMPARE(reachAddres.getAddresses().at(0).getDescription(), QString("Conference room phone"));
        QCOMPARE(reachAddres.getAddresses().at(0).getLanguage(), QString("en"));

        QCOMPARE(reachAddres.getAddresses().at(1).getAddress(), QString("sip:room123@example.com"));
        QCOMPARE(reachAddres.getAddresses().at(1).getDescription(), QString("In-room video system"));
        QCOMPARE(reachAddres.getAddresses().at(1).getLanguage(), QString("en"));
    }
}

void tst_QXmppPep::testPublishReachabilityAddress()
{
    const QByteArray xml(
                "<iq id=\"publish1\""
                    " to=\"pubsub.shakespeare.example\""
                    " from=\"romeo@example.com\""
                    " type=\"set\">"
                  "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\">"
                    "<publish node=\"urn:xmpp:reach:0\">"
                      "<item id=\"a1s2d3f4g5h6bjeh936\">"
                        "<reach xmlns=\"urn:xmpp:reach:0\">"
                          "<addr uri=\"tel:+1-303-555-1212\">"
                            "<desc xml:lang=\"en\">Conference room phone</desc>"
                          "</addr>"
                          "<addr uri=\"sip:room123@example.com\">"
                            "<desc xml:lang=\"en\">In-room video system</desc>"
                          "</addr>"
                        "</reach>"
                      "</item>"
                    "</publish>"
                  "</pubsub>"
                "</iq>" );

    QXmppReachAddress reachAddr;
    QXmppAddress addr1("tel:+1-303-555-1212", "Conference room phone", "en");
    QXmppAddress addr2("sip:room123@example.com", "In-room video system", "en");
    reachAddr.addAddress(addr1);
    reachAddr.addAddress(addr2);

    QXmppPubSubIq publish;
    QXmppPubSubItem item;
    QList <QXmppPubSubItem> listItems;

    publish.setType(QXmppIq::Set);
    publish.setQueryType(QXmppPubSubIq::PublishQuery);
    publish.setFrom("romeo@example.com");
    publish.setTo("pubsub.shakespeare.example");
    publish.setId("publish1");
    publish.setQueryNode("urn:xmpp:reach:0");


    item.setId("a1s2d3f4g5h6bjeh936");
    item.setContents(reachAddr.toQXmppElement());

    listItems.append(item);

    publish.setItems(listItems);

    serializePacket(publish, xml);
}


void tst_QXmppPep::testCreateGamingItem()
{

    // expected
    const QByteArray expectedXml(
                "<game xmlns=\"urn:xmpp:gaming:0\">"
                  "<character_name>Ingralic</character_name>"
                  "<character_profile>http://www.chesspark.com/Ingralic/</character_profile>"
                  "<name>chess</name>"
                  "<level>91</level>"
                  "<server_address>http://www.chesspark.com/Server/</server_address>"
                  "<server_name>Abyss</server_name>"
                  "<uri>http://www.chesspark.com/</uri>"
                "</game>");

    // tests
    QXmppGaming gaming;
    parsePacket(gaming, expectedXml);
    QCOMPARE(gaming.characterName(), QString("Ingralic"));
    QCOMPARE(gaming.characterProfile(), QString("http://www.chesspark.com/Ingralic/"));
    QCOMPARE(gaming.name(), QString("chess"));
    QCOMPARE(gaming.level(), QString("91"));

    QCOMPARE(gaming.serverAddress(), QString("http://www.chesspark.com/Server/"));
    QCOMPARE(gaming.serverName(), QString("Abyss"));
    QCOMPARE(gaming.uri(), QString("http://www.chesspark.com/"));
    serializePacket(gaming, expectedXml);
}

QTEST_MAIN(tst_QXmppPep)
#include "tst_qxmpppep.moc"
