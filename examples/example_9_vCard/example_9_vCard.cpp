/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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

#include <iostream>

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QXmlStreamWriter>

#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

#include "example_9_vCard.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check;
    Q_UNUSED(check);

    check = connect(this, SIGNAL(connected()),
                    SLOT(clientConnected()));
    Q_ASSERT(check);

    check = connect(&this->rosterManager(), SIGNAL(rosterReceived()),
                    SLOT(rosterReceived()));
    Q_ASSERT(check);
}

xmppClient::~xmppClient()
{

}

void xmppClient::clientConnected()
{
    std::cout<<"example_9_vCard:: CONNECTED"<<std::endl;
}

void xmppClient::rosterReceived()
{
    std::cout<<"example_9_vCard:: Roster Received"<<std::endl;
    bool check = connect(&this->vCardManager(), SIGNAL(vCardReceived(QXmppVCardIq)),
        SLOT(vCardReceived(QXmppVCardIq)));
    Q_ASSERT(check);
    Q_UNUSED(check);

    QStringList list = rosterManager().getRosterBareJids();
    for(int i = 0; i < list.size(); ++i)
    {
        // request vCard of all the bareJids in roster
        vCardManager().requestVCard(list.at(i));
    }
}

void xmppClient::vCardReceived(const QXmppVCardIq& vCard)
{
    QString bareJid = vCard.from();
    std::cout<<"example_9_vCard:: vCard Received:: " << qPrintable(bareJid) <<std::endl;

    QString out("FullName: %1\nNickName: %2\n");
    std::cout<<qPrintable(out.arg(vCard.fullName()).arg(vCard.nickName())) <<std::endl;

    QString vCardsDir("vCards/");

    QDir dir;
    if(!dir.exists(vCardsDir))
        dir.mkdir(vCardsDir);

    QFile file("vCards/" + bareJid + ".xml");
    if(file.open(QIODevice::ReadWrite))
    {
        QXmlStreamWriter stream(&file);
        vCard.toXml(&stream);
        file.close();
        std::cout<<"example_9_vCard:: vCard written to the file:: " << qPrintable(bareJid) <<std::endl;
    }

    QString name("vCards/" + bareJid + ".png");
    QByteArray photo = vCard.photo();
    QBuffer buffer;
    buffer.setData(photo);
    buffer.open(QIODevice::ReadOnly);
    QImageReader imageReader(&buffer);
    QImage image = imageReader.read();
    if(image.save(name))
    {
        std::cout<<"example_9_vCard:: Avatar saved to file" <<std::endl<<std::endl;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    xmppClient client;
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return a.exec();
}
