/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include <QObject>
#include <QAbstractSocket>

#include "QXmppConfiguration.h"
#include "QXmppLogger.h"
#include "QXmppPresence.h"

class QXmppClientExtension;
class QXmppClientPrivate;
class QXmppPresence;
class QXmppMessage;
class QXmppPacket;
class QXmppIq;
class QXmppStream;

// managers
class QXmppDiscoveryIq;
class QXmppReconnectionManager;
class QXmppRosterManager;
class QXmppVCardManager;
class QXmppVersionManager;

/// \defgroup Core

/// \defgroup Managers

/// \brief The QXmppClient class is the main class for using QXmpp.
///
/// It provides the user all the required functionality to connect to the server
/// and perform operations afterwards.
///
/// This class will provide the handle/reference to QXmppRosterManager (roster management),
/// QXmppVCardManager (vCard manager), QXmppReconnectionManager (reconnection
/// mechanism) and QXmppVersionManager (software version information).
///
/// By default, a reconnection mechanism exists, which makes sure of reconnecting
/// to the server on disconnections due to an error. User can have a custom
/// reconnection mechanism as well.
///
/// Not all the managers or extensions have been enabled by default. One can
/// enable/disable the managers using the funtions addExtension() and
/// removeExtension(). findExtension() can be used to find reference/pointer to
/// particular instansiated and enabled manager.
///
/// List of managers enabled by default:
/// - QXmppRosterManager
/// - QXmppVCardManager
/// - QXmppVersionManager
/// - QXmppDiscoveryManager
/// - QXmppEntityTimeManager
///
/// \ingroup Core

class QXmppClient : public QXmppLoggable
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

    bool addExtension(QXmppClientExtension* extension);
    bool removeExtension(QXmppClientExtension* extension);

    QList<QXmppClientExtension*> extensions();

    /// \brief Returns the extension which can be cast into type T*, or 0
    /// if there is no such extension.
    ///
    /// Usage example:
    /// \code
    /// QXmppDiscoveryManager* ext = client->findExtension<QXmppDiscoveryManager>();
    /// if(ext)
    /// {
    ///     //extension found, do stuff...
    /// }
    /// \endcode
    ///
    template<typename T>
    T* findExtension()
    {
        QList<QXmppClientExtension*> list = extensions();
        for (int i = 0; i < list.size(); ++i)
        {
            T* extension = qobject_cast<T*>(list.at(i));
            if(extension)
                return extension;
        }
        return 0;
    }

    void connectToServer(const QXmppConfiguration&,
                         const QXmppPresence& initialPresence = 
                         QXmppPresence());
    void connectToServer(const QString &jid,
                         const QString &password);
    void disconnectFromServer();
    bool isConnected() const;

    QXmppPresence clientPresence() const;
    void setClientPresence(const QXmppPresence &presence);

    QXmppConfiguration &configuration();
    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    QAbstractSocket::SocketError socketError();
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppRosterManager& rosterManager();
    QXmppVCardManager& vCardManager();
    QXmppVersionManager& versionManager();

    QXmppReconnectionManager* reconnectionManager();
    bool setReconnectionManager(QXmppReconnectionManager*);

    /// \cond
    // FIXME: these methods are deprecated, their API is just too hard to read.
    // If you need this level of customisation, work directly with QXmppConfiguration.
    void Q_DECL_DEPRECATED connectToServer(const QString& host,
                         const QString& user,
                         const QString& password,
                         const QString& domain,
                         int port = 5222,
                         const QXmppPresence& initialPresence =
                         QXmppPresence());
    void Q_DECL_DEPRECATED connectToServer(const QString& host,
                         const QString& bareJid,
                         const QString& password,
                         int port = 5222,
                         const QXmppPresence& initialPresence =
                         QXmppPresence());

    // deprecated in release 0.2.0
    // deprecated accessors, use the form without "get" instead
    const QXmppPresence Q_DECL_DEPRECATED & getClientPresence() const;
    QXmppConfiguration Q_DECL_DEPRECATED & getConfiguration();
    const QXmppConfiguration Q_DECL_DEPRECATED & getConfiguration() const;
    QXmppReconnectionManager Q_DECL_DEPRECATED * getReconnectionManager();
    QXmppRosterManager Q_DECL_DEPRECATED & getRoster();
    QXmppVCardManager Q_DECL_DEPRECATED & getVCardManager();
    QAbstractSocket::SocketError Q_DECL_DEPRECATED getSocketError();
    QXmppStanza::Error::Condition Q_DECL_DEPRECATED getXmppStreamError();

    /// was clashing with QObject::disconnect use disconnectFromServer() instead
    void Q_DECL_DEPRECATED disconnect();

    // deprecated methods, use setClientPresence(QXmppPresence) instead.
    void Q_DECL_DEPRECATED setClientPresence(const QString& statusText);
    void Q_DECL_DEPRECATED setClientPresence(QXmppPresence::Type presenceType);
    void Q_DECL_DEPRECATED setClientPresence(QXmppPresence::Status::Type statusType);
    /// \endcond

signals:

    /// This signal is emitted when the client connects successfully to the XMPP
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
    /// QXmppRosterManager::rosterReceived(). After this signal, QXmppRosterManager object gets
    /// populated and you can use rosterManager() to get the handle of QXmppRosterManager object.
    ///
    void connected();

    /// This signal is emitted when the XMPP connection disconnects.
    ///
    void disconnected();

    /// This signal is emitted when the XMPP connection encounters any error.
    /// The QXmppClient::Error parameter specifies the type of error occurred.
    /// It could be due to TCP socket or the xml stream or the stanza.
    /// Depending upon the type of error occurred use the respective get function to
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

    /// \cond
    // Deprecated in release 0.3.0
    // Use QXmppDiscoveryManager::informationReceived(const QXmppDiscoveryIq&)
    // Notifies that an XMPP service discovery iq stanza is received.
    void discoveryIqReceived(const QXmppDiscoveryIq&);
    /// \endcond

public slots:
    bool sendPacket(const QXmppPacket&);
    void sendMessage(const QString& bareJid, const QString& message);

private slots:
    void slotElementReceived(const QDomElement &element, bool &handled);
    void xmppConnected();

private:
    QXmppClientPrivate * const d;
};

#endif // QXMPPCLIENT_H
