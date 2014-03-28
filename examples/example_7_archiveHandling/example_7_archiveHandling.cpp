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

#include <QCoreApplication>
#include <QDateTime>

#include "QXmppArchiveIq.h"
#include "QXmppArchiveManager.h"

#include "example_7_archiveHandling.h"

static void logStart(const QString &msg)
{
    qDebug("example_7_archiveHandling : %s", qPrintable(msg));
}

static void logEnd(const QString &msg)
{
    qDebug(" => %s", qPrintable(msg));
}

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
    , m_collectionCount(-1)
    , m_pageDirection(PageForwards)
    , m_pageSize(10)
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

    check = connect(archiveManager, SIGNAL(archiveChatReceived(QXmppArchiveChat, QXmppResultSetReply)),
                    SLOT(archiveChatReceived(QXmppArchiveChat, QXmppResultSetReply)));
    Q_ASSERT(check);

    check = connect(archiveManager, SIGNAL(archiveListReceived(QList<QXmppArchiveChat>, QXmppResultSetReply)),
                    SLOT(archiveListReceived(QList<QXmppArchiveChat>, QXmppResultSetReply)));
    Q_ASSERT(check);

    // set limits
    m_startDate = QDateTime::currentDateTime().addDays(-21);
    m_endDate = QDateTime::currentDateTime();
}

xmppClient::~xmppClient()
{

}

void xmppClient::setPageDirection(PageDirection direction)
{
    m_pageDirection = direction;
}

void xmppClient::setPageSize(int size)
{
    m_pageSize = size;
}

void xmppClient::clientConnected()
{
    logEnd("connected");

    // we want 0 results, i.e. only result-set management information (count)
    logStart("fetching collection count");
    QXmppResultSetQuery rsmQuery;
    rsmQuery.setMax(0);
    archiveManager->listCollections("", m_startDate, m_endDate, rsmQuery);
}

void xmppClient::archiveListReceived(const QList<QXmppArchiveChat> &chats, const QXmppResultSetReply &rsmReply)
{
    if (m_collectionCount < 0) {
        logEnd(QString::number(rsmReply.count()) + " items");
        m_collectionCount = rsmReply.count();

        // fetch first page
        logStart("fetching collection first page");
        QXmppResultSetQuery rsmQuery;
        rsmQuery.setMax(m_pageSize);
        if (m_pageDirection == PageBackwards)
            rsmQuery.setBefore("");
        archiveManager->listCollections("", m_startDate, m_endDate, rsmQuery);
    } else if (!chats.size()) {
        logEnd("no items");
    } else {
        logEnd(QString("items %1 to %2 of %3").arg(QString::number(rsmReply.index()), QString::number(rsmReply.index() + chats.size() - 1), QString::number(rsmReply.count())));
        foreach (const QXmppArchiveChat &chat, chats) {
            qDebug("chat start %s", qPrintable(chat.start().toString()));
            // NOTE: to actually retrieve conversations, uncomment this
            //archiveManager->retrieveCollection(chat.with(), chat.start());
        }
        if (!rsmReply.isNull()) {
            // fetch next page
            QXmppResultSetQuery rsmQuery;
            rsmQuery.setMax(m_pageSize);
            if (m_pageDirection == PageBackwards) {
                logStart("fetching collection previous page");
                rsmQuery.setBefore(rsmReply.first());
            } else {
                logStart("fetching collection next page");
                rsmQuery.setAfter(rsmReply.last());
            }
            archiveManager->listCollections("", m_startDate, m_endDate, rsmQuery);
        }
    }
}

void xmppClient::archiveChatReceived(const QXmppArchiveChat &chat, const QXmppResultSetReply &rsmReply)
{
    logEnd(QString("chat received, RSM count %1").arg(QString::number(rsmReply.count())));
    foreach (const QXmppArchiveMessage &msg, chat.messages()) {
        qDebug("example_7_archiveHandling : %s", qPrintable(msg.body()));
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    xmppClient client;
    client.setPageSize(15);
    client.setPageDirection(xmppClient::PageBackwards);
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return a.exec();
}
