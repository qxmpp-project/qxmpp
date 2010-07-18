/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
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

#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHash>
#include <QVariant>

#include "QXmppConfiguration.h"
#include "QXmppLogger.h"
#include "QXmppPresence.h"

class QXmppStream;
class QXmppPresence;
class QXmppMessage;
class QXmppPacket;
class QXmppIq;
class QXmppMucManager;
class QXmppRoster;
class QXmppReconnectionManager;
class QXmppVCardManager;
class QXmppInvokable;
class QXmppRpcInvokeIq;
class QXmppRemoteMethod;
struct QXmppRemoteMethodResult;
class QXmppArchiveManager;
class QXmppDiscoveryIq;
class QXmppVCardManager;
class QXmppTransferManager;
class QXmppCallManager;

/// \defgroup Managers

/// \brief The QXmppClient class is the main class for using QXmpp.
///
/// It provides the user all the required functionality to connect to the server
/// and perform operations afterwards.
///
/// This class will provide the handle/reference to QXmppRoster (roster management),
/// QXmppVCardManager (vCard manager), QXmppReconnectionManager (reconnection
/// mechanism) and QXmppTransferManager (file transfers).
///
/// By default, a reconnection mechanism exists, which makes sure of reconnecting
/// to the server on disconnections due to an error. User can have a custom
/// reconnection mechanism as well.
///
/// For removing QXmpp dependency in QtGui, use DEFINE = QXMPP_NO_GUI
/// in the source.pro file and build as usual
///

class QXmppClient : public QObject
{
    Q_OBJECT

public:
    /// An enumeration for type of error.
    /// Error could come due a TCP socket or XML stream or due to various stanzas.
    enum Error
    {
        SocketError,        ///< Error due to TCP socket
        KeepAliveError,     ///< Error due to no response to a keep alive
        XmppStreamError,    ///< Error due to XML stream
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
    bool isConnected() const;

    QXmppArchiveManager& archiveManager();
    QXmppCallManager& callManager();
    QXmppMucManager& mucManager();
    QXmppTransferManager& transferManager();

    // FIXME : these accessors should be deprecated in favour of
    // versions without the "get".
    QXmppRoster& getRoster();
    QXmppConfiguration& getConfiguration();
    const QXmppConfiguration& getConfiguration() const;
    QXmppReconnectionManager* getReconnectionManager();
    bool setReconnectionManager(QXmppReconnectionManager*);
    const QXmppPresence& getClientPresence() const;
    QXmppVCardManager& getVCardManager();

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

    /// This signal is emitted when a raw XML element is received. You can
    /// connect to this signal if you want to handle raw XML elements yourself.
    ///
    /// WARNING: this signal is experimental and you can seriously disrupt
    /// packet handling when using it, so use with care and at your own risk.
    /// 
    /// Set 'handled' to true if you handled the element yourself and you wish
    /// to bypass normal handling for the element. If you do this, QXmpp will
    /// do absolutely no processing itself, so do not expect the usual signals
    /// to be emitted.
    void elementReceived(const QDomElement &element, bool &handled);

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

    /// Notifies that an XMPP service discovery iq stanza is received.
    void discoveryIqReceived(const QXmppDiscoveryIq&);

    /// This signal is emitted to send logging messages.
    void logMessage(QXmppLogger::MessageType type, const QString &msg);

public:
    QAbstractSocket::SocketError getSocketError();

    void addInvokableInterface( QXmppInvokable *interface );
    QXmppRemoteMethodResult callRemoteMethod( const QString &jid,
                                              const QString &interface,
                                              const QVariant &arg1 = QVariant(),
                                              const QVariant &arg2 = QVariant(),
                                              const QVariant &arg3 = QVariant(),
                                              const QVariant &arg4 = QVariant(),
                                              const QVariant &arg5 = QVariant(),
                                              const QVariant &arg6 = QVariant(),
                                              const QVariant &arg7 = QVariant(),
                                              const QVariant &arg8 = QVariant(),
                                              const QVariant &arg9 = QVariant(),
                                              const QVariant &arg10 = QVariant() );

    QXmppStanza::Error::Condition getXmppStreamError();

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

public slots:
    bool sendPacket(const QXmppPacket&);
    void sendMessage(const QString& bareJid, const QString& message);

    void setClientPresence(const QXmppPresence&);
    void setClientPresence(const QString& statusText);
    void setClientPresence(QXmppPresence::Type presenceType);
    void setClientPresence(QXmppPresence::Status::Type statusType);

private slots:
    void invokeInterfaceMethod( const QXmppRpcInvokeIq &iq );
    void xmppConnected();

private:
    QXmppStream* m_stream;  ///< Pointer to QXmppStream object a wrapper over
                            ///< TCP socket and XMPP protocol
    QXmppPresence m_clientPresence; ///< Stores the current presence of the connected client
    QXmppArchiveManager *m_archiveManager;  ///< Pointer to the archive manager
    QXmppCallManager *m_callManager;        ///< Pointer to the call manager
    QXmppMucManager* m_mucManager;          ///< Pointer to the multi-user chat manager
    QXmppReconnectionManager* m_reconnectionManager;    ///< Pointer to the reconnection manager
    QXmppRoster *m_roster;                  ///< Pointer to the roster manager
    QXmppTransferManager *m_transferManager;///< Pointer to the transfer manager
    QXmppVCardManager *m_vCardManager;      ///< Pointer to the vCard manager
    QHash<QString,QXmppInvokable *> m_interfaces;
};

#endif // QXMPPCLIENT_H
