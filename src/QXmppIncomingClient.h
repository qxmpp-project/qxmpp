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

#ifndef QXMPPINCOMINGCLIENT_H
#define QXMPPINCOMINGCLIENT_H

#include "QXmppStream.h"

class QXmppIncomingClientPrivate;

/// \brief Interface for password checkers.
///

class QXmppPasswordChecker
{
public:
    /// Checks that the given credentials are valid.
    ///
    /// \param username
    /// \param password
    virtual bool checkCredentials(const QString &username, const QString &password) = 0;
    virtual bool getPassword(const QString &username, QString &password);
    virtual bool hasPasswords() const;
};

/// \brief The QXmppIncomingClient class represents an incoming XMPP stream
/// from an XMPP client.
///

class QXmppIncomingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent = 0);
    ~QXmppIncomingClient();

    bool isConnected() const;
    QString jid() const;

    void setPasswordChecker(QXmppPasswordChecker *checker);

protected:
    /// \cond
    void handleStream(const QDomElement &element);
    void handleStanza(const QDomElement &element);
    /// \endcond

private slots:
    void slotTimeout();

private:
    Q_DISABLE_COPY(QXmppIncomingClient)
    QXmppIncomingClientPrivate* const d;
};

#endif
