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

#include <QCoreApplication>
#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppGlobal.h"
#include "QXmppOutgoingClient.h"
#include "QXmppVersionManager.h"
#include "QXmppVersionIq.h"

QXmppVersionManager::QXmppVersionManager() : QXmppClientExtension(),
    m_name(qApp->applicationName()),
    m_version(qApp->applicationVersion())
{
    if(m_name.isEmpty())
        m_name = "Based on QXmpp";

    if(m_version.isEmpty())
        m_version = QXmppVersion();
}

QStringList QXmppVersionManager::discoveryFeatures() const
{
    // XEP-0092: Software Version
    return QStringList() << ns_version;
}

bool QXmppVersionManager::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppVersionIq::isVersionIq(element))
    {
        QXmppVersionIq versionIq;
        versionIq.parse(element);

        if(versionIq.type() == QXmppIq::Get)
        {
            // respond to query
            QXmppVersionIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(versionIq.id());
            responseIq.setTo(versionIq.from());

            responseIq.setName(name());
            responseIq.setVersion(version());
            responseIq.setOs(os());

            // TODO set OS aswell
            stream->sendPacket(responseIq);
        }

        emit versionReceived(versionIq);
        return true;
    }

    return false;
}

void QXmppVersionManager::requestVersion(const QString& jid)
{
    QXmppVersionIq request;
    request.setType(QXmppIq::Get);
    request.setFrom(client()->configuration().jid());
    request.setTo(jid);
    client()->sendPacket(request);
}

void QXmppVersionManager::setName(const QString& name)
{
    m_name = name;
}

void QXmppVersionManager::setVersion(const QString& version)
{
    m_version = version;
}

void QXmppVersionManager::setOs(const QString& os)
{
    m_os = os;
}

QString QXmppVersionManager::name()
{
    return m_name;
}

QString QXmppVersionManager::version()
{
    return m_version;
}

QString QXmppVersionManager::os()
{
    return m_os;
}
