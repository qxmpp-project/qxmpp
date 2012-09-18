/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *	Ian Reinhart Geiser
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include "QXmppCallManager.h"
#include "QXmppClient.h"
#include "qdnslookup.h"

class QHostInfo;

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    xmppClient(QObject *parent = 0);
    void setRecipient(const QString &recipient);

private slots:
    void slotAudioModeChanged(QIODevice::OpenMode mode);
    void slotCallReceived(QXmppCall *call);
    void slotCallStateChanged(QXmppCall::State state);
    void slotConnected();
    void slotDnsLookupFinished();
    void slotHostInfoFinished(const QHostInfo &hostInfo);
    void slotPresenceReceived(const QXmppPresence &presence);

private:
    QXmppCallManager *callManager;
    QDnsLookup m_dns;
    QString m_recipient;
    quint16 m_turnPort;
};

#endif // IBBCLIENT_H
