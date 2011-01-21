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

#ifndef QXMPPOUTGOINGSERVER_H
#define QXMPPOUTGOINGSERVER_H

#include "QXmppStream.h"

class QSslError;
class QXmppDialback;
class QXmppOutgoingServer;
class QXmppOutgoingServerPrivate;
class QXmppSrvInfo;

/// \brief The QXmppOutgoingServer class represents an outgoing XMPP stream
/// to another XMPP server.
///

class QXmppOutgoingServer : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingServer(const QString &domain, QObject *parent);
    ~QXmppOutgoingServer();

    void connectToHost(const QString &domain);
    bool isConnected() const;

    QString localStreamKey() const;
    void setLocalStreamKey(const QString &key);
    void setVerify(const QString &id, const QString &key);

    QString remoteDomain() const;

signals:
    /// This signal is emitted when a dialback verify response is received.
    void dialbackResponseReceived(const QXmppDialback &response);

protected:
    /// \cond
    void handleStart();
    void handleStream(const QDomElement &streamElement);
    void handleStanza(const QDomElement &stanzaElement);
    /// \endcond

private slots:
    void connectToHost(const QXmppSrvInfo &serviceInfo);
    void sendDialback();
    void slotSslErrors(const QList<QSslError> &errors);

private:
    Q_DISABLE_COPY(QXmppOutgoingServer)
    QXmppOutgoingServerPrivate* const d;
};

#endif
