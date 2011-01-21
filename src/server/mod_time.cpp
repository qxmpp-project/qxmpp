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
#include "QXmppEntityTimeIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppStream.h"

#include "mod_time.h"

QStringList QXmppServerTime::discoveryFeatures() const
{
    return QStringList() << ns_entity_time;
}

bool QXmppServerTime::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    Q_UNUSED(stream);

    if (element.attribute("to") != server()->domain())
        return false;

    // XEP-0202: Entity Time
    if(QXmppEntityTimeIq::isEntityTimeIq(element))
    {
        QXmppEntityTimeIq timeIq;
        timeIq.parse(element);

        if (timeIq.type() == QXmppIq::Get)
        {
            QXmppEntityTimeIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(timeIq.id());
            responseIq.setTo(timeIq.from());

            QDateTime currentTime = QDateTime::currentDateTime();
            QDateTime utc = currentTime.toUTC();
            responseIq.setUtc(utc);

            currentTime.setTimeSpec(Qt::UTC);
            responseIq.setTzo(utc.secsTo(currentTime));

            server()->sendPacket(responseIq);
        }
        return true;
    }

    return false;
}

// PLUGIN

class QXmppServerTimePlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("time"))
            return new QXmppServerTime;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("time");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_time, QXmppServerTimePlugin)

