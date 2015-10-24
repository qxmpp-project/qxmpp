/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#include <QDomElement>

#include "QXmppCarbonsManager.h"
#include "QXmppConstants.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMessage.h"


QXmppCarbonsManager::QXmppCarbonsManager()
    : m_carbonsEnabled(false), m_carbonsSupported(false)
{

}

QXmppCarbonsManager::~QXmppCarbonsManager()
{

}

/// Returns whether message carbons are currently enabled

bool QXmppCarbonsManager::carbonsEnabled() const
{
    return m_carbonsEnabled;
}

/// Enable or disable message carbons

void QXmppCarbonsManager::setCarbonsEnabled(bool enabled)
{
    if(m_carbonsEnabled == enabled)
        return;

    m_carbonsEnabled = enabled;

    if(m_carbonsSupported && client()) {
        QXmppIq iq(QXmppIq::Set);
        QXmppElement carbonselement;
        carbonselement.setTagName(m_carbonsEnabled ? "enable" : "disable");
        carbonselement.setAttribute("xmlns", ns_carbons);

        iq.setExtensions(QXmppElementList() << carbonselement);
        client()->sendPacket(iq);
    }
}

QStringList QXmppCarbonsManager::discoveryFeatures() const
{
    return QStringList() << ns_carbons;
}

bool QXmppCarbonsManager::handleStanza(const QDomElement &element)
{
    if(element.tagName() != "message")
        return false;

    bool sent = true;
    QDomElement carbon = element.firstChildElement("sent");
    if(carbon.isNull()) {
        carbon = element.firstChildElement("received");
        sent = false;
    }

    if(carbon.isNull() || carbon.namespaceURI() != ns_carbons)
        return false;   // Neither sent nor received -> no carbon message

    QDomElement forwarded = carbon.firstChildElement("forwarded");
    if(forwarded.isNull())
        return false;

    QDomElement messageelement = forwarded.firstChildElement("message");
    if(messageelement.isNull())
        return false;

    QXmppMessage message;
    message.parse(messageelement);

    if(sent)
        emit messageSent(message);
    else
        emit messageReceived(message);

    return true;
}

void QXmppCarbonsManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);

    // Connect to discovery manager
    QXmppDiscoveryManager* discoManager = client->findExtension<QXmppDiscoveryManager>();
    if(!discoManager)
        warning("QXmppCarbonsManager: could not find QXmppDiscoveryManager, not able to determine server support for message carbons.");
    else {
        bool check = connect(discoManager, SIGNAL(infoReceived(const QXmppDiscoveryIq&)),
                        this, SLOT(_q_infoReceived(const QXmppDiscoveryIq&)));
        Q_ASSERT(check);
    }


}

void QXmppCarbonsManager::_q_infoReceived(const QXmppDiscoveryIq& infoiq)
{
     if(!m_carbonsSupported && infoiq.features().contains(ns_carbons)) {
         m_carbonsSupported = true;

         // if the user previously enabled carbons,
         // but server support was not yet available, enable them again:
         if(m_carbonsEnabled) {
             m_carbonsEnabled = false; // otherwise, setCarbonsEnabled would not have any effect
             setCarbonsEnabled(true);
         }
     }
}
