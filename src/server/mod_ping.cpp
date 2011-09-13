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

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppPingIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppStream.h"

#include "mod_ping.h"

QStringList QXmppServerPing::discoveryFeatures() const
{
    return QStringList() << ns_ping;
}

bool QXmppServerPing::handleStanza(const QDomElement &element)
{
    if (element.attribute("to") != server()->domain())
        return false;

    // XEP-0199: XMPP Ping
    if (element.tagName() == "iq" && QXmppPingIq::isPingIq(element))
    {
        QXmppPingIq request;
        request.parse(element);

        QXmppIq response(QXmppIq::Result);
        response.setId(request.id());
        response.setFrom(request.to());
        response.setTo(request.from());
        server()->sendPacket(response);
        return true;
    }

    return false;
}

// PLUGIN

class QXmppServerPingPlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("ping"))
            return new QXmppServerPing;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("ping");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_ping, QXmppServerPingPlugin)

