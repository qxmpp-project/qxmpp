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
#include "QXmppVersionIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppStream.h"

#include "mod_version.h"

QStringList QXmppServerVersion::discoveryFeatures() const
{
    return QStringList() << ns_version;
}

bool QXmppServerVersion::handleStanza(const QDomElement &element)
{
    if (element.attribute("to") != server()->domain())
        return false;

    // XEP-0092: Software Version
    if(QXmppVersionIq::isVersionIq(element))
    {
        QXmppVersionIq versionIq;
        versionIq.parse(element);

        if (versionIq.type() == QXmppIq::Get)
        {
            QXmppVersionIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(versionIq.id());
            responseIq.setTo(versionIq.from());
            responseIq.setName(qApp->applicationName());
            responseIq.setVersion(qApp->applicationVersion());
            server()->sendPacket(responseIq);
        }
        return true;
    }

    return false;
}

// PLUGIN

class QXmppServerVersionPlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("version"))
            return new QXmppServerVersion;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("version");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_version, QXmppServerVersionPlugin)

