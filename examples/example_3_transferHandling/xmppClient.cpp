/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *	Ian Reinhart Geiser
 *  Jeremy Lainé
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

#include <QBuffer>
#include <QDebug>

#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "xmppClient.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent), transferManager(0)
{
    bool check;
    Q_UNUSED(check);

    // add transfer manager
    transferManager = new QXmppTransferManager;
    addExtension(transferManager);

    // uncomment one of the following if you only want to use a specific transfer method:
    //
    // transferManager->setSupportedMethods(QXmppTransferJob::InBandMethod);
    // transferManager->setSupportedMethods(QXmppTransferJob::SocksMethod);

    check = connect(this, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SLOT(slotPresenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    check = connect(transferManager, SIGNAL(fileReceived(QXmppTransferJob*)),
                    this, SLOT(slotFileReceived(QXmppTransferJob*)));
    Q_ASSERT(check);
}

/// A file transfer failed.

void xmppClient::slotError(QXmppTransferJob::Error error)
{
    qDebug() << "Transmission failed:" << error;
}

/// A file transfer request was received.

void xmppClient::slotFileReceived(QXmppTransferJob *job)
{
    bool check;
    Q_UNUSED(check);

    qDebug() << "Got transfer request from:" << job->jid();

    check = connect(job, SIGNAL(error(QXmppTransferJob::Error)),
                    this, SLOT(slotError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()),
                    this, SLOT(slotFinished()));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(progress(qint64,qint64)),
                    this, SLOT(slotProgress(qint64,qint64)));
    Q_ASSERT(check);

    // allocate a buffer to receive the file
    QBuffer *buffer = new QBuffer(this);
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
    bool check;
    Q_UNUSED(check);

    const QLatin1String recipient("qxmpp.test2@gmail.com");

    // if we are the recipient, or if the presence is not from the recipient,
    // do nothing
    if (QXmppUtils::jidToBareJid(configuration().jid()) == recipient ||
        QXmppUtils::jidToBareJid(presence.from()) != recipient ||
        presence.type() != QXmppPresence::Available)
        return;

    // send the file and connect to the job's signals
    QXmppTransferJob *job = transferManager->sendFile(presence.from(), "xmppClient.cpp");

    check = connect(job, SIGNAL(error(QXmppTransferJob::Error)),
                    this, SLOT(slotError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()),
                    this, SLOT(slotFinished()));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(progress(qint64,qint64)),
                    this, SLOT(slotProgress(qint64,qint64)));
    Q_ASSERT(check);
}

/// A file transfer has made progress.

void xmppClient::slotProgress(qint64 done, qint64 total)
{
    qDebug() << "Transmission progress:" << done << "/" << total;
}
