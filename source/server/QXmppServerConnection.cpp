/*
 * Copyright (C) 2010 Sjors Gielen, Manjeet Dahiya
 *
 * Authors:
 *	Manjeet Dahiya
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


#include "QXmppServerConnection.h"
#include "QXmppLogger.h"
#include "QXmppStream.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#define QXMPPSERVERCONNECTION_DEBUG

#define ASSERT_CONNECT(a, b, c, d) \
      { bool check = connect(a, b, c, d); \
      Q_ASSERT( check ); }

/**
 * @brief Creates a QXmppServerConnection object.
 *
 * This class is meant for server to server connections, both initiated by this
 * server (in which case the connectToHost method should be used), and
 * initiated by other servers (in which case a QSslSocket should be given to
 * this class). To listen to incoming connections, use QXmppServer.
 * 
 * @param socket Server socket if nonzero, ignored if zero
 * @param data   Server data to start parsing, only if server socket nonzero
 * @param parent QObject parent of this object
 */
QXmppServerConnection::QXmppServerConnection(QSslSocket *socket,
    const QByteArray &data, QObject *parent)
: QObject(parent)
, m_logger(0)
, m_stream(0)
{
    Q_ASSERT( socket == 0 || socket->state() == QAbstractSocket::ConnectedState );

    m_logger = QXmppLogger::getLogger();
    //m_stream = new QXmppStream(this);
    //m_stream->setSocket( socket );

    ASSERT_CONNECT(m_stream, SIGNAL(messageReceived(const QXmppMessage&)),
                       this, SIGNAL(messageReceived(const QXmppMessage&)));

    ASSERT_CONNECT(m_stream, SIGNAL(disconnected()),
                       this, SIGNAL(disconnected()));

    ASSERT_CONNECT(m_stream, SIGNAL(xmppConnected()),
                       this, SIGNAL(connected()));

    ASSERT_CONNECT(m_stream, SIGNAL(error(QXmppServerConnection::Error)),
                       this, SIGNAL(error(QXmppServerConnection::Error)));
}

/// Destructor, destroys the QXmppServerConnection object.
///

QXmppServerConnection::~QXmppServerConnection()
{
}

/**
 * @brief Attempts to connect to another XMPP server.
 *
 * @param host Hostname of the XMPP server where connection has to be made
 * (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
 * the form of a string (e.g. "192.168.1.25").
 * @param domain Domain name of the other side e.g. "gmail.com", "jabber.org".
 * @param port Port number at which the XMPP server is listening. The default
 * value is 5269.
 */
void QXmppServerConnection::connectToServer(const QString& host, const QString& domain,
                                  int port)
{
    disconnect();
    //m_stream->connect( host, domain, port );
}

/// After successfully connecting to the server use this function to send
/// stanzas to the server. This function can solely be used to send various kind
/// of stanzas to the server. QXmppPacket is a parent class of all the stanzas
/// QXmppMessage, QXmppPresence, QXmppIq, QXmppBind, QXmppRosterIq, QXmppSession
/// and QXmppVCard.
///
/// Following code snippet illustrates how to send a message using this function:
/// \code
/// QXmppMessage message(from, to, message);
/// client.sendPacket(message);
/// \endcode
///
/// \param packet A valid XMPP stanza. It can be an iq, a message or a presence stanza.
///

void QXmppServerConnection::sendPacket(const QXmppPacket& packet)
{
    Q_ASSERT( m_stream != 0 );
    m_stream->sendPacket(packet);
}

/// Disconnects the client and the current presence of client changes to
/// QXmppPresence::Unavailable and statatus text changes to "Logged out".
///
/// \note Make sure that the clientPresence is changed to
/// QXmppPresence::Available, if you are again calling connectToServer() after
/// calling the disconnect() function.
///

void QXmppServerConnection::disconnect()
{
    Q_ASSERT( m_stream != 0 );
    m_stream->disconnect();
}

/// Returns the socket error if QXmppServerConnection::Error is QXmppServerConnection::SocketError.
///
/// \return QAbstractSocket::SocketError
///

QAbstractSocket::SocketError QXmppServerConnection::getSocketError()
{
    return m_stream->getSocketError();
}

/// Returns the XMPP stream error if QXmppServerConnection::Error is QXmppServerConnection::XmppStreamError.
///
/// \return QXmppServerConnection::Error::Condition
///

QXmppStanza::Error::Condition QXmppServerConnection::getXmppStreamError()
{
    return m_stream->getXmppStreamError();
}

/// Return the QXmppLogger associated with the client.

QXmppLogger *QXmppServerConnection::logger()
{
    return m_logger;
}

void QXmppServerConnection::setLogger(QXmppLogger *logger)
{
    m_logger = logger;
}

