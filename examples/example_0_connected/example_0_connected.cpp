/*
 * Copyright (C) 2008-2012 The QXmpp developers
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


#include <QCoreApplication>

#include "QXmppClient.h"
#include "QXmppLogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QXmppClient client;
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);

    // For jabber
    // client.connectToServer("username@jabber.org", "passwd");

    // For google talk
    // client.connectToServer("qxmpp.test1@gmail.com", "passwd");

    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return app.exec();
}
