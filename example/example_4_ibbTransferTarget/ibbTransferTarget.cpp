/*
 * Copyright (C) 2008-2010 QXmpp Developers
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

#include "ibbTransferTarget.h"

IbbTransferTarget::IbbTransferTarget(QObject *parent)
    : QXmppClient(parent)
{
    m_buffer = new QBuffer(this);

    bool check = connect(&getTransferManager(), SIGNAL(fileReceived(QXmppTransferJob*)),
                    this, SLOT(slotFileReceived(QXmppTransferJob*)));
    Q_ASSERT(check);
    Q_UNUSED(check);

    getTransferManager().setSupportedMethods(QXmppTransferJob::InBandMethod);
}

IbbTransferTarget::~IbbTransferTarget()
{
}

void IbbTransferTarget::slotFileReceived(QXmppTransferJob *job)
{
    qDebug() << "Got transfer request from:" << job->jid();

    bool check = connect(job, SIGNAL(error(QXmppTransferJob::Error)), this, SLOT(slotError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()), this, SLOT(slotFinished()));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(progress(qint64,qint64)), this, SLOT(slotProgress(qint64,qint64)));
    Q_ASSERT(check);

    m_buffer->open(QIODevice::WriteOnly);
    job->accept(m_buffer);
}

void IbbTransferTarget::slotError(QXmppTransferJob::Error error)
{
    qDebug() << "Transmission failed:" << error;
}

void IbbTransferTarget::slotFinished()
{
    qDebug() << "Transfer finished:" << m_buffer->data();
}

void IbbTransferTarget::slotProgress(qint64 done, qint64 total)
{
     qDebug() << "Transfer progress:" << done << "/" << total;
}
