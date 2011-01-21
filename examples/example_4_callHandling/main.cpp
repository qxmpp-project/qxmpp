/*
 * Copyright (C) 2008-2011 The QXmpp developers
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

#include <cstdlib>
#include <cstdio>

#include <QCoreApplication>

#include "QXmppLogger.h"
#include "xmppClient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    // we want one argument : "send" or "receive"
    if (argc != 2 || (strcmp(argv[1], "send") && strcmp(argv[1], "receive")))
    {
        fprintf(stderr, "Usage: callClient send|receive\n");
        return EXIT_FAILURE;
    }
    const QString username = (!strcmp(argv[1], "send")) ? QLatin1String("qxmpp.test1") : QLatin1String("qxmpp.test2");

    xmppClient client;
    client.connectToServer(username + "@gmail.com", "qxmpp123");
    return a.exec();
}
