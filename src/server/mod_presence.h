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

#ifndef QXMPP_SERVER_PRESENCE_H
#define QXMPP_SERVER_PRESENCE_H

#include "QXmppServerExtension.h"

class QXmppPresence;
class QXmppServerPresencePrivate;

/// \brief QXmppServer extension for presence handling.
///

class QXmppServerPresence : public QXmppServerExtension
{
    Q_OBJECT
    Q_CLASSINFO("ExtensionName", "presence");

public:
    QXmppServerPresence();
    ~QXmppServerPresence();

    QList<QXmppPresence> availablePresences(const QString &bareJid) const;
    bool handleStanza(const QDomElement &element);
    QSet<QString> presenceSubscribers(const QString &jid);
    bool start();
    void stop();

    static QXmppServerPresence* instance(QXmppServer *server);

private slots:
    void _q_clientDisconnected(const QString &jid);

private:
    friend class QXmppServerPresencePrivate;
    QXmppServerPresencePrivate *d;
};

#endif
