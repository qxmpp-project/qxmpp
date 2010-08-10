/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "rpcClient.h"
#include "QXmppRemoteMethod.h"
#include <qdebug.h>
#include <QTimer>

rpcClient::rpcClient(QObject *parent)
    : QXmppClient(parent)
{
    connect( this, SIGNAL(connected()), this, SLOT(isConnected()));
}

rpcClient::~rpcClient()
{

}

void rpcClient::isConnected()
{
    //We need to wait until we have sent the presense stuff, or for some
    //reason the server ignores us...
    QTimer::singleShot(5000, this, SLOT(invokeRemoteMethod()));
}

void rpcClient::invokeRemoteMethod()
{
    QXmppRemoteMethodResult methodResult = callRemoteMethod(
            "server@geiseri.com/QXmpp", "RemoteInterface.echoString", "This is a test" );
    if( methodResult.hasError )
        error( methodResult.code, methodResult.errorMessage );
    else
        result( methodResult.result );
}

void rpcClient::result(const QVariant &value )
{
    qDebug() << "Result:" << value;
}

void rpcClient::error( int code, const QString &message )
{
    qDebug() << "Error:" << code << message;
}
