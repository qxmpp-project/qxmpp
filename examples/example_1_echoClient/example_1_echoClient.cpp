// SPDX-FileCopyrightText: 2012 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "example_1_echoClient.h"

#include "QXmppLogger.h"
#include "QXmppMessage.h"

#include <QCoreApplication>

echoClient::echoClient(QObject *parent)
    : QXmppClient(parent)
{
    connect(this, &QXmppClient::messageReceived, this, &echoClient::messageReceived);
}

echoClient::~echoClient()
{
}

void echoClient::messageReceived(const QXmppMessage &message)
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
