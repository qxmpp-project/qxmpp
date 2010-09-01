/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include <QtCore/QCoreApplication>
#include "rpcClient.h"
#include "QXmppLogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    QXmppConfiguration config;
    config.setUser("server");
    config.setDomain("geiseri.com");
    config.setPassword("Passw0rd");
    config.setHost("jabber.geiseri.com");
    config.setUseSASLAuthentication( false );

    rpcClient client;
    client.connectToServer(config);
    return a.exec();
}
