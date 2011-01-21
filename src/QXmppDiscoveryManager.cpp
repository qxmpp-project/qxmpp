/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "QXmppDiscoveryManager.h"

#include <QDomElement>
#include <QCoreApplication>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppStream.h"
#include "QXmppGlobal.h"

QXmppDiscoveryManager::QXmppDiscoveryManager() : QXmppClientExtension(),
    m_clientCapabilitiesNode("http://code.google.com/p/qxmpp"),
    m_clientCategory("client"),
    m_clientType("pc"),
    m_clientName(QString("%1 %2").arg(qApp->applicationName(), qApp->applicationVersion()))
{
    if(qApp->applicationName().isEmpty() && qApp->applicationVersion().isEmpty())
    {
        m_clientName = QString("%1 %2").arg("Based on QXmpp", QXmppVersion());
    }
}

bool QXmppDiscoveryManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element))
    {
        QXmppDiscoveryIq receivedIq;
        receivedIq.parse(element);

        if(receivedIq.type() == QXmppIq::Get &&
           receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
           (receivedIq.queryNode().isEmpty() || receivedIq.queryNode().startsWith(m_clientCapabilitiesNode)))
        {
            // respond to query
            QXmppDiscoveryIq qxmppFeatures = capabilities();
            qxmppFeatures.setId(receivedIq.id());
            qxmppFeatures.setTo(receivedIq.from());
            qxmppFeatures.setQueryNode(receivedIq.queryNode());
            client()->sendPacket(qxmppFeatures);
        }
        else if(receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery)
            emit infoReceived(receivedIq);
        else if(receivedIq.queryType() == QXmppDiscoveryIq::ItemsQuery)
            emit itemsReceived(receivedIq);

        return true;
    }
    return false;
}

/// Requests information from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).

QString QXmppDiscoveryManager::requestInfo(const QString& jid, const QString& node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(jid);
    if(!node.isEmpty())
        request.setQueryNode(node);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

/// Requests items from the specified XMPP entity.
///
/// \param jid  The target entity's JID.
/// \param node The target node (optional).

QString QXmppDiscoveryManager::requestItems(const QString& jid, const QString& node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setTo(jid);
    if(!node.isEmpty())
        request.setQueryNode(node);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

QStringList QXmppDiscoveryManager::discoveryFeatures() const
{
    return QStringList() << ns_disco_info;
}

QXmppDiscoveryIq QXmppDiscoveryManager::capabilities()
{
    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);

    // features
    QStringList features;
    features
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_capabilities      // XEP-0115 : Entity Capabilities
        << ns_ping;             // XEP-0199: XMPP Ping

    foreach(QXmppClientExtension* extension, client()->extensions())
    {
        if(extension)
            features << extension->discoveryFeatures();
    }

    iq.setFeatures(features);

    // identities
    QList<QXmppDiscoveryIq::Identity> identities;

    QXmppDiscoveryIq::Identity identity;
    identity.setCategory(clientCategory());
    identity.setType(clientType());
    identity.setName(clientName());
    identities << identity;

    foreach(QXmppClientExtension* extension, client()->extensions())
    {
        if(extension)
            identities << extension->discoveryIdentities();
    }

    iq.setIdentities(identities);
    return iq;
}

/// Sets the capabilities node of the local XMPP client.
///
/// \param node

void QXmppDiscoveryManager::setClientCapabilitiesNode(const QString &node)
{
    m_clientCapabilitiesNode = node;
}

/// Sets the category of the local XMPP client.
///
/// You can find a list of valid categories at:
/// http://xmpp.org/registrar/disco-categories.html
///
/// \param category

void QXmppDiscoveryManager::setClientCategory(const QString& category)
{
    m_clientCategory = category;
}

/// Sets the type of the local XMPP client.
///
/// You can find a list of valid types at:
/// http://xmpp.org/registrar/disco-categories.html
///
/// \param type

void QXmppDiscoveryManager::setClientType(const QString& type)
{
    m_clientType = type;
}

/// Sets the name of the local XMPP client.
///
/// \param name

void QXmppDiscoveryManager::setClientName(const QString& name)
{
    m_clientName = name;
}

/// Returns the capabilities node of the local XMPP client.
///
/// By default this is "http://code.google.com/p/qxmpp".

QString QXmppDiscoveryManager::clientCapabilitiesNode() const
{
    return m_clientCapabilitiesNode;
}

/// Returns the category of the local XMPP client.
///
/// By default this is "client".

QString QXmppDiscoveryManager::clientCategory() const
{
    return m_clientCategory;
}

/// Returns the type of the local XMPP client.
///
/// By default this is "pc".

QString QXmppDiscoveryManager::clientType() const
{
    return m_clientType;
}

/// Returns the name of the local XMPP client.
///
/// By default this is "Based on QXmpp x.y.z".

QString QXmppDiscoveryManager::clientName() const
{
    return m_clientName;
}
