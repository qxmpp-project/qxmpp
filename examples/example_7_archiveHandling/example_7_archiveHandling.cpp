// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "example_7_archiveHandling.h"

#include "QXmppArchiveIq.h"
#include "QXmppArchiveManager.h"

#include <QCoreApplication>
#include <QDateTime>

static void logStart(const QString &msg)
{
    qDebug("example_7_archiveHandling : %s", qPrintable(msg));
}

static void logEnd(const QString &msg)
{
    qDebug(" => %s", qPrintable(msg));
}

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent), m_collectionCount(-1), m_pageDirection(PageForwards), m_pageSize(10)
{

    // add archive manager
    archiveManager = new QXmppArchiveManager;
    addExtension(archiveManager);

    // connect signals
    connect(this, &QXmppClient::connected,
            this, &xmppClient::clientConnected);

    connect(archiveManager, &QXmppArchiveManager::archiveChatReceived,
            this, &xmppClient::archiveChatReceived);

    connect(archiveManager, &QXmppArchiveManager::archiveListReceived,
            this, &xmppClient::archiveListReceived);

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
        for (const auto &chat : chats) {
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
    logEnd(QString("chat received, RSM count %1")
               .arg(QString::number(rsmReply.count())));

    const auto messages = chat.messages();
    for (const auto &msg : messages) {
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
