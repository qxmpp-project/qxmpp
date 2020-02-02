/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppVersionManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppGlobal.h"
#include "QXmppVersionIq.h"

#include <QCoreApplication>
#include <QDomElement>
#include <QSysInfo>

class QXmppVersionManagerPrivate
{
public:
    QString clientName;
    QString clientVersion;
    QString clientOs;
};

QXmppVersionManager::QXmppVersionManager()
    : d(new QXmppVersionManagerPrivate)
{
    d->clientName = qApp->applicationName();
    if (d->clientName.isEmpty())
        d->clientName = "Based on QXmpp";

    d->clientOs = QSysInfo::prettyProductName();
    d->clientVersion = qApp->applicationVersion();
    if (d->clientVersion.isEmpty())
        d->clientVersion = QXmppVersion();
}

QXmppVersionManager::~QXmppVersionManager()
{
    delete d;
}

/// Request version information from the specified XMPP entity.
///
/// \param jid

QString QXmppVersionManager::requestVersion(const QString& jid)
{
    QXmppVersionIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

/// Sets the local XMPP client's name.
///
/// \param name

void QXmppVersionManager::setClientName(const QString& name)
{
    d->clientName = name;
}

/// Sets the local XMPP client's version.
///
/// \param version

void QXmppVersionManager::setClientVersion(const QString& version)
{
    d->clientVersion = version;
}

/// Sets the local XMPP client's operating system.
///
/// \param os

void QXmppVersionManager::setClientOs(const QString& os)
{
    d->clientOs = os;
}

/// Returns the local XMPP client's name.
///
/// By default this is set to the QApplication::applicationName(), or
/// "Based on QXmpp" if not specified.

QString QXmppVersionManager::clientName() const
{
    return d->clientName;
}

/// Returns the local XMPP client's version.
///
/// By default this is set to QApplication::applicationVersion(), or
/// QXmpp's version if not specified.

QString QXmppVersionManager::clientVersion() const
{
    return d->clientVersion;
}

/// Returns the local XMPP client's operating system.
///
/// By default this equals to QSysInfo::prettyProductName() which contains the
/// OS name and version (e.g. "Windows 8.1" or "Debian GNU/Linux buster").

QString QXmppVersionManager::clientOs() const
{
    return d->clientOs;
}

/// \cond
QStringList QXmppVersionManager::discoveryFeatures() const
{
    // XEP-0092: Software Version
    return QStringList() << ns_version;
}

bool QXmppVersionManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppVersionIq::isVersionIq(element))
    {
        QXmppVersionIq versionIq;
        versionIq.parse(element);

        if (versionIq.type() == QXmppIq::Get) {
            // respond to query
            QXmppVersionIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(versionIq.id());
            responseIq.setTo(versionIq.from());

            responseIq.setName(clientName());
            responseIq.setVersion(clientVersion());
            responseIq.setOs(clientOs());

            client()->sendPacket(responseIq);
        } else if (versionIq.type() == QXmppIq::Result) {
            // emit response
            emit versionReceived(versionIq);
        }

        return true;
    }

    return false;
}
/// \endcond
