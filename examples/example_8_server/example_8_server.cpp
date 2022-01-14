// SPDX-FileCopyrightText: 2019 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppLogger.h"
#include "QXmppPasswordChecker.h"
#include "QXmppServer.h"

#include <QtCore/QCoreApplication>

#define USERNAME "qxmpp.test1"
#define PASSWORD "qxmpp123"

class passwordChecker : public QXmppPasswordChecker
{
    /// Retrieves the password for the given username.
    QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest &request, QString &password) override
    {
        if (request.username() == USERNAME) {
            password = PASSWORD;
            return QXmppPasswordReply::NoError;
        } else {
            return QXmppPasswordReply::AuthorizationError;
        }
    };

    /// Returns true as we implemented getPassword().
    bool hasGetPassword() const override
    {
        return true;
    };
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // we want one argument : the domain to serve
    if (argc != 2) {
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
