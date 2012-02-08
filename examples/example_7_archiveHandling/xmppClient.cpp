/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#include <QDateTime>
#include <iostream>

#include "QXmppArchiveIq.h"
#include "QXmppArchiveManager.h"

#include "xmppClient.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check;
    Q_UNUSED(check);

    // add archive manager
    archiveManager = new QXmppArchiveManager;
    addExtension(archiveManager);

    // connect signals
    check = connect(this, SIGNAL(connected()),
                    this, SLOT(clientConnected()));
    Q_ASSERT(check);

    check = connect(archiveManager, SIGNAL(archiveChatReceived(QXmppArchiveChat)),
                    SLOT(archiveChatReceived(QXmppArchiveChat)));
    Q_ASSERT(check);

    check = connect(archiveManager, SIGNAL(archiveListReceived(QList<QXmppArchiveChat>)),
                    SLOT(archiveListReceived(QList<QXmppArchiveChat>)));
    Q_ASSERT(check);
}

xmppClient::~xmppClient()
{

}

void xmppClient::clientConnected()
{
    std::cout << "example_7_archiveHandling:: CONNECTED" << std::endl;
    archiveManager->listCollections("",
            QDateTime::currentDateTime().addDays(-7));
}

void xmppClient::archiveListReceived(const QList<QXmppArchiveChat> &chats)
{
    std::cout << "example_7_archiveHandling:: LIST RECEIVED" << std::endl;
    foreach (const QXmppArchiveChat &chat, chats)
        archiveManager->retrieveCollection(chat.with(), chat.start());
}

void xmppClient::archiveChatReceived(const QXmppArchiveChat &chat)
{
    std::cout << "example_7_archiveHandling:: CHAT RECEIVED" << std::endl;
    foreach (const QXmppArchiveMessage &msg, chat.messages())
    {
        std::cout << "example_7_archiveHandling::" << msg.body().toStdString() << std::endl;
    }
}

