/*
 * Copyright (C) 2010 Sjors Gielen, Manjeet Dahiya
 *
 * Authors:
 *  Manjeet Dahiya
 *  Sjors Gielen
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

#ifndef QXMPPSERVERCONNECTION_H
#define QXMPPSERVERCONNECTION_H

#include <QObject>
#include <QSslSocket>

#include "QXmppClient.h"

/**
 * @brief The QXmppServerConnection class handles connections to other servers.
 */

class QXmppServerConnection : public QObject
{
    Q_OBJECT

public:
    // should probably be moved to QXmppStream
    enum Error
    {
        SocketError,        ///< Error due to TCP socket
        KeepAliveError,     ///< Error due to no response to a keep alive
        XmppStreamError,    ///< Error due to XML stream
    };

    QXmppServerConnection(QSslSocket *serverSocket = 0,
        const QByteArray &parseData = QByteArray(), QObject *parent = 0);
    ~QXmppServerConnection();
    void connectToServer(const QString& host,
                         const QString& domain,
                         int port = 5269);
    void disconnect();

signals:

    /// This signal is emitted when the client connects sucessfully to the XMPP
    /// server i.e. when a successful XMPP connection is established.
    /// XMPP Connection involves following sequential steps:
    ///     - TCP socket connection
    ///     - Client sends start stream
    ///     - Server sends start stream
    ///     - TLS negotiation (encryption)
    ///     - Authentication
    ///     - Resource binding
    ///     - Session establishment
    ///
    /// After all these steps a successful XMPP connection is established and
    /// connected() signal is emitted.
    ///
    /// After the connected() signal is emitted QXmpp will send the roster request
    /// to the server. On receiving the roster, QXmpp will emit
    /// QXmppRoster::rosterReceived(). After this signal, QXmppRoster object gets
    /// populated and you can use getRoster() to get the handle of QXmppRoster object.
    ///
    void connected();

    /// This signal is emitted when the XMPP connection disconnects.
    ///
    void disconnected();

    /// This signal is emitted when the XMPP connection encounters any error.
    /// The QXmppClient::Error parameter specifies the type of error occured.
    /// It could be due to TCP socket or the xml stream or the stanza.
    /// Depending upon the type of error occured use the respective get function to
    /// know the error.
    void error(QXmppClient::Error);

    /// Notifies that an XMPP message stanza is received. The QXmppMessage
    /// parameter contains the details of the message sent to this client.
    /// In other words whenever someone sends you a message this signal is
    /// emitted.
    void messageReceived(const QXmppMessage&);

public:
    QAbstractSocket::SocketError socketError();
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

public slots:
    void sendPacket(const QXmppPacket&);

private:
    QXmppStream* m_stream;  ///< Pointer to QXmppStream object, a wrapper over
                            ///< TCP socket and XMPP protocol
};

#endif // QXMPPCLIENT_H
