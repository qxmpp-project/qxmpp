// SPDX-FileCopyrightText: 2012 Ian Reinhart Geiser <geiseri@kde.org>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "example_3_transferHandling.h"

#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include <cstdio>
#include <cstdlib>

#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent), transferManager(nullptr)
{

    // add transfer manager
    transferManager = new QXmppTransferManager;
    transferManager->setProxy("proxy.qxmpp.org");
    addExtension(transferManager);

    // uncomment one of the following if you only want to use a specific transfer method:
    //
    // transferManager->setSupportedMethods(QXmppTransferJob::InBandMethod);
    // transferManager->setSupportedMethods(QXmppTransferJob::SocksMethod);

    connect(this, &QXmppClient::presenceReceived,
            this, &xmppClient::slotPresenceReceived);

    connect(transferManager, &QXmppTransferManager::fileReceived,
            this, &xmppClient::slotFileReceived);
}

void xmppClient::setRecipient(const QString &recipient)
{
    m_recipient = recipient;
}

/// A file transfer failed.

void xmppClient::slotError(QXmppTransferJob::Error error)
{
    qDebug() << "Transmission failed:" << error;
}

/// A file transfer request was received.

void xmppClient::slotFileReceived(QXmppTransferJob *job)
{
    qDebug() << "Got transfer request from:" << job->jid();

    connect(job, SIGNAL(error(QXmppTransferJob::Error)),
            this, SLOT(slotError(QXmppTransferJob::Error)));

    connect(job, &QXmppTransferJob::finished,
            this, &xmppClient::slotFinished);

    connect(job, &QXmppTransferJob::progress,
            this, &xmppClient::slotProgress);

    // allocate a buffer to receive the file
    auto *buffer = new QBuffer(this);
    buffer->open(QIODevice::WriteOnly);
    job->accept(buffer);
}

/// A file transfer finished.

void xmppClient::slotFinished()
{
    qDebug() << "Transmission finished";
}

/// A presence was received

void xmppClient::slotPresenceReceived(const QXmppPresence &presence)
{
    // if we don't have a recipient, or if the presence is not from the recipient,
    // do nothing
    if (m_recipient.isEmpty() ||
        QXmppUtils::jidToBareJid(presence.from()) != m_recipient ||
        presence.type() != QXmppPresence::Available) {
        return;
    }

    // send the file and connect to the job's signals
    QXmppTransferJob *job = transferManager->sendFile(presence.from(), ":/example_3_transferHandling.cpp", "example source code");

    connect(job, SIGNAL(error(QXmppTransferJob::Error)),
            this, SLOT(slotError(QXmppTransferJob::Error)));
    connect(job, &QXmppTransferJob::finished,
            this, &xmppClient::slotFinished);

    connect(job, &QXmppTransferJob::progress,
            this, &xmppClient::slotProgress);
}

/// A file transfer has made progress.

void xmppClient::slotProgress(qint64 done, qint64 total)
{
    qDebug() << "Transmission progress:" << done << "/" << total;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // we want one argument : "send" or "receive"
    if (argc != 2 || (strcmp(argv[1], "send") && strcmp(argv[1], "receive"))) {
        fprintf(stderr, "Usage: %s send|receive\n", argv[0]);
        return EXIT_FAILURE;
    }

    xmppClient client;
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);
    if (!strcmp(argv[1], "send")) {
        client.setRecipient("qxmpp.test2@qxmpp.org");
        client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");
    } else {
        client.connectToServer("qxmpp.test2@qxmpp.org", "qxmpp456");
    }

    return a.exec();
}
