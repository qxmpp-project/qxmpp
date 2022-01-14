// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "example_9_vCard.h"

#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QXmlStreamWriter>

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

void xmppClient::vCardReceived(const QXmppVCardIq &vCard)
{
    QString bareJid = vCard.from();
    qDebug() << "example_9_vCard: vCard Received:" << bareJid;

    qDebug() << "FullName:" << vCard.fullName();
    qDebug() << "Nickname:" << vCard.nickName();

    QString vCardsDir("vCards/");

    QDir dir;
    if (!dir.exists(vCardsDir))
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
