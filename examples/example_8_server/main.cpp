/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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
#include "QXmppPasswordChecker.h"
#include "QXmppServer.h"

#define USERNAME "qxmpp.test1"
#define PASSWORD "qxmpp123"

class passwordChecker : public QXmppPasswordChecker
{
    /// Retrieves the password for the given username.
    QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password)
    {
        if (request.username() == USERNAME)
        {
            password = PASSWORD;
            return QXmppPasswordReply::NoError;
        } else {
            return QXmppPasswordReply::AuthorizationError;
        }
    };

    /// Returns true as we implemented getPassword().
    bool hasGetPassword() const
    {
        return true;
    };
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // we want one argument : the domain to serve
    if (argc != 2)
    {
        fprintf(stderr, "Usage: xmppServer <domain>\n");
        return EXIT_FAILURE;
    }
    const QString domain = QString::fromLocal8Bit(argv[1]);

    QXmppLogger logger;
    logger.setLoggingType(QXmppLogger::StdoutLogging);

    passwordChecker checker;

    QXmppServer server;
    server.setDomain(domain);
    server.setLogger(&logger);
    server.setPasswordChecker(&checker);
    server.listenForClients();
    server.listenForServers();
    return a.exec();
}
