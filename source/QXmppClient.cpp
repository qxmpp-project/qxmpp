/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#include "QXmppClient.h"
#include "QXmppStream.h"
#include "QXmppRoster.h"
#include "QXmppMessage.h"
#include "QXmppReconnectionManager.h"
#include "QXmppIbbTransferManager.h"
#include "QXmppInvokable.h"
#include "QXmppRpcIq.h"
#include "QXmppUtils.h"

/// Creates a QXmppClient object.
/// \param parent is passed to the QObject's contructor.
/// The default value is 0.

QXmppClient::QXmppClient(QObject *parent)
    : QObject(parent), m_stream(0), m_clientPrecence(QXmppPresence::Available),
    m_reconnectionManager(0), m_ibbTransferManager(0)
{
    m_stream = new QXmppStream(this);

    bool check = connect(m_stream, SIGNAL(messageReceived(const QXmppMessage&)),
                         this, SIGNAL(messageReceived(const QXmppMessage&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(presenceReceived(const QXmppPresence&)),
                    this, SIGNAL(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(iqReceived(const QXmppIq&)), this,
        SIGNAL(iqReceived(const QXmppIq&)));

    check = connect(m_stream, SIGNAL(disconnected()), this,
        SIGNAL(disconnected()));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(xmppConnected()), this,
        SIGNAL(connected()));
    Q_ASSERT(check);

    check = connect(m_stream, SIGNAL(error(QXmppClient::Error)), this,
        SIGNAL(error(QXmppClient::Error)));
    Q_ASSERT(check);

    check = setReconnectionManager(new QXmppReconnectionManager(this));
    Q_ASSERT(check);

    m_ibbTransferManager = new QXmppIbbTransferManager(this);
}

/// Destroys the QXmppClient object.
///

QXmppClient::~QXmppClient()
{
}

/// Returns a modifiable reference to the current configuration of QXmppClient.
/// \return Reference to the QXmppClient's configuration for the connection.

QXmppConfiguration& QXmppClient::getConfiguration()
{
    return m_config;
}

/// Overloaded function. It returns a const reference to the current configuration
/// of QXmppClient.
/// \return Constant reference to the QXmppClient's configuration for the connection.

const QXmppConfiguration& QXmppClient::getConfiguration() const
{
    return m_config;
}

/// Attempts to connect to the XMPP server. Server details and other configurations
/// are specified using the config parameter. Use signals connected(), error(QXmppClient::Error)
/// and disconnected() to know the status of the connection.
/// \param config Specifies the configuration object for connecting the XMPP server.
/// This contains the host name, user, passwd etc. See QXmppConfiguration for details.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QXmppConfiguration& config,
                                  const QXmppPresence& initialPresence)
{
    m_config = config;

    if(!m_config.getAutoReconnectionEnabled())
    {
        delete m_reconnectionManager;
        m_reconnectionManager = 0;
    }

    m_clientPrecence = initialPresence;

    m_stream->connect();
}

/// Overloaded function.
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
/// \param user Username of the account at the specified XMPP server. It should
/// be the name without the domain name. E.g. "qxmpp.test1" and not
/// "qxmpp.test1@gmail.com"
/// \param passwd Password for the specified username
/// \param domain Domain name e.g. "gmail.com" and "jabber.org".
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QString& host, const QString& user,
                                  const QString& passwd, const QString& domain,
                                  int port,
                                  const QXmppPresence& initialPresence)
{
    m_config.setHost(host);
    m_config.setUser(user);
    m_config.setPasswd(passwd);
    m_config.setDomain(domain);
    m_config.setPort(port);

    m_clientPrecence = initialPresence;

    m_stream->connect();
}

/// Overloaded function.
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
/// \param bareJid BareJid of the account at the specified XMPP server.
/// E.g. "qxmpp.test1@gmail.com" or qxmpptest@jabber.org.
/// \param passwd Password for the specified username
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QString& host,
                                  const QString& bareJid,
                                  const QString& passwd,
                                  int port,
                                  const QXmppPresence& initialPresence)
{
    QString user, domain;
    QStringList list = bareJid.split("@");
    if(list.size() == 2)
    {
        user = list.at(0);
        domain = list.at(1);
        connectToServer(host, user, passwd, domain, port, initialPresence);
    }
    else
    {
        qWarning("QXmppClient::connectToServer: Invalid bareJid");
        log(QString("Invalid bareJid"));
    }
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

void QXmppClient::sendPacket(const QXmppPacket& packet)
{
    if(m_stream)
    {
        m_stream->sendPacket(packet);
    }
}

/// Disconnects the client and the current presence of client changes to
/// QXmppPresence::Unavailable and statatus text changes to "Logged out".
///
/// \note Make sure that the clientPresence is changed to
/// QXmppPresence::Available, if you are again calling connectToServer() after
/// calling the disconnect() function.
///

void QXmppClient::disconnect()
{
    m_clientPrecence.setType(QXmppPresence::Unavailable);
    m_clientPrecence.getStatus().setType(QXmppPresence::Status::Online);
    m_clientPrecence.getStatus().setStatusText("Logged out");
    sendPacket(m_clientPrecence);
    if(m_stream)
        m_stream->disconnect();
}

/// Returns the reference to QXmppRoster object of the client.
/// \return Reference to the roster object of the connected client. Use this to
/// get the list of friends in the roster and their presence information.
///

QXmppRoster& QXmppClient::getRoster()
{
    return m_stream->getRoster();
}

/// Utility function to send message to all the resources associated with the
/// specified bareJid.
///
/// \param bareJid bareJid of the receiving entity
/// \param message Message string to be sent.

void QXmppClient::sendMessage(const QString& bareJid, const QString& message)
{
    QStringList resources = getRoster().getResources(bareJid);
    for(int i = 0; i < resources.size(); ++i)
    {
        sendPacket(QXmppMessage("", bareJid + "/" + resources.at(i), message));
    }
}

/// Changes the presence of the connected client.
///
/// \param presence QXmppPresence object
///

void QXmppClient::setClientPresence(const QXmppPresence& presence)
{
    m_clientPrecence = presence;
    sendPacket(m_clientPrecence);
}

/// Overloaded function.
///
/// It only changes the status text.
///
/// \param statusText New status message string
///

void QXmppClient::setClientPresence(const QString& statusText)
{
    m_clientPrecence.getStatus().setStatusText(statusText);
    sendPacket(m_clientPrecence);
}

/// Overloaded function.
///
/// It only changes the QXmppPresence::Type.
///
/// \param presenceType New QXmppPresence::Type
///

void QXmppClient::setClientPresence(QXmppPresence::Type presenceType)
{
    if(presenceType == QXmppPresence::Unavailable)
    {
        disconnect();
    }
    else
    {
        m_clientPrecence.setType(presenceType);
        sendPacket(m_clientPrecence);
    }
}

/// Overloaded function.
///
/// It only changes the QXmppPresence::Status::Type.
///
/// \param statusType New QXmppPresence::Status::Type
///

void QXmppClient::setClientPresence(QXmppPresence::Status::Type statusType)
{
    m_clientPrecence.getStatus().setType(statusType);
    sendPacket(m_clientPrecence);
}

/// Function to get the client's current presence.
///
/// \return Constant reference to the client's presence object
///

const QXmppPresence& QXmppClient::getClientPresence() const
{
    return m_clientPrecence;
}

/// Function to get reconnection manager. By default there exists a reconnection
/// manager. See QXmppReconnectionManager for more details of the reconnection
/// mechanism.
///
/// \return Pointer to QXmppReconnectionManager
///

QXmppReconnectionManager* QXmppClient::getReconnectionManager()
{
    return m_reconnectionManager;
}

/// Sets the user defined reconnection manager.
///
/// \return true if all the signal-slot connections are made correctly.
///

bool QXmppClient::setReconnectionManager(QXmppReconnectionManager*
                                         reconnectionManager)
{
	if(!reconnectionManager)
		return false;
		
    if(m_reconnectionManager)
        delete m_reconnectionManager;

    m_reconnectionManager = reconnectionManager;

    bool check = connect(this, SIGNAL(connected()), m_reconnectionManager,
                         SLOT(connected()));
    Q_ASSERT(check);
	if(!check)
		return false;

    check = connect(this, SIGNAL(error(QXmppClient::Error)),
                    m_reconnectionManager, SLOT(error(QXmppClient::Error)));
    Q_ASSERT(check);
	if(!check)
		return false;
	
    return true;
}

/// Returns the socket error if QXmppClient::Error is QXmppClient::SocketError.
///
/// \return QAbstractSocket::SocketError
///

QAbstractSocket::SocketError QXmppClient::getSocketError()
{
    return m_stream->getSocketError();
}

/// Returns the reference to QXmppVCardManager, implimentation of XEP-0054.
/// http://xmpp.org/extensions/xep-0054.html
///

QXmppVCardManager& QXmppClient::getVCardManager()
{
    return m_stream->getVCardManager();
}

void QXmppClient::addInvokableInterface( QXmppInvokable *interface )
{
    m_interfaces[ interface->metaObject()->className() ] = interface;
}


void QXmppClient::invokeInterfaceMethod( const QXmppRpcInvokeIq &iq )
{
    QXmppStanza::Error error;
    QString interface = iq.getInterface();
    QXmppInvokable *iface = m_interfaces[ interface ];
    if( iface )
    {
        if ( iface->isAuthorized( iq.getFrom() ) )
        {

            if ( iface->interfaces().contains( iq.getMethod() ) )
            {
                QVariant result = iface->dispatch(iq.getMethod().toLatin1(),
                                                  iq.getPayload() );
                QXmppRpcResponseIq resultIq;
                resultIq.setId(iq.getId());
                resultIq.setTo(iq.getFrom());
                resultIq.setFrom( m_config.getJid());
                resultIq.setPayload(result);
                m_stream->sendPacket( resultIq );
                return;
            }
            else
            {
                error.setType(QXmppStanza::Error::Cancel);
                error.setCondition(QXmppStanza::Error::ItemNotFound);

            }
        }
        else
        {
            error.setType(QXmppStanza::Error::Auth);
            error.setCondition(QXmppStanza::Error::Forbidden);
        }
    }
    else
    {
        error.setType(QXmppStanza::Error::Cancel);
        error.setCondition(QXmppStanza::Error::ItemNotFound);
    }
    QXmppRpcErrorIq errorIq;
    errorIq.setId(iq.getId());
    errorIq.setTo(iq.getFrom());
    errorIq.setFrom( m_config.getJid());
    errorIq.setQuery( iq );
    errorIq.setError( error );
    m_stream->sendPacket( errorIq );
}

QXmppIbbTransferManager* QXmppClient::getIbbTransferManager() const
{
   return m_ibbTransferManager;
}
