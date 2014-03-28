/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
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


#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include <QDateTime>

#include "QXmppClient.h"

class QXmppArchiveChat;
class QXmppArchiveManager;
class QXmppResultSetReply;

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    enum PageDirection {
        PageForwards = 0,
        PageBackwards
    };

    xmppClient(QObject *parent = 0);
    ~xmppClient();

    void setPageDirection(PageDirection direction);
    void setPageSize(int size);

public slots:
    void clientConnected();
    void archiveListReceived(const QList<QXmppArchiveChat> &chats, const QXmppResultSetReply &rsmReply);
    void archiveChatReceived(const QXmppArchiveChat &chat, const QXmppResultSetReply &rsmReply);

private:
    QXmppArchiveManager *archiveManager;
    int m_collectionCount;
    QDateTime m_startDate;
    QDateTime m_endDate;
    PageDirection m_pageDirection;
    int m_pageSize;
};

#endif // XMPPCLIENT_H
