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

#include "QXmppDiscoveryManager.h"

#include <QDomElement>
#include <QCoreApplication>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppDataForm.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppStream.h"
#include "QXmppGlobal.h"

class QXmppDiscoveryManagerPrivate
{
public:
    QString clientCapabilitiesNode;
    QString clientCategory;
    QString clientType;
    QString clientName;
    QXmppDataForm clientInfoForm;
};

QXmppDiscoveryManager::QXmppDiscoveryManager()
    : d(new QXmppDiscoveryManagerPrivate)
{
    d->clientCapabilitiesNode = "https://github.com/qxmpp-project/qxmpp";
    d->clientCategory = "client";
    d->clientType = "pc";
    if (qApp->applicationName().isEmpty() && qApp->applicationVersion().isEmpty())
        d->clientName = QString("%1 %2").arg("Based on QXmpp", QXmppVersion());
    else
        d->clientName = QString("%1 %2").arg(qApp->applicationName(), qApp->applicationVersion());
}

QXmppDiscoveryManager::~QXmppDiscoveryManager()
{
    delete d;
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

/// Returns the client's full capabilities.

QXmppDiscoveryIq QXmppDiscoveryManager::capabilities()
{
    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);

    // features
    QStringList features;
    features
        << ns_data              // XEP-0004: Data Forms
        << ns_rsm               // XEP-0059: Result Set Management
        << ns_xhtml_im          // XEP-0071: XHTML-IM
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_capabilities      // XEP-0115: Entity Capabilities
        << ns_ping              // XEP-0199: XMPP Ping
        << ns_attention         // XEP-0224: Attention
        << ns_chat_markers;     // XEP-0333: Chat Markers

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

    // extended information
    if (!d->clientInfoForm.isNull())
        iq.setForm(d->clientInfoForm);

    return iq;
}

/// Sets the capabilities node of the local XMPP client.
///
/// \param node

void QXmppDiscoveryManager::setClientCapabilitiesNode(const QString &node)
{
    d->clientCapabilitiesNode = node;
}

/// Sets the category of the local XMPP client.
///
/// You can find a list of valid categories at:
/// http://xmpp.org/registrar/disco-categories.html
///
/// \param category

void QXmppDiscoveryManager::setClientCategory(const QString& category)
{
    d->clientCategory = category;
}

/// Sets the type of the local XMPP client.
///
/// You can find a list of valid types at:
/// http://xmpp.org/registrar/disco-categories.html
///
/// \param type

void QXmppDiscoveryManager::setClientType(const QString& type)
{
    d->clientType = type;
}

/// Sets the name of the local XMPP client.
///
/// \param name

void QXmppDiscoveryManager::setClientName(const QString& name)
{
    d->clientName = name;
}

/// Returns the capabilities node of the local XMPP client.
///
/// By default this is "https://github.com/qxmpp-project/qxmpp".

QString QXmppDiscoveryManager::clientCapabilitiesNode() const
{
    return d->clientCapabilitiesNode;
}

/// Returns the category of the local XMPP client.
///
/// By default this is "client".

QString QXmppDiscoveryManager::clientCategory() const
{
    return d->clientCategory;
}

/// Returns the type of the local XMPP client.
///
/// By default this is "pc".

QString QXmppDiscoveryManager::clientType() const
{
    return d->clientType;
}

/// Returns the name of the local XMPP client.
///
/// By default this is "Based on QXmpp x.y.z".

QString QXmppDiscoveryManager::clientName() const
{
    return d->clientName;
}

/// Returns the client's extended information form, as defined
/// by XEP-0128 Service Discovery Extensions.

QXmppDataForm QXmppDiscoveryManager::clientInfoForm() const
{
    return d->clientInfoForm;
}

/// Sets the client's extended information form, as defined
/// by XEP-0128 Service Discovery Extensions.

void QXmppDiscoveryManager::setClientInfoForm(const QXmppDataForm &form)
{
    d->clientInfoForm = form;
}

/// \cond
QStringList QXmppDiscoveryManager::discoveryFeatures() const
{
    return QStringList() << ns_disco_info;
}

bool QXmppDiscoveryManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element))
    {
        QXmppDiscoveryIq receivedIq;
        receivedIq.parse(element);

        switch (receivedIq.type()) {
        case QXmppIq::Get:
            if (receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
                (receivedIq.queryNode().isEmpty() ||
                 receivedIq.queryNode().startsWith(d->clientCapabilitiesNode))) {

                // respond to info queries for the client itself
                QXmppDiscoveryIq qxmppFeatures = capabilities();
                qxmppFeatures.setId(receivedIq.id());
                qxmppFeatures.setTo(receivedIq.from());
                qxmppFeatures.setQueryNode(receivedIq.queryNode());
                client()->sendPacket(qxmppFeatures);
                return true;
            } else {
                // let other managers handle other queries
                return false;
            }

        case QXmppIq::Result:
        case QXmppIq::Error:
            // handle all replies
            if (receivedIq.queryType() == QXmppDiscoveryIq::InfoQuery) {
                emit infoReceived(receivedIq);
            } else if (receivedIq.queryType() == QXmppDiscoveryIq::ItemsQuery) {
                emit itemsReceived(receivedIq);
            }
            return true;

        case QXmppIq::Set:
            // let other manager handle "set" IQs
            return false;
        }
    }
    return false;
}
/// \endcond
