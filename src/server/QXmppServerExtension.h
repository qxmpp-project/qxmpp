/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPSERVEREXTENSION_H
#define QXMPPSERVEREXTENSION_H

#include <QVariant>

#include "QXmppLogger.h"

class QDomElement;
class QStringList;

class QXmppServer;
class QXmppServerExtensionPrivate;
class QXmppStream;

/// \brief The QXmppServerExtension class is the base class for QXmppServer
/// extensions.
///
/// If you want to extend QXmppServer, for instance to support an IQ type
/// which is not natively supported, you can subclass QXmppServerExtension
/// and implement handleStanza(). You can then add your extension to the
/// client instance using QXmppServer::addExtension().
///
/// \ingroup Core

class QXMPP_EXPORT QXmppServerExtension : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppServerExtension();
    ~QXmppServerExtension();
    virtual QString extensionName() const;
    virtual int extensionPriority() const;

    virtual QStringList discoveryFeatures() const;
    virtual QStringList discoveryItems() const;
    virtual bool handleStanza(const QDomElement &stanza);
    virtual QSet<QString> presenceSubscribers(const QString &jid);
    virtual QSet<QString> presenceSubscriptions(const QString &jid);

    virtual bool start();
    virtual void stop();

protected:
    QXmppServer *server() const;

private:
    void setServer(QXmppServer *server);
    QXmppServerExtensionPrivate * const d;

    friend class QXmppServer;
};

#endif
