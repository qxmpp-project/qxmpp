/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "capabilitiesCollection.h"

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include <QXmlStreamWriter>

capabilitiesCollection::capabilitiesCollection(QXmppClient* client) :
    QObject(client), m_client(client)
{
    QXmppDiscoveryManager* ext = m_client->findExtension<QXmppDiscoveryManager>();
    if(ext)
    {
        bool check = connect(ext, SIGNAL(infoReceived(const QXmppDiscoveryIq&)),
                             SLOT(infoReceived(const QXmppDiscoveryIq&)));
        Q_ASSERT(check);
    }
}

bool capabilitiesCollection::isCapabilityAvailable(const QString& nodeVer)
{
    return m_mapCapabilities.contains(nodeVer);
}

void capabilitiesCollection::requestInfo(const QString& jid, const QString& node)
{
    QXmppDiscoveryManager* ext = m_client->findExtension<QXmppDiscoveryManager>();
    if(ext)
    {
        ext->requestInfo(jid, node);
    }
}

void capabilitiesCollection::infoReceived(const QXmppDiscoveryIq& discoIqRcv)
{
    QXmppDiscoveryIq discoIq = discoIqRcv;
    if(discoIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
       discoIq.type() == QXmppIq::Result)
    {
        if(!discoIq.queryNode().isEmpty())
        {
            discoIq.setTo("");
            discoIq.setFrom("");
            discoIq.setId("");
            m_mapCapabilities[discoIq.queryNode()] = discoIq;
        }
    }
}
