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

/// \class QXmppClient
/// \brief The QXmppClient is the main class for using QXmpp.
///
/// It provides the user all the required functionality to connect to the server
/// and perform operations afterwards.
///
/// This class will provide the handle/reference to QXmppRoster (roster management),
///  QXmppVCardManager (vCard manager) and QXmppReconnectionManager (reconnection
/// mechanism).
///
/// By default, a reconnection mechanism exists, which makes sure of reconnecting
/// to the server on dissconnections due to an error. User can have a custom
/// reconnection mechanism as well.
///

#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHash>
#include <QVariant>

#include "QXmppConfiguration.h"
#include "QXmppPresence.h"

class QXmppStream;
class QXmppPresence;
class QXmppMessage;
class QXmppPacket;
class QXmppIq;
class QXmppRoster;
class QXmppReconnectionManager;
class QXmppVCardManager;
class QXmppInvokable;
class QXmppRpcInvokeIq;
class QXmppRemoteMethod;
class QXmppIbbTransferManager;

class QXmppClient : public QObject
{
    Q_OBJECT

public:
    /// An enumeration for type of error.
    /// Error could come due a TCP socket or XML stream or due to various stanzas.
    enum Error
    {
        SocketError,        ///< Error due to TCP socket
        XmppStreamError,    ///< Error due to XML stream
        XmppStanzaError     ///< Error due to stanza
    };

    QXmppClient(QObject *parent = 0);
    ~QXmppClient();
    void connectToServer(const QString& host,
                         const QString& user,
                         const QString& passwd,
                         const QString& domain,
                         int port = 5222,
                         const QXmppPresence& initialPresence =
                         QXmppPresence());
    void connectToServer(const QString& host,
                         const QString& bareJid,
                         const QString& passwd,
                         int port = 5222,
                         const QXmppPresence& initialPresence =
                         QXmppPresence());
    void connectToServer(const QXmppConfiguration&,
                         const QXmppPresence& initialPresence = 
                         QXmppPresence());
    void disconnect();
    QXmppRoster& getRoster();
    QXmppConfiguration& getConfiguration();
    const QXmppConfiguration& getConfiguration() const;
    QXmppReconnectionManager* getReconnectionManager();
    bool setReconnectionManager(QXmppReconnectionManager*);
    const QXmppPresence& getClientPresence() const;
    QXmppVCardManager& getVCardManager();
    QXmppIbbTransferManager* getIbbTransferManager() const;

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

    /// Notifies that an XMPP presence stanza is received. The QXmppPresence
    /// parameter contains the details of the presence sent to this client.
    /// This signal is emitted when someone login/logout or when someone's status
    /// changes Busy, Idle, Invisible etc.
    void presenceReceived(const QXmppPresence&);

    /// Notifies that an XMPP iq stanza is received. The QXmppIq
    /// parameter contains the details of the iq sent to this client.
    /// IQ stanzas provide a structured request-response mechanism. Roster
    /// management, setting-getting vCards etc is done using iq stanzas.
    void iqReceived(const QXmppIq&);

public:
    QAbstractSocket::SocketError getSocketError();
    void addInvokableInterface( QXmppInvokable *interface );
    void invokeInterfaceMethod( const QXmppRpcInvokeIq &iq );

//    QXmppStanza::Error getXmppStreamError();

public slots:
    void sendPacket(const QXmppPacket&);
    void sendMessage(const QString& bareJid, const QString& message);

    void setClientPresence(const QXmppPresence&);
    void setClientPresence(const QString& statusText);
    void setClientPresence(QXmppPresence::Type presenceType);
    void setClientPresence(QXmppPresence::Status::Type statusType);

private:
    QXmppStream* m_stream;  ///< Pointer to QXmppStream object a wrapper over
                            ///< TCP socket and XMPP protocol
    QXmppConfiguration m_config;    ///< This object provides the configuration
                                    ///< required for connecting to the XMPP server.
    QXmppPresence m_clientPrecence; ///< Stores the current presence of connected client
    QXmppReconnectionManager* m_reconnectionManager;    ///< Pointer to the reconnection manager
    QHash<QString,QXmppInvokable *> m_interfaces;
    QXmppIbbTransferManager* m_ibbTransferManager;    ///< Pointer to the ibb transfer manager
};

#endif // QXMPPCLIENT_H
