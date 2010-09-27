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


#include "QXmppEntityTimeManager.h"

#include <QDomElement>
#include <QDateTime>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppEntityTimeIq.h"
#include "QXmppUtils.h"

QString QXmppEntityTimeManager::requestTime(const QString& jid)
{
    QXmppEntityTimeIq request;
    request.setType(QXmppIq::Get);
    request.setFrom(client()->configuration().jid());
    request.setTo(jid);
    if(client()->sendPacket(request))
        return request.id();
    else
        return "";
}

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
            responseIq.setUtc(datetimeToString(utc));

            currentTime.setTimeSpec(Qt::UTC);
            int tzo_sec = utc.secsTo(currentTime);
            QTime tzo_time;
            if(tzo_sec < 0)
                tzo_time = tzo_time.addSecs(-tzo_sec);
            else
                tzo_time = tzo_time.addSecs(tzo_sec);
            QString tzo;
            if(tzo_sec < 0)
                tzo = "-" + tzo_time.toString("hh:mm");
            else
                tzo = "+" + tzo_time.toString("hh:mm");

            responseIq.setTzo(tzo);

            client()->sendPacket(responseIq);
        }

        emit timeReceived(entityTime);
        return true;
    }

    return false;
}
