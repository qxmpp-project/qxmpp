/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#include "xmppClient.h"
#include "QXmppMessage.h"
#include "QXmppRoster.h"
#include <iostream>

xmppClient::xmppClient(QObject *parent)
    : QXmppClient(parent)
{
    bool check = connect(this, SIGNAL(connected()),
        SLOT(clientConnected()));
    Q_ASSERT(check);

    check = connect(&this->getRoster(), SIGNAL(rosterReceived()),
        SLOT(rosterReceived()));
    Q_ASSERT(check);

    /// Then QXmppRoster::presenceChanged() is emitted whenever presence of someone
    /// in roster changes
    check = connect(&this->getRoster(),
                    SIGNAL(presenceChanged(const QString&, const QString&)),
        SLOT(presenceChanged(const QString&, const QString&)));
    Q_ASSERT(check);
}

xmppClient::~xmppClient()
{

}

void xmppClient::clientConnected()
{
    std::cout<<"example_2_rosterHandling:: CONNECTED"<<std::endl;
}

void xmppClient::rosterReceived()
{
    std::cout<<"example_2_rosterHandling:: Roster Received"<<std::endl;
    QStringList list = getRoster().getRosterBareJids();
    for(int i = 0; i < list.size(); ++i)
    {
        std::cout<<"Roster Received:: "<<qPrintable(list.at(i))<<std::endl;
    }
}

void xmppClient::presenceChanged(const QString& bareJid,
                                 const QString& resource)
{
    std::cout<<"Presence changed:: "<< qPrintable(bareJid)
            << qPrintable(resource)<<std::endl;
}
