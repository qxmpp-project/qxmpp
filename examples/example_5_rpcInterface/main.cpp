// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppLogger.h"
#include "QXmppRpcManager.h"

#include "remoteinterface.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    QXmppClient client;

    // add RPC extension and register interface
    auto *manager = new QXmppRpcManager;
    client.addExtension(manager);
    manager->addInvokableInterface(new RemoteInterface(&client));

    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");
    return a.exec();
}
