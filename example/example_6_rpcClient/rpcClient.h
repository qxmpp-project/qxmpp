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


#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "QXmppClient.h"

class rpcClient : public QXmppClient
{
    Q_OBJECT

public:
    rpcClient(QObject *parent = 0);
    ~rpcClient();

public slots:
    void isConnected();
    void invokeRemoteMethod();
    void result(const QVariant &value );
    void error( int code, const QString &message );

};

#endif // RPCCLIENT_H
