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

#ifndef QXMPPINCOMINGCLIENT_H
#define QXMPPINCOMINGCLIENT_H

#include "QXmppStream.h"

class QXmppIncomingClientPrivate;
class QXmppPasswordChecker;

/// \brief Interface for password checkers.
///

/// \brief The QXmppIncomingClient class represents an incoming XMPP stream
/// from an XMPP client.
///

class QXMPP_EXPORT QXmppIncomingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent = 0);
    ~QXmppIncomingClient();

    bool isConnected() const;
    QString jid() const;

    void setInactivityTimeout(int secs);
    void setPasswordChecker(QXmppPasswordChecker *checker);

signals:
    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element);

protected:
    /// \cond
    void handleStream(const QDomElement &element);
    void handleStanza(const QDomElement &element);
    /// \endcond

private slots:
    void onDigestReply();
    void onPasswordReply();
    void onSocketDisconnected();
    void onTimeout();

private:
    Q_DISABLE_COPY(QXmppIncomingClient)
    QXmppIncomingClientPrivate* d;
    friend class QXmppIncomingClientPrivate;
};

#endif
