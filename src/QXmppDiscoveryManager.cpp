/*
 * Copyright (C) 2008-2010 The QXmpp developers
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
    m_clientCategory("client"),
    m_clientType("pc"),
    m_clientName(QString("%1 %2").arg(qApp->applicationName(), qApp->applicationVersion()))
{
    if(qApp->applicationName().isEmpty() && qApp->applicationVersion().isEmpty())
    {
        m_clientName = QString("%1 %2").arg("Based on QXmpp", QXmppVersion());
    }
}

bool QXmppDiscoveryManager::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element))
    {
        QXmppDiscoveryIq receivedIq;
        receivedIq.parse(element);

        if(receivedIq.type() == QXmppIq::Get &&
           receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
           (receivedIq.queryNode().isEmpty() || receivedIq.queryNode().startsWith(QString(capabilities_node))))
        {
            // respond to query
            QXmppDiscoveryIq qxmppFeatures = capabilities();
            qxmppFeatures.setId(receivedIq.id());
            qxmppFeatures.setTo(receivedIq.from());
            qxmppFeatures.setQueryNode(receivedIq.queryNode());
            stream->sendPacket(qxmppFeatures);
        }
        else if(receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery)
            emit infoReceived(receivedIq);
        else if(receivedIq.queryType() == QXmppDiscoveryIq::ItemsQuery)
            emit itemsReceived(receivedIq);

        return true;
    }
    return false;
}

QString QXmppDiscoveryManager::requestInfo(const QString& jid, const QString& node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(jid);
    request.setFrom(client()->configuration().jid());
    if(!node.isEmpty())
        request.setQueryNode(node);
    if(client()->sendPacket(request))
        return request.id();
    else
        return "";
}

QString QXmppDiscoveryManager::requestItems(const QString& jid, const QString& node)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setTo(jid);
    request.setFrom(client()->configuration().jid());
    if(!node.isEmpty())
        request.setQueryNode(node);
    if(client()->sendPacket(request))
        return request.id();
    else
        return "";
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
        << ns_rpc               // XEP-0009: Jabber-RPC
//        << ns_vcard             // XEP-0054: vcard-temp
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_capabilities      // XEP-0115 : Entity Capabilities
        << ns_ping;             // XEP-0199: XMPP Ping

    foreach(QXmppClientExtension* extension, client()->extensions())
    {
        if(extension)
            features << extension->discoveryFeatures();
    }

    iq.setFeatures(features);

    // TODO: get identities from the extensions itself like the features
    // identities
    QList<QXmppDiscoveryIq::Identity> identities;
    QXmppDiscoveryIq::Identity identity;

    identity.setCategory("automation");
    identity.setType("rpc");
    identities.append(identity);

    identity.setCategory(clientCategory());
    identity.setType(clientType());
    identity.setName(clientName());
    identities.append(identity);

    iq.setIdentities(identities);
    return iq;
}

/// http://xmpp.org/registrar/disco-categories.html#client
void QXmppDiscoveryManager::setClientCategory(const QString& category)
{
    m_clientCategory = category;
}

void QXmppDiscoveryManager::setClientType(const QString& type)
{
    m_clientType = type;
}

void QXmppDiscoveryManager::setClientName(const QString& name)
{
    m_clientName = name;
}

QString QXmppDiscoveryManager::clientCategory()
{
    return m_clientCategory;
}

QString QXmppDiscoveryManager::clientType()
{
    return m_clientType;
}

QString QXmppDiscoveryManager::clientName()
{
    return m_clientName;
}
