/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QCoreApplication>
#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppStream.h"

#include "mod_disco.h"

QStringList QXmppServerDiscovery::items() const
{
    return m_discoveryItems;
}

void QXmppServerDiscovery::setItems(const QStringList &items)
{
    m_discoveryItems = items;
}

QStringList QXmppServerDiscovery::discoveryFeatures() const
{
    return QStringList() << ns_disco_info << ns_disco_items;
}

QStringList QXmppServerDiscovery::discoveryItems() const
{
    return m_discoveryItems;
}

bool QXmppServerDiscovery::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    Q_UNUSED(stream);

    if (element.attribute("to") != server()->domain())
        return false;

    // XEP-0030: Service Discovery
    const QString type = element.attribute("type");
    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element) && type == "get")
    {
        QXmppDiscoveryIq request;
        request.parse(element);

        QXmppDiscoveryIq response;
        response.setType(QXmppIq::Result);
        response.setId(request.id());
        response.setFrom(request.to());
        response.setTo(request.from());
        response.setQueryType(request.queryType());

        if (request.queryType() == QXmppDiscoveryIq::ItemsQuery)
        {
            QList<QXmppDiscoveryIq::Item> items;
            foreach (QXmppServerExtension *extension, server()->extensions())
            {
                foreach (const QString &jid, extension->discoveryItems())
                {
                    QXmppDiscoveryIq::Item item;
                    item.setJid(jid);
                    items.append(item);
                }
            }
            response.setItems(items);
        } else {
            // identities
            QList<QXmppDiscoveryIq::Identity> identities;
            QXmppDiscoveryIq::Identity identity;
            identity.setCategory("server");
            identity.setType("im");
            identity.setName(qApp->applicationName());
            identities.append(identity);
            response.setIdentities(identities);

            // features
            QStringList features;
            foreach (QXmppServerExtension *extension, server()->extensions())
                features += extension->discoveryFeatures();
            response.setFeatures(features);
        }
        server()->sendPacket(response);
        return true;
    }
    return false;
}

// PLUGIN

class QXmppServerDiscoveryPlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("ping"))
            return new QXmppServerDiscovery;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("ping");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_disco, QXmppServerDiscoveryPlugin)

