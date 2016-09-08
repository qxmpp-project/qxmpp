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

#include "QXmppCarbonManager.h"
#include "QXmppConstants_p.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"


QXmppCarbonManager::QXmppCarbonManager()
    : m_carbonsEnabled(false)
{

}

QXmppCarbonManager::~QXmppCarbonManager()
{

}

/// Returns whether message carbons are currently enabled

bool QXmppCarbonManager::carbonsEnabled() const
{
    return m_carbonsEnabled;
}

/// Enable or disable message carbons.
/// This function does not check whether the server supports
/// message carbons, but just sends the corresponding stanza
/// to the server, so one must check in advance by using the
/// discovery manager.
///
/// By default, carbon copies are disabled.

void QXmppCarbonManager::setCarbonsEnabled(bool enabled)
{
    if(m_carbonsEnabled == enabled)
        return;

    m_carbonsEnabled = enabled;

    if(client()) {
        QXmppIq iq(QXmppIq::Set);
        QXmppElement carbonselement;
        carbonselement.setTagName(m_carbonsEnabled ? "enable" : "disable");
        carbonselement.setAttribute("xmlns", ns_carbons);

        iq.setExtensions(QXmppElementList() << carbonselement);
        client()->sendPacket(iq);
    }
}

QStringList QXmppCarbonManager::discoveryFeatures() const
{
    return QStringList() << ns_carbons;
}

bool QXmppCarbonManager::handleStanza(const QDomElement &element)
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
