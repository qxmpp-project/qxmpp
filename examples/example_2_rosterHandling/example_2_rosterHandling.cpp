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

#include "QXmppMessage.h"
#include "QXmppRosterManager.h"

#include "example_2_rosterHandling.h"

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check;
    Q_UNUSED(check);

    check = connect(this, SIGNAL(connected()),
                    SLOT(clientConnected()));
    Q_ASSERT(check);

    check = connect(&this->rosterManager(), SIGNAL(rosterReceived()),
                    SLOT(rosterReceived()));
    Q_ASSERT(check);

    /// Then QXmppRoster::presenceChanged() is emitted whenever presence of someone
    /// in roster changes
    check = connect(&this->rosterManager(), SIGNAL(presenceChanged(QString,QString)),
                    SLOT(presenceChanged(QString,QString)));
    Q_ASSERT(check);
}

xmppClient::~xmppClient()
{

}

void xmppClient::clientConnected()
{
    qDebug("example_2_rosterHandling:: CONNECTED");
}

void xmppClient::rosterReceived()
{
    qDebug("example_2_rosterHandling:: Roster received");
    foreach (const QString &bareJid, rosterManager().getRosterBareJids()) {
        QString name = rosterManager().getRosterEntry(bareJid).name();
        if(name.isEmpty())
            name = "-";
        qDebug("example_2_rosterHandling:: Roster received: %s [%s]", qPrintable(bareJid), qPrintable(name));
    }
}

void xmppClient::presenceChanged(const QString& bareJid,
                                 const QString& resource)
{
    qDebug("example_2_rosterHandling:: Presence changed %s/%s", qPrintable(bareJid), qPrintable(resource));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    xmppClient client;
    client.connectToServer("qxmpp.test1@qxmpp.org", "qxmpp123");

    return app.exec();
}
