/*
 * Copyright (C) 2008-2009 QXmpp Developers
 *
 * Author:
 *	Ian Reinhart Geiser
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


#include "ibbTransferTarget.h"
#include "QXmppMessage.h"
#include "QXmppIbbTransferManager.h"
#include <QBuffer>
#include <qdebug.h>

IbbTransferTarget::IbbTransferTarget(QObject *parent)
    : QXmppClient(parent)
{
    bool check = connect(getIbbTransferManager(), SIGNAL(byteStreamRequestReceived(QString,QString)),
                    this, SLOT(openReceived(QString,QString)));
    Q_ASSERT(check);

    check = connect(getIbbTransferManager(), SIGNAL(byteStreamClosed(QString,QString)),
                    this, SLOT(closeReceived(QString,QString)));
    Q_ASSERT(check);

    m_buffer = new QBuffer(this);
    m_buffer->open(QIODevice::WriteOnly);
}

IbbTransferTarget::~IbbTransferTarget()
{
}

void IbbTransferTarget::openReceived( const QString &sid, const QString& from)
{
    qDebug() << "Got open byte stream request from" << from;
    getIbbTransferManager()->acceptByteStreamRequest(sid, m_buffer);
}

void IbbTransferTarget::closeReceived( const QString& sid, const QString& reason)
{
     qDebug() << "Stream done:" << m_buffer->data();
}
