/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *	Manjeet Dahiya
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

#include <QCoreApplication>

#include "QXmppClient.h"
#include "QXmppLogger.h"
#include "QXmppRpcManager.h"

#include "remoteinterface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    QXmppClient client;

    // add RPC extension and register interface
    QXmppRpcManager *manager = new QXmppRpcManager;
    client.addExtension(manager);
    manager->addInvokableInterface(new RemoteInterface(&client));

    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");
    return a.exec();
}
