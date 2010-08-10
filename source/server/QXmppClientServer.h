/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXMPPCLIENTSERVER_H
#define QXMPPCLIENTSERVER_H

#include <QObject>
#include <QSslSocket>

#include "QXmppClient.h"

/**
 * @brief The QXmppClientServer receives connections from clients.
 */

class QXmppClientServer : public QObject
{
    Q_OBJECT

public:
    QXmppClientServer(QSslSocket *serverSocket,
        const QByteArray &parseData = QByteArray(), QObject *parent = 0);
    ~QXmppClientServer();

    void disconnect();

signals:

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
