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


#include "QXmppEntityTimeManager.h"

#include <QDomElement>
#include <QDateTime>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppEntityTimeIq.h"
#include "QXmppUtils.h"

/// Request the time from an XMPP entity.
///
/// \param jid

QString QXmppEntityTimeManager::requestTime(const QString& jid)
{
    QXmppEntityTimeIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

/// \cond
QStringList QXmppEntityTimeManager::discoveryFeatures() const
{
    return QStringList() << ns_entity_time;
}

bool QXmppEntityTimeManager::handleStanza(const QDomElement &element)
{
    if(element.tagName() == "iq" && QXmppEntityTimeIq::isEntityTimeIq(element))
    {
        QXmppEntityTimeIq entityTime;
        entityTime.parse(element);

        if(entityTime.type() == QXmppIq::Get)
        {
            // respond to query
            QXmppEntityTimeIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(entityTime.id());
            responseIq.setTo(entityTime.from());

            QDateTime currentTime = QDateTime::currentDateTime();
            QDateTime utc = currentTime.toUTC();
            responseIq.setUtc(utc);

            currentTime.setTimeSpec(Qt::UTC);
            responseIq.setTzo(utc.secsTo(currentTime));

            client()->sendPacket(responseIq);
        }

        emit timeReceived(entityTime);
        return true;
    }

    return false;
}
/// \endcond
