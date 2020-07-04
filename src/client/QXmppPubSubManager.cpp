/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Germán Márquez Mejía
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

#include "QXmppClient.h"
#include "QXmppPubSubManager.h"
#include "QXmppUtils.h"

///
/// Creates an empty PEP node with the default configuration.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#createNode on the current account's bare JID.
///
/// Calling this before \ref QXmppPubSubManager#publishPepItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of XEP-0060).
///
/// \param nodeName the name of the PEP node to be created
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::createPepNode(const QString &nodeName)
{
    return createNode(client()->configuration().jidBare(), nodeName);
}

///
/// Deletes a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#deleteNode on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete along with all of its items
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::deletePepNode(const QString &nodeName)
{
    return deleteNode(client()->configuration().jidBare(), nodeName);
}

///
/// Publishs one item to a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishPepItem with no publish options.
///
/// \param nodeName the name of the PEP node to publish the item to
/// \param item the item to publish
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishPepItem(const QString &nodeName, const QXmppPubSubItem &item)
{
    return publishPepItem(nodeName, item, QXmppDataForm());
}

///
/// Publishs one item to a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishPepItems with a single item in the items list.
///
/// \param nodeName the name of the PEP node to publish the item to
/// \param item the item to publish
/// \param publishOptions publish-options for fine tuning (optional). Pass
/// an empty form to honor the default options of the PEP node
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishPepItem(const QString &nodeName, const QXmppPubSubItem &item, const QXmppDataForm &publishOptions)
{
    return publishPepItems(nodeName, QList<QXmppPubSubItem>({item}), publishOptions);
}

///
/// Publishs items to a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishPepItems with no publish options.
///
/// \param nodeName the name of the PEP node to publish the items to
/// \param items the items to publish
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishPepItems(const QString &nodeName, const QList<QXmppPubSubItem> &items)
{
    return publishPepItems(nodeName, items, QXmppDataForm());
}

///
/// Publishs items to a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishItems on the current account's bare JID.
///
/// \param nodeName the name  of the PEP node to publish the items to
/// \param items the items to publish
/// \param publishOptions publish-options for fine tuning (optional). Pass
/// an empty form to honor the default options of the PEP node
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishPepItems(const QString &nodeName, const QList<QXmppPubSubItem> &items, const QXmppDataForm &publishOptions)
{
    return publishItems(client()->configuration().jidBare(), nodeName, items, publishOptions);
}

///
/// Deletes an item from a PEP node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#retractItem on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete the item from
/// \param itemId the ID of the item to delete
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::retractPepItem(const QString &nodeName, const QString &itemId)
{
    return retractItem(client()->configuration().jidBare(), nodeName, itemId);
}

///
/// Creates an empty pubsub node with the default configuration.
///
/// Calling this before \ref QXmppPubSubManager#publishItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of XEP-0060).
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to be created
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::createNode(const QString &jid, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq::QueryType::CreateQuery);
    request.setQueryNodeName(nodeName);
    request.setTo(jid);

    if (client()->sendPacket(request))
        return request.id();
    return QString();
}

///
/// Deletes a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete along with all of its items
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::deleteNode(const QString &jid, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq::QueryType::DeleteQuery);
    request.setQueryNodeName(nodeName);
    request.setTo(jid);

    if (client()->sendPacket(request))
        return request.id();
    return QString();
}

///
/// Publishs one item to a pubsub node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishItem with no publish options.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the item to
/// \param item the item to publish
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishItem(const QString &jid, const QString &nodeName, const QXmppPubSubItem &item)
{
    return publishItem(jid, nodeName, item, QXmppDataForm());
}

///
/// Publishs one item to a pubsub node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishItems with a single item in the items list.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the item to
/// \param item the item to publish
/// \param publishOptions publish-options for the item (optional). Pass
/// an empty form to honor the default options of the node
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishItem(const QString &jid, const QString &nodeName, const QXmppPubSubItem &item, const QXmppDataForm &publishOptions)
{
    return publishItems(jid, nodeName, QList<QXmppPubSubItem>({item}), publishOptions);
}

///
/// Publishs items to a pubsub node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#publishItems with no publish options.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the items to
/// \param items the items to publish
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishItems(const QString &jid, const QString &nodeName, const QList<QXmppPubSubItem> &items)
{
    return publishItems(jid, nodeName, items, QXmppDataForm());
}

///
/// Publishs items to a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the items to
/// \param items the items to publish
/// \param publishOptions publish-options for the items (optional). Pass
/// an empty form to honor the default options of the node
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::publishItems(const QString &jid, const QString &nodeName, const QList<QXmppPubSubItem> &items, const QXmppDataForm &publishOptions)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq::QueryType::PublishQuery);
    request.setQueryNodeName(nodeName);
    request.setItems(items);

    if (!publishOptions.isNull())
        request.setPublishOptions(publishOptions);

    request.setTo(jid);

    if (client()->sendPacket(request))
        return request.id();
    return QString();
}

///
/// Deletes an item from a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete the item from
/// \param itemId the ID of the item to delete
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::retractItem(const QString &jid, const QString &nodeName, const QString &itemId)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq::QueryType::RetractQuery);
    request.setQueryNodeName(nodeName);
    request.setItems(QList<QXmppPubSubItem>({QXmppPubSubItem(itemId)}));
    request.setTo(jid);

    if (client()->sendPacket(request))
        return request.id();
    return QString();
}

///
/// Requests one item of an entity's node by ID.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#requestItems with a list of item IDs containing
/// only the given item ID.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this should be an account's bare JID
/// \param nodeName the name of the node to query
/// \param itemId the ID of the item to retrieve
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::requestItem(const QString &jid, const QString &nodeName, const QString &itemId)
{
    return requestItems(jid, nodeName, QStringList(itemId));
}

///
/// Requests all items of an entity's node.
///
/// This is a convenience method equivalent to calling
/// \ref QXmppPubSubManager#requestItems with an empty item ID list.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this should be an account's bare JID
/// \param nodeName the name of the node to query
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::requestItems(const QString &jid, const QString &nodeName)
{
    return requestItems(jid, nodeName, QStringList());
}

///
/// Requests items of an entity's node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this should be an account's bare JID
/// \param nodeName the name of the node to query
/// \param itemIds the ID's of the items to retrieve. If empty, retrieves all the items
/// \return the ID of the resulting IQ stanza if sent. Empty string otherwise
///
QString QXmppPubSubManager::requestItems(const QString &jid, const QString &nodeName, const QStringList &itemIds)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppPubSubIq::QueryType::ItemsQuery);
    request.setQueryNodeName(nodeName);

    if (!itemIds.isEmpty()) {
        QList<QXmppPubSubItem> items;
        for (const QString &id : itemIds) {
            items << QXmppPubSubItem(id);
        }
        request.setItems(items);
    }

    request.setTo(jid);

    if (client()->sendPacket(request))
        return request.id();
    return QString();
}

/// \cond
bool QXmppPubSubManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppPubSubIq::isPubSubIq(element)) {
        QXmppPubSubIq iq;
        iq.parse(element);

        if (iq.queryType() == QXmppPubSubIq::QueryType::ItemsQuery) {
            emit itemsReceived(iq);
            return true;
        }
    }

    return false;
}

void QXmppPubSubManager::setClient(QXmppClient* client)
{
    QXmppClientExtension::setClient(client);

    connect(client, &QXmppClient::messageReceived,
            this, &QXmppPubSubManager::_q_messageReceived);
}
/// \endcond

void QXmppPubSubManager::_q_messageReceived(const QXmppMessage &message)
{
    if (message.isPubSubEvent())
        emit eventNotificationReceived(message);
}
