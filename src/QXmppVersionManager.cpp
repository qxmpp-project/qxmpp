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

#include "QXmppVersionManager.h"
#include "QXmppOutgoingClient.h"
#include "QXmppVersionIq.h"
#include "QXmppGlobal.h"

#include <QCoreApplication>

QXmppVersionManager::QXmppVersionManager(QXmppOutgoingClient* stream, QObject *parent)
    : QObject(parent),
    m_stream(stream)
{
    bool check = QObject::connect(m_stream, SIGNAL(versionIqReceived(const QXmppVersionIq&)),
        this, SLOT(versionIqReceived(const QXmppVersionIq&)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

void QXmppVersionManager::versionIqReceived(const QXmppVersionIq& versionIq)
{
    if(versionIq.type() == QXmppIq::Get)
    {
        // respond to query
        QXmppVersionIq responseIq;
        responseIq.setType(QXmppIq::Result);
        responseIq.setId(versionIq.id());
        responseIq.setTo(versionIq.from());

        QString name = qApp->applicationName();
        if(name.isEmpty())
            name = "Based on QXmpp";
        responseIq.setName(name);

        QString version = qApp->applicationVersion();
        if(version.isEmpty())
            version = QXmppVersion();
        responseIq.setVersion(version);

        // TODO set OS aswell

        m_stream->sendPacket(responseIq);
    }

    emit versionReceived(versionIq);
}
