/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppLogger.h"
#include "QXmppOutgoingClient.h"
#include "QXmppMessage.h"

#include "QXmppArchiveManager.h"
#include "QXmppCallManager.h"
#include "QXmppInvokable.h"
#include "QXmppMucManager.h"
#include "QXmppReconnectionManager.h"
#include "QXmppRpcIq.h"
#include "QXmppRemoteMethod.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"
#include "QXmppTransferManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"
#include "QXmppEntityTimeManager.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppConstants.h"

class QXmppClientPrivate
{
public:
    QXmppClientPrivate(QXmppClient *);

    QList<QXmppClientExtension*> extensions;
    QXmppLogger *logger;
    QXmppOutgoingClient* stream;  ///< Pointer to QXmppOutgoingClient object a wrapper over
                          ///< TCP socket and XMPP protocol
    QXmppPresence clientPresence; ///< Stores the current presence of the connected client

    QXmppArchiveManager *archiveManager;  ///< Pointer to the archive manager
    QXmppCallManager *callManager;        ///< Pointer to the call manager
    QXmppMucManager *mucManager;          ///< Pointer to the multi-user chat manager
    QXmppReconnectionManager *reconnectionManager;    ///< Pointer to the reconnection manager
    QXmppRosterManager *rosterManager;    ///< Pointer to the roster manager
    QXmppTransferManager *transferManager;///< Pointer to the transfer manager
    QXmppVCardManager *vCardManager;      ///< Pointer to the vCard manager
    QXmppVersionManager *versionManager;      ///< Pointer to the version manager

    QHash<QString,QXmppInvokable*> interfaces;

    void addProperCapability(QXmppPresence& presence);

    QXmppClient *client;
};

QXmppClientPrivate::QXmppClientPrivate(QXmppClient *parentClient)
    : stream(0),
    clientPresence(QXmppPresence::Available),
    reconnectionManager(0), client(parentClient)
{
}

void QXmppClientPrivate::addProperCapability(QXmppPresence& presence)
{
    QXmppDiscoveryManager* ext = client->findExtension<QXmppDiscoveryManager>();
    if(ext)
    {
        presence.setCapabilityHash("sha-1");
        presence.setCapabilityNode(QString(capabilities_node));
        presence.setCapabilityVer(ext->capabilities().verificationString());
    }
}

/// \mainpage
///
/// QXmpp is a cross-platform C++ XMPP client library based on the Qt
/// framework. It tries to use Qt's programming conventions in order to ease
/// the learning curve for new programmers.
///
/// QXmpp based clients are built using QXmppClient instances which handle the
/// establishment of the XMPP connection and provide a number of high-level
/// "managers" to perform specific tasks.
/// 
/// <B>Main Class:</B>
/// - QXmppClient
///
/// <B>Managers to perform specific tasks:</B>
/// - QXmppRosterManager
/// - QXmppVCardManager
/// - QXmppTransferManager
/// - QXmppMucManager
/// - QXmppCallManager
/// - QXmppArchiveManager
/// - QXmppVersionManager
/// - QXmppDiscoveryManager
/// - QXmppEntityTimeManager
///
/// <B>XMPP stanzas:</B> If you are interested in a more low-level API, you can refer to these
/// classes.
/// - QXmppIq
/// - QXmppMessage
/// - QXmppPresence
///
/// <BR><BR>
/// <B>Project Details:</B>
///
/// Project Page: http://code.google.com/p/qxmpp/
/// <BR>
/// Report Issues: http://code.google.com/p/qxmpp/issues/
/// <BR>
/// New Releases: http://code.google.com/p/qxmpp/downloads/
///

/// Creates a QXmppClient object.
/// \param parent is passed to the QObject's constructor.
/// The default value is 0.

QXmppClient::QXmppClient(QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppClientPrivate(this))
{
    d->stream = new QXmppOutgoingClient(this);
    d->addProperCapability(d->clientPresence);

    bool check = connect(d->stream, SIGNAL(elementReceived(const QDomElement&, bool&)),
                         this, SLOT(slotElementReceived(const QDomElement&, bool&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(messageReceived(const QXmppMessage&)),
                         this, SIGNAL(messageReceived(const QXmppMessage&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(presenceReceived(const QXmppPresence&)),
                    this, SIGNAL(presenceReceived(const QXmppPresence&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(iqReceived(const QXmppIq&)), this,
        SIGNAL(iqReceived(const QXmppIq&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(disconnected()), this,
        SIGNAL(disconnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(connected()), this,
        SLOT(xmppConnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(connected()), this,
        SIGNAL(connected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(error(QXmppClient::Error)), this,
        SIGNAL(error(QXmppClient::Error)));
    Q_ASSERT(check);

    check = setReconnectionManager(new QXmppReconnectionManager(this));
    Q_ASSERT(check);

    // rpc
    check = connect(d->stream, SIGNAL(rpcCallInvoke(QXmppRpcInvokeIq)),
        this, SLOT(invokeInterfaceMethod(QXmppRpcInvokeIq)));
    Q_ASSERT(check);

    // logging
    d->logger = 0;
    setLogger(QXmppLogger::getLogger());

    // create managers
    // TODO move manager references to d->extensions
    d->rosterManager = new QXmppRosterManager(this);
    addExtension(d->rosterManager);

    d->archiveManager = new QXmppArchiveManager;
    addExtension(d->archiveManager);

#if 0
    d->callManager = new QXmppCallManager(this);
    addExtension(d->callManager);
#endif

    d->mucManager = new QXmppMucManager(this);
    addExtension(d->mucManager);

    d->transferManager = new QXmppTransferManager(this);
    addExtension(d->transferManager);

    d->vCardManager = new QXmppVCardManager;
    addExtension(d->vCardManager);

    d->versionManager = new QXmppVersionManager;
    addExtension(d->versionManager);

    addExtension(new QXmppEntityTimeManager());

    QXmppDiscoveryManager *discoveryManager = new QXmppDiscoveryManager;
    addExtension(discoveryManager);

    // obsolete signal
    check = connect(discoveryManager, SIGNAL(infoReceived(QXmppDiscoveryIq)),
                    this, SIGNAL(discoveryIqReceived(QXmppDiscoveryIq)));
    Q_ASSERT(check);

    check = connect(discoveryManager, SIGNAL(itemsReceived(QXmppDiscoveryIq)),
                    this, SIGNAL(discoveryIqReceived(QXmppDiscoveryIq)));
    Q_ASSERT(check);
}

/// Destructor, destroys the QXmppClient object.
///

QXmppClient::~QXmppClient()
{
    delete d;
}

/// Registers a new extension with the client.
///
/// \param extension

bool QXmppClient::addExtension(QXmppClientExtension* extension)
{
    if (d->extensions.contains(extension))
    {
        qWarning("Cannot add extension, it has already been added");
        return false;
    }

    extension->setParent(this);
    extension->setClient(this);

    // Features
    d->stream->addFeatures(extension->discoveryFeatures());

    d->extensions << extension;
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
    if(!config.autoReconnectionEnabled())
    {
        delete d->reconnectionManager;
        d->reconnectionManager = 0;
    }

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
    config.setUser(jidToUser(jid));
    config.setDomain(jidToDomain(jid));
    config.setPassword(password);
    connectToServer(config);
}

/// Overloaded function.
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
/// \param user Username of the account at the specified XMPP server. It should
/// be the name without the domain name. E.g. "qxmpp.test1" and not
/// "qxmpp.test1@gmail.com"
/// \param password Password for the specified username
/// \param domain Domain name e.g. "gmail.com" and "jabber.org".
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QString& host, const QString& user,
                                  const QString& password, const QString& domain,
                                  int port,
                                  const QXmppPresence& initialPresence)
{
    QXmppConfiguration &config = d->stream->configuration();
    config.setHost(host);
    config.setUser(user);
    config.setPassword(password);
    config.setDomain(domain);
    config.setPort(port);
    connectToServer(config, initialPresence);
}

/// Overloaded function.
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
/// \param bareJid BareJid of the account at the specified XMPP server.
/// (e.g. "qxmpp.test1@gmail.com" or qxmpptest@jabber.org.)
/// \param password Password for the specified username
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QString& host,
                                  const QString& bareJid,
                                  const QString& password,
                                  int port,
                                  const QXmppPresence& initialPresence)
{
    QXmppConfiguration config;
    config.setHost(host);
    config.setUser(jidToUser(bareJid));
    config.setDomain(jidToDomain(bareJid));
    config.setPassword(password);
    config.setPort(port);
    connectToServer(config, initialPresence);
}

/// After successfully connecting to the server use this function to send
/// stanzas to the server. This function can solely be used to send various kind
/// of stanzas to the server. QXmppPacket is a parent class of all the stanzas
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

bool QXmppClient::sendPacket(const QXmppPacket& packet)
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
    d->clientPresence.setType(QXmppPresence::Unavailable);
    d->clientPresence.status().setType(QXmppPresence::Status::Offline);
    d->clientPresence.status().setStatusText("Logged out");
    if (d->stream->isConnected())
    {
        sendPacket(d->clientPresence);
    }
    d->stream->disconnectFromHost();
}

/// Returns true if the client is connected to the XMPP server.
///

bool QXmppClient::isConnected() const
{
    return d->stream && d->stream->isConnected();
}

/// Returns the reference to QXmppRosterManager object of the client.
/// \return Reference to the roster object of the connected client. Use this to
/// get the list of friends in the roster and their presence information.
///

QXmppRosterManager& QXmppClient::rosterManager()
{
    return *d->rosterManager;
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
    if (presence.type() == QXmppPresence::Unavailable)
    {
        d->clientPresence = presence;

        // NOTE: we can't call disconnect() because it alters 
        // the client presence
        if (d->stream->isConnected())
        {
            sendPacket(d->clientPresence);
            d->stream->disconnectFromHost();
        }
    }
    else if (!d->stream->isConnected())
        connectToServer(d->stream->configuration(), presence);
    else
    {
        d->clientPresence = presence;
        d->addProperCapability(d->clientPresence);
        sendPacket(d->clientPresence);
    }
}

/// Function to get reconnection manager. By default there exists a reconnection
/// manager. See QXmppReconnectionManager for more details of the reconnection
/// mechanism.
///
/// \return Pointer to QXmppReconnectionManager
///

QXmppReconnectionManager* QXmppClient::reconnectionManager()
{
    return d->reconnectionManager;
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

    if(d->reconnectionManager)
        delete d->reconnectionManager;

    d->reconnectionManager = reconnectionManager;

    bool check = connect(this, SIGNAL(connected()), d->reconnectionManager,
                         SLOT(connected()));
    Q_ASSERT(check);
    if(!check)
        return false;

    check = connect(this, SIGNAL(error(QXmppClient::Error)),
                    d->reconnectionManager, SLOT(error(QXmppClient::Error)));
    Q_ASSERT(check);
    if(!check)
        return false;

    return true;
}

/// Returns the socket error if error() is QXmppClient::SocketError.
///

QAbstractSocket::SocketError QXmppClient::socketError()
{
    return d->stream->socketError();
}

/// Returns the XMPP stream error if QXmppClient::Error is QXmppClient::XmppStreamError.
///

QXmppStanza::Error::Condition QXmppClient::xmppStreamError()
{
    return d->stream->xmppStreamError();
}

/// Returns the reference to QXmppVCardManager, implimentation of XEP-0054.
/// http://xmpp.org/extensions/xep-0054.html
///

QXmppVCardManager& QXmppClient::vCardManager()
{
    return *d->vCardManager;
}

QXmppVersionManager& QXmppClient::versionManager()
{
    return *d->versionManager;
}

/// Give extensions a chance to handle incoming stanzas.
///
/// \param element
/// \param handled

void QXmppClient::slotElementReceived(const QDomElement &element, bool &handled)
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

void QXmppClient::addInvokableInterface( QXmppInvokable *interface )
{
    d->interfaces[ interface->metaObject()->className() ] = interface;
}


void QXmppClient::invokeInterfaceMethod( const QXmppRpcInvokeIq &iq )
{
    QXmppStanza::Error error;
    
    const QStringList methodBits = iq.method().split('.');
    if (methodBits.size() != 2)
        return;
    const QString interface = methodBits.first();
    const QString method = methodBits.last();
    QXmppInvokable *iface = d->interfaces.value(interface);
    if (iface)
    {
        if ( iface->isAuthorized( iq.from() ) )
        {

            if ( iface->interfaces().contains(method) )
            {
                QVariant result = iface->dispatch(method.toLatin1(),
                                                  iq.arguments() );
                QXmppRpcResponseIq resultIq;
                resultIq.setId(iq.id());
                resultIq.setTo(iq.from());
                resultIq.setFrom(d->stream->configuration().jid());
                resultIq.setValues(QVariantList() << result);
                d->stream->sendPacket( resultIq );
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
    errorIq.setId(iq.id());
    errorIq.setTo(iq.from());
    errorIq.setFrom(d->stream->configuration().jid());
    errorIq.setQuery( iq );
    errorIq.setError( error );
    d->stream->sendPacket( errorIq );
}

QXmppRemoteMethodResult QXmppClient::callRemoteMethod( const QString &jid,
                                          const QString &interface,
                                          const QVariant &arg1,
                                          const QVariant &arg2,
                                          const QVariant &arg3,
                                          const QVariant &arg4,
                                          const QVariant &arg5,
                                          const QVariant &arg6,
                                          const QVariant &arg7,
                                          const QVariant &arg8,
                                          const QVariant &arg9,
                                          const QVariant &arg10 )
{
    QVariantList args;
    if( arg1.isValid() ) args << arg1;
    if( arg2.isValid() ) args << arg2;
    if( arg3.isValid() ) args << arg3;
    if( arg4.isValid() ) args << arg4;
    if( arg5.isValid() ) args << arg5;
    if( arg6.isValid() ) args << arg6;
    if( arg7.isValid() ) args << arg7;
    if( arg8.isValid() ) args << arg8;
    if( arg9.isValid() ) args << arg9;
    if( arg10.isValid() ) args << arg10;

    QXmppRemoteMethod method( jid, interface, args, this );
    connect( d->stream, SIGNAL(rpcCallResponse(QXmppRpcResponseIq)),
             &method, SLOT(gotResult(QXmppRpcResponseIq)));
    connect( d->stream, SIGNAL(rpcCallError(QXmppRpcErrorIq)),
             &method, SLOT(gotError(QXmppRpcErrorIq)));


    return method.call();
}

/// Returns the reference to QXmppArchiveManager, implementation of XEP-0136.
/// http://xmpp.org/extensions/xep-0136.html
///

QXmppArchiveManager& QXmppClient::archiveManager()
{
    return *d->archiveManager;
}

/// Returns the reference to QXmppCallManager, implementation of XEP-0166.
/// http://xmpp.org/extensions/xep-0166.html
///

QXmppCallManager& QXmppClient::callManager()
{
    return *d->callManager;
}

/// Returns the reference to QXmppMucManager, implementation of XEP-0045.
/// http://xmpp.org/extensions/xep-045.html
///
QXmppMucManager& QXmppClient::mucManager()
{
    return *d->mucManager;
}

/// Returns the reference to QXmppTransferManager, implementation of:
///
///  * XEP-0047: In-Band Bytestreams
///  * XEP-0095: Stream Initiation
///  * XEP-0096: SI File Transfer
///

QXmppTransferManager& QXmppClient::transferManager()
{
    return *d->transferManager;
}

/// Returns the QXmppLogger associated with the current QXmppClient.

QXmppLogger *QXmppClient::logger()
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the current QXmppClient.

void QXmppClient::setLogger(QXmppLogger *logger)
{
    if (d->logger)
        QObject::disconnect(this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
                   d->logger, SLOT(log(QXmppLogger::MessageType, QString)));
    d->logger = logger;
    if (d->logger)
        connect(this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
                d->logger, SLOT(log(QXmppLogger::MessageType, QString)));
}

/// At connection establishment, send initial presence.

void QXmppClient::xmppConnected()
{
    sendPacket(d->clientPresence);
}

// deprecated functions
const QXmppPresence& QXmppClient::getClientPresence() const
{
    return d->clientPresence;
}

QXmppConfiguration& QXmppClient::getConfiguration()
{
    return d->stream->configuration();
}

const QXmppConfiguration& QXmppClient::getConfiguration() const
{
    return d->stream->configuration();
}

QXmppRosterManager& QXmppClient::getRoster()
{
    return *d->rosterManager;
}

QAbstractSocket::SocketError QXmppClient::getSocketError()
{
    return d->stream->socketError();
}

QXmppVCardManager& QXmppClient::getVCardManager()
{
    return *d->vCardManager;
}

QXmppStanza::Error::Condition QXmppClient::getXmppStreamError()
{
    return d->stream->xmppStreamError();
}

void QXmppClient::disconnect()
{
    disconnectFromServer();
}

void QXmppClient::setClientPresence(const QString& statusText)
{
    QXmppPresence newPresence = d->clientPresence;
    newPresence.status().setStatusText(statusText);
    setClientPresence(newPresence);
}

void QXmppClient::setClientPresence(QXmppPresence::Type presenceType)
{
    QXmppPresence newPresence = d->clientPresence;
    newPresence.setType(presenceType);
    setClientPresence(newPresence);
}

void QXmppClient::setClientPresence(QXmppPresence::Status::Type statusType)
{
    QXmppPresence newPresence = d->clientPresence;
    if (statusType == QXmppPresence::Status::Offline)
        newPresence.setType(QXmppPresence::Unavailable);
    else
        newPresence.setType(QXmppPresence::Available);
    newPresence.status().setType(statusType);
    setClientPresence(newPresence);
}

QXmppReconnectionManager* QXmppClient::getReconnectionManager()
{
    return d->reconnectionManager;
}
