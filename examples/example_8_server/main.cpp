/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
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

#include "QXmppLogger.h"
#include "QXmppIncomingClient.h"
#include "QXmppServer.h"

#define USERNAME "qxmpp.test1"
#define PASSWORD "qxmpp123"

class passwordChecker : public QXmppPasswordChecker
{
    /// Checks that the given credentials are valid.
    bool check(const QString &username, const QString &password)
    {
        return (username == USERNAME && password == PASSWORD);
    };

    /// Retrieves the password for the given username.
    bool get(const QString &username, QString &password)
    {
        if (username == USERNAME)
        {
            password = PASSWORD;
            return true;
        } else {
            return false;
        }
    };
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);
    
    passwordChecker checker;

    QXmppServer server;
    server.setDomain("example.com");
    server.setLogger(&logger);
    server.setPasswordChecker(&checker);
    server.listenForClients();
    return a.exec();
}
