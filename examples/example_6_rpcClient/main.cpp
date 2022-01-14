// SPDX-FileCopyrightText: 2010 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppLogger.h"

#include "rpcClient.h"
#include <QtCore/QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    rpcClient client;
    client.connectToServer("qxmpp.test2@qxmpp.org", "qxmpp456");
    return a.exec();
}
