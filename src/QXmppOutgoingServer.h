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

#ifndef QXMPPOUTGOINGSERVER_H
#define QXMPPOUTGOINGSERVER_H

#include "QXmppOutgoingClient.h"

class QXmppDialback;
class QXmppOutgoingServer;
class QXmppOutgoingServerPrivate;

/// The QXmppOutgoingServer class represents an outgoing XMPP stream
/// to another XMPP server.
///

class QXmppOutgoingServer : public QXmppOutgoingClient
{
    Q_OBJECT

public:
    QXmppOutgoingServer(const QString &domain, QObject *parent);
    ~QXmppOutgoingServer();

    bool isConnected() const;

    QString localStreamKey() const;
    void setLocalStreamKey(const QString &key);
    void setVerify(const QString &id, const QString &key);

signals:
    void dialbackResponseReceived(const QXmppDialback &response);

protected:
    void handleStart();
    void handleStanza(const QDomElement &stanzaElement);

private:
    Q_DISABLE_COPY(QXmppOutgoingServer)
    QXmppOutgoingServerPrivate* const d;
};

#endif
