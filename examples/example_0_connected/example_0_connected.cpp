// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppLogger.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QXmppClient client;
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);

    // For jabber
    // client.connectToServer("username@jabber.org", "passwd");

    // For google talk
    // client.connectToServer("username@gmail.com", "passwd");

    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return app.exec();
}
