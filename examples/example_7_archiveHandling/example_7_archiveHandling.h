// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include "QXmppArchiveIq.h"
#include "QXmppClient.h"

#include <QDateTime>

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

    xmppClient(QObject *parent = nullptr);
    ~xmppClient() override;

    void setPageDirection(PageDirection direction);
    void setPageSize(int size);

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

#endif  // XMPPCLIENT_H
