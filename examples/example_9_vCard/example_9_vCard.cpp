/*
 * Copyright (C) 2008-2019 The QXmpp developers
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

#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>
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
    : QXmppClient(parent),
      m_rosterManager(findExtension<QXmppRosterManager>()),
      m_vCardManager(findExtension<QXmppVCardManager>())
{
    connect(this, &QXmppClient::connected, this, &xmppClient::clientConnected);

    connect(m_rosterManager, &QXmppRosterManager::rosterReceived,
            this, &xmppClient::rosterReceived);
}

xmppClient::~xmppClient() = default;

void xmppClient::clientConnected()
{
    qDebug() << "example_9_vCard: CONNECTED";
}

void xmppClient::rosterReceived()
{
    qDebug() << "example_9_vCard: Roster Received";

    connect(m_vCardManager, &QXmppVCardManager::vCardReceived,
            this, &xmppClient::vCardReceived);

    // request vCard of all the bareJids in roster
    const QStringList jids = m_rosterManager->getRosterBareJids();
    for (const auto &jid : jids)
        m_vCardManager->requestVCard(jid);
}

void xmppClient::vCardReceived(const QXmppVCardIq& vCard)
{
    QString bareJid = vCard.from();
    qDebug() << "example_9_vCard: vCard Received:" << bareJid;

    qDebug() << "FullName:" << vCard.fullName();
    qDebug() << "Nickname:" << vCard.nickName();

    QString vCardsDir("vCards/");

    QDir dir;
    if(!dir.exists(vCardsDir))
        dir.mkdir(vCardsDir);

    QFile file("vCards/" + bareJid + ".xml");
    if (file.open(QIODevice::ReadWrite)) {
        QXmlStreamWriter stream(&file);
        vCard.toXml(&stream);
        file.close();
        qDebug() << "example_9_vCard: vCard written to the file:" << bareJid;
    }

    QString name("vCards/" + bareJid + ".png");
    QByteArray photo = vCard.photo();
    QBuffer buffer;
    buffer.setData(photo);
    buffer.open(QIODevice::ReadOnly);
    QImageReader imageReader(&buffer);
    QImage image = imageReader.read();
    if (image.save(name))
        qDebug() << "example_9_vCard: Avatar saved to file";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    xmppClient client;
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return a.exec();
}
