/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#include "ibbClient.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include "QXmppIbbTransferManager.h"
#include <QBuffer>
#include <qdebug.h>

ibbClient::ibbClient(QObject *parent)
    : QXmppClient(parent)
{
    m_buffer = new QBuffer(this);
    QByteArray bytes;
    for( int idx = 0; idx < 1000; idx++ )
        bytes += generateStanzaHash().toLatin1();

    m_buffer->setData( bytes );
    m_buffer->open(QIODevice::ReadOnly);

    connect( this, SIGNAL(connected()),
             this, SLOT(slotConnected()));
    connect( getIbbTransferManager(), SIGNAL(byteStreamCanceled(QString,QString)),
             this, SLOT( slotByteStreamCanceled(QString,QString)) );
    connect( getIbbTransferManager(), SIGNAL(byteStreamClosed(QString,QString)),
             this, SLOT( slotByteStreamClosed(QString,QString)) );
    connect( getIbbTransferManager(), SIGNAL( byteStreamOpened(QString)),
             this, SLOT( slotByteStreamOpened(QString)) );
    connect( getIbbTransferManager(), SIGNAL( byteStreamRequestReceived(QString,QString)),
             this, SLOT( slotByteStreamRequestReceived(QString,QString)) );
}

ibbClient::~ibbClient()
{

}

void ibbClient::slotConnected()
{
    getIbbTransferManager()->sendByteStreamRequest( generateStanzaHash(), "client@geiseri.com/QXmpp", m_buffer );
}


void ibbClient::slotByteStreamRequestReceived( const QString &sid, const QString &remoteJid )
{
    qDebug() << "Remote JID:" << remoteJid << " asked for transfer";
    getIbbTransferManager()->acceptByteStreamRequest( sid, m_buffer );
}

void ibbClient::slotByteStreamClosed( const QString &sid , const QString &reason )
{
    qDebug() << "Transmission done" << m_buffer->buffer();
}

void ibbClient::slotByteStreamCanceled( const QString &sid , const QString &reason )
{
    qDebug() << "Transmission canceled" << reason;
}

void ibbClient::slotByteStreamOpened( const QString &sid )
{
    qDebug() << "Bytestream opened";
}
