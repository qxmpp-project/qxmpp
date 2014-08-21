/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#include <QSslSocket>
#include <QTimer>

#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppConstants.h"
#include "QXmppLogger.h"
#include "QXmppOutgoingClient.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"
#include "QXmppEntityTimeManager.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppDiscoveryIq.h"

class QXmppClientPrivate
{
public:
    QXmppClientPrivate(QXmppClient *qq);

    QXmppPresence clientPresence;                   ///< Current presence of the client
    QList<QXmppClientExtension*> extensions;
    QXmppLogger *logger;
    QXmppOutgoingClient *stream;                    ///< Pointer to the XMPP stream

    // reconnection
    bool receivedConflict;
    int reconnectionTries;
    QTimer *reconnectionTimer;

    void addProperCapability(QXmppPresence& presence);
    int getNextReconnectTime() const;

private:
    QXmppClient *q;
};

QXmppClientPrivate::QXmppClientPrivate(QXmppClient *qq)
    : clientPresence(QXmppPresence::Available)
    , logger(0)
    , stream(0)
    , receivedConflict(false)
    , reconnectionTries(0)
    , reconnectionTimer(0)
    , q(qq)
{
}

void QXmppClientPrivate::addProperCapability(QXmppPresence& presence)
{
    QXmppDiscoveryManager* ext = q->findExtension<QXmppDiscoveryManager>();
    if(ext) {
        presence.setCapabilityHash("sha-1");
        presence.setCapabilityNode(ext->clientCapabilitiesNode());
        presence.setCapabilityVer(ext->capabilities().verificationString());
    }
}

int QXmppClientPrivate::getNextReconnectTime() const
{
    if (reconnectionTries < 5)
        return 10 * 1000;
    else if (reconnectionTries < 10)
        return 20 * 1000;
    else if (reconnectionTries < 15)
        return 40 * 1000;
    else
        return 60 * 1000;
}

/// Creates a QXmppClient object.
/// \param parent is passed to the QObject's constructor.
/// The default value is 0.

QXmppClient::QXmppClient(QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppClientPrivate(this))
{
    bool check;
    Q_UNUSED(check);

    d->stream = new QXmppOutgoingClient(this);
    d->addProperCapability(d->clientPresence);

    check = connect(d->stream, SIGNAL(elementReceived(QDomElement,bool&)),
                    this, SLOT(_q_elementReceived(QDomElement,bool&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(messageReceived(QXmppMessage)),
                    this, SIGNAL(messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SIGNAL(presenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(iqReceived(QXmppIq)),
                    this, SIGNAL(iqReceived(QXmppIq)));
    Q_ASSERT(check);

    check = connect(d->stream->socket(), SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                    this, SLOT(_q_socketStateChanged(QAbstractSocket::SocketState)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(connected()),
                    this, SLOT(_q_streamConnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(disconnected()),
                    this, SLOT(_q_streamDisconnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(error(QXmppClient::Error)),
                    this, SLOT(_q_streamError(QXmppClient::Error)));
    Q_ASSERT(check);

    // reconnection
    d->reconnectionTimer = new QTimer(this);
    d->reconnectionTimer->setSingleShot(true);
    connect(d->reconnectionTimer, SIGNAL(timeout()),
            this, SLOT(_q_reconnect()));
    Q_ASSERT(check);

    // logging
    setLogger(QXmppLogger::getLogger());

    addExtension(new QXmppRosterManager(this));
    addExtension(new QXmppVCardManager);
    addExtension(new QXmppVersionManager);
    addExtension(new QXmppEntityTimeManager());
    addExtension(new QXmppDiscoveryManager());
}

/// Destructor, destroys the QXmppClient object.
///

QXmppClient::~QXmppClient()
{
    delete d;
}

/// Registers a new \a extension with the client.
///
/// \param extension

bool QXmppClient::addExtension(QXmppClientExtension* extension)
{
    return insertExtension(d->extensions.size(), extension);
}

/// Registers a new \a extension with the client at the given \a index.
///
/// \param index
/// \param extension

bool QXmppClient::insertExtension(int index, QXmppClientExtension *extension)
{
    if (d->extensions.contains(extension))
    {
        qWarning("Cannot add extension, it has already been added");
        return false;
    }

    extension->setParent(this);
    extension->setClient(this);
    d->extensions.insert(index, extension);
    return true;
}

/// Unregisters the given extension from the client. If the extension
/// is found, it will be destroyed.
///
/// \param extension

bool QXmppClient::removeExtension(QXmppClientExtension* extension)
{
    if (d->extensions.contains(extension))
    {
        d->extensions.removeAll(extension);
        delete extension;
        return true;
    } else {
        qWarning("Cannot remove extension, it was never added");
        return false;
    }
}

/// Returns a list containing all the client's extensions.
///

QList<QXmppClientExtension*> QXmppClient::extensions()
{
    return d->extensions;
}

/// Returns a modifiable reference to the current configuration of QXmppClient.
/// \return Reference to the QXmppClient's configuration for the connection.

QXmppConfiguration& QXmppClient::configuration()
{
    return d->stream->configuration();
}

/// Attempts to connect to the XMPP server. Server details and other configurations
/// are specified using the config parameter. Use signals connected(), error(QXmppClient::Error)
/// and disconnected() to know the status of the connection.
/// \param config Specifies the configuration object for connecting the XMPP server.
/// This contains the host name, user, password etc. See QXmppConfiguration for details.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QXmppConfiguration& config,
                                  const QXmppPresence& initialPresence)
{
    d->stream->configuration() = config;
    d->clientPresence = initialPresence;
    d->addProperCapability(d->clientPresence);

    d->stream->connectToHost();
}

/// Overloaded function to simply connect to an XMPP server with a JID and password.
///
/// \param jid JID for the account.
/// \param password Password for the account.

void QXmppClient::connectToServer(const QString &jid, const QString &password)
{
    QXmppConfiguration config;
    config.setJid(jid);
    config.setPassword(password);
    connectToServer(config);
}

/// After successfully connecting to the server use this function to send
/// stanzas to the server. This function can solely be used to send various kind
/// of stanzas to the server. QXmppStanza is a parent class of all the stanzas
/// QXmppMessage, QXmppPresence, QXmppIq, QXmppBind, QXmppRosterIq, QXmppSession
/// and QXmppVCard.
///
/// \return Returns true if the packet was sent, false otherwise.
///
/// Following code snippet illustrates how to send a message using this function:
/// \code
/// QXmppMessage message(from, to, message);
/// client.sendPacket(message);
/// \endcode
///
/// \param packet A valid XMPP stanza. It can be an iq, a message or a presence stanza.
///

bool QXmppClient::sendPacket(const QXmppStanza& packet)
{
    return d->stream->sendPacket(packet);
}

/// Disconnects the client and the current presence of client changes to
/// QXmppPresence::Unavailable and status text changes to "Logged out".
///
/// \note Make sure that the clientPresence is changed to
/// QXmppPresence::Available, if you are again calling connectToServer() after
/// calling the disconnectFromServer() function.
///

void QXmppClient::disconnectFromServer()
{
    // cancel reconnection
    d->reconnectionTimer->stop();

    d->clientPresence.setType(QXmppPresence::Unavailable);
    d->clientPresence.setStatusText("Logged out");
    if (d->stream->isConnected())
        sendPacket(d->clientPresence);

    d->stream->disconnectFromHost();
}

/// Returns true if the client has authenticated with the XMPP server.

bool QXmppClient::isAuthenticated() const
{
    return d->stream->isAuthenticated();
}

/// Returns true if the client is connected to the XMPP server.
///

bool QXmppClient::isConnected() const
{
    return d->stream->isConnected();
}

/// Returns the reference to QXmppRosterManager object of the client.
/// \return Reference to the roster object of the connected client. Use this to
/// get the list of friends in the roster and their presence information.
///

QXmppRosterManager& QXmppClient::rosterManager()
{
    return *findExtension<QXmppRosterManager>();
}

/// Utility function to send message to all the resources associated with the
/// specified bareJid. If there are no resources available, that is the contact
/// is offline or not present in the roster, it will still send a message to
/// the bareJid.
///
/// \param bareJid bareJid of the receiving entity
/// \param message Message string to be sent.

void QXmppClient::sendMessage(const QString& bareJid, const QString& message)
{
    QStringList resources = rosterManager().getResources(bareJid);
    if(!resources.isEmpty())
    {
        for(int i = 0; i < resources.size(); ++i)
        {
            sendPacket(QXmppMessage("", bareJid + "/" + resources.at(i), message));
        }
    }
    else
    {
        sendPacket(QXmppMessage("", bareJid, message));
    }
}

/// Returns the client's current state.

QXmppClient::State QXmppClient::state() const
{
    if (d->stream->isConnected())
        return QXmppClient::ConnectedState;
    else if (d->stream->socket()->state() != QAbstractSocket::UnconnectedState &&
             d->stream->socket()->state() != QAbstractSocket::ClosingState)
        return QXmppClient::ConnectingState;
    else
        return QXmppClient::DisconnectedState;
}

/// Returns the client's current presence.
///

QXmppPresence QXmppClient::clientPresence() const
{
    return d->clientPresence;
}

/// Changes the presence of the connected client.
///
/// The connection to the server will be updated accordingly:
///
/// \li If the presence type is QXmppPresence::Unavailable, the connection
/// to the server will be closed.
///
/// \li Otherwise, the connection to the server will be established
/// as needed.
///
/// \param presence QXmppPresence object
///

void QXmppClient::setClientPresence(const QXmppPresence& presence)
{
    d->clientPresence = presence;
    d->addProperCapability(d->clientPresence);

    if (presence.type() == QXmppPresence::Unavailable)
    {
        // cancel reconnection
        d->reconnectionTimer->stop();

        // NOTE: we can't call disconnect() because it alters
        // the client presence
        if (d->stream->isConnected())
            sendPacket(d->clientPresence);

        d->stream->disconnectFromHost();
    }
    else if (d->stream->isConnected())
        sendPacket(d->clientPresence);
    else
        connectToServer(d->stream->configuration(), presence);
}

/// Returns the socket error if error() is QXmppClient::SocketError.
///

QAbstractSocket::SocketError QXmppClient::socketError()
{
    return d->stream->socket()->error();
}

/// Returns the human-readable description of the last socket error if error() is QXmppClient::SocketError.
///

QString QXmppClient::socketErrorString() const
{
    return d->stream->socket()->errorString();
}

/// Returns the XMPP stream error if QXmppClient::Error is QXmppClient::XmppStreamError.
///

QXmppStanza::Error::Condition QXmppClient::xmppStreamError()
{
    return d->stream->xmppStreamError();
}

/// Returns the reference to QXmppVCardManager, implementation of XEP-0054.
/// http://xmpp.org/extensions/xep-0054.html
///

QXmppVCardManager& QXmppClient::vCardManager()
{
    return *findExtension<QXmppVCardManager>();
}

/// Returns the reference to QXmppVersionManager, implementation of XEP-0092.
/// http://xmpp.org/extensions/xep-0092.html
///

QXmppVersionManager& QXmppClient::versionManager()
{
    return *findExtension<QXmppVersionManager>();
}

/// Give extensions a chance to handle incoming stanzas.
///
/// \param element
/// \param handled

void QXmppClient::_q_elementReceived(const QDomElement &element, bool &handled)
{
    foreach (QXmppClientExtension *extension, d->extensions)
    {
        if (extension->handleStanza(element))
        {
            handled = true;
            return;
        }
    }
}

void QXmppClient::_q_reconnect()
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        debug("Reconnecting to server");
        d->stream->connectToHost();
    }
}

void QXmppClient::_q_socketStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState);
    emit stateChanged(state());
}

/// At connection establishment, send initial presence.

void QXmppClient::_q_streamConnected()
{
    d->receivedConflict = false;
    d->reconnectionTries = 0;

    // notify managers
    emit connected();
    emit stateChanged(QXmppClient::ConnectedState);

    // send initial presence
    if (d->stream->isAuthenticated())
        sendPacket(d->clientPresence);
}

void QXmppClient::_q_streamDisconnected()
{
    // notify managers
    emit disconnected();
    emit stateChanged(QXmppClient::DisconnectedState);
}

void QXmppClient::_q_streamError(QXmppClient::Error err)
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        if (err == QXmppClient::XmppStreamError) {
            // if we receive a resource conflict, inhibit reconnection
            if (d->stream->xmppStreamError() == QXmppStanza::Error::Conflict)
                d->receivedConflict = true;
        } else if (err == QXmppClient::SocketError && !d->receivedConflict) {
            // schedule reconnect
            d->reconnectionTimer->start(d->getNextReconnectTime());
        } else if (err == QXmppClient::KeepAliveError) {
            // if we got a keepalive error, reconnect in one second
            d->reconnectionTimer->start(1000);
        }
    }

    // notify managers
    emit error(err);
}

/// Returns the QXmppLogger associated with the current QXmppClient.

QXmppLogger *QXmppClient::logger() const
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the current QXmppClient.

void QXmppClient::setLogger(QXmppLogger *logger)
{
    if (logger != d->logger) {
        if (d->logger) {
            disconnect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                       d->logger, SLOT(log(QXmppLogger::MessageType,QString)));
            disconnect(this, SIGNAL(setGauge(QString,double)),
                       d->logger, SLOT(setGauge(QString,double)));
            disconnect(this, SIGNAL(updateCounter(QString,qint64)),
                       d->logger, SLOT(updateCounter(QString,qint64)));
        }

        d->logger = logger;
        if (d->logger) {
            connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                    d->logger, SLOT(log(QXmppLogger::MessageType,QString)));
            connect(this, SIGNAL(setGauge(QString,double)),
                    d->logger, SLOT(setGauge(QString,double)));
            connect(this, SIGNAL(updateCounter(QString,qint64)),
                    d->logger, SLOT(updateCounter(QString,qint64)));
        }

        emit loggerChanged(d->logger);
    }
}

