/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
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

#include "QXmppLogger.h"
#include "QXmppMessage.h"

#include "example_1_echoClient.h"

echoClient::echoClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check = connect(this, SIGNAL(messageReceived(QXmppMessage)),
        SLOT(messageReceived(QXmppMessage)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

echoClient::~echoClient()
{

}

void echoClient::messageReceived(const QXmppMessage& message)
{
    QString from = message.from();
    QString msg = message.body();

    sendPacket(QXmppMessage("", from, "Your message: " + msg));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    echoClient client;
    client.logger()->setLoggingType(QXmppLogger::StdoutLogging);
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return app.exec();
}
