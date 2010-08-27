/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXMPPSERVEREXTENSION_H
#define QXMPPSERVEREXTENSION_H

#include <QObject>

class QDomElement;
class QStringList;

class QXmppServer;
class QXmppStream;

/// \brief The QXmppServerExtension class is the base class for QXmppServer
/// extensions.
///

class QXmppServerExtension : public QObject
{
    Q_OBJECT

public:
    QString extensionName() const;

    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(QXmppStream *stream, const QDomElement &stanza);
    virtual QStringList presenceSubscribers(const QString &jid);
    virtual bool start(QXmppServer *server);
    virtual void stop();
};

#endif
