/*
 * Copyright (C) 2008-2011 The QXmpp developers
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

#include <QDomElement>
#include <QFileInfo>
#include <QPluginLoader>
#include <QSslSocket>

#include "QXmppConstants.h"
#include "QXmppDialback.h"
#include "QXmppIq.h"
#include "QXmppIncomingClient.h"
#include "QXmppIncomingServer.h"
#include "QXmppOutgoingServer.h"
#include "QXmppPresence.h"
#include "QXmppServer.h"
#include "QXmppServerExtension.h"
#include "QXmppServerPlugin.h"
#include "QXmppUtils.h"

// Core plugins
Q_IMPORT_PLUGIN(mod_disco)
Q_IMPORT_PLUGIN(mod_ping)
Q_IMPORT_PLUGIN(mod_proxy65)
Q_IMPORT_PLUGIN(mod_stats)
Q_IMPORT_PLUGIN(mod_time)
Q_IMPORT_PLUGIN(mod_version)

class QXmppServerPrivate
{
public:
    QXmppServerPrivate();
    void loadExtensions(QXmppServer *server);
    QStringList presenceSubscribers(const QString &jid);
    QStringList presenceSubscriptions(const QString &jid);
    void startExtensions();
    void stopExtensions();

    void info(const QString &message);
    void warning(const QString &message);

    QString domain;
    QList<QXmppServerExtension*> extensions;
    QMap<QString, QMap<QString, QXmppPresence> > presences;
    QMap<QString, QSet<QString> > subscribers;
    QXmppLogger *logger;
    QXmppPasswordChecker *passwordChecker;

    // client-to-server
    QXmppSslServer *serverForClients;
    QList<QXmppIncomingClient*> incomingClients;

    // server-to-server
    QList<QXmppIncomingServer*> incomingServers;
    QList<QXmppOutgoingServer*> outgoingServers;
    QXmppSslServer *serverForServers;
    QMap<QXmppStream*, QList<QByteArray> > queues;

private:
    bool loaded;
    bool started;
};

QXmppServerPrivate::QXmppServerPrivate()
    : logger(0),
    passwordChecker(0),
    loaded(false),
    started(false)
{
}

void QXmppServerPrivate::info(const QString &message)
{
    if (logger)
        logger->log(QXmppLogger::InformationMessage, message);
}

void QXmppServerPrivate::warning(const QString &message)
{
    if (logger)
        logger->log(QXmppLogger::WarningMessage, message);
}

/// Load the server's extensions.
///
/// \param server

void QXmppServerPrivate::loadExtensions(QXmppServer *server)
{
    if (!loaded)
    {
        QObjectList plugins = QPluginLoader::staticInstances();
        foreach (QObject *object, plugins)
        {
            QXmppServerPlugin *plugin = qobject_cast<QXmppServerPlugin*>(object);
            if (!plugin)
                continue;

            foreach (const QString &key, plugin->keys())
                server->addExtension(plugin->create(key));
        }
        loaded = true;
    }
}

QStringList QXmppServerPrivate::presenceSubscribers(const QString &jid)
{
    // start with directed presences
    QSet<QString> recipients = subscribers.value(jid);

    // query extensions
    foreach (QXmppServerExtension *extension, extensions)
    {
        const QStringList extras = extension->presenceSubscribers(jid);
        foreach (const QString &extra, extras)
            recipients.insert(extra);
    }

    return recipients.toList();
}

QStringList QXmppServerPrivate::presenceSubscriptions(const QString &jid)
{
    // FIXME : start with directed presences?
    QSet<QString> recipients;

    // query extensions
    foreach (QXmppServerExtension *extension, extensions)
    {
        const QStringList extras = extension->presenceSubscriptions(jid);
        foreach (const QString &extra, extras)
            recipients.insert(extra);
    }

    return recipients.toList();
}

/// Start the server's extensions.

void QXmppServerPrivate::startExtensions()
{
    if (!started)
    {
        foreach (QXmppServerExtension *extension, extensions)
            if (!extension->start())
                warning(QString("Could not start extension %1").arg(extension->extensionName()));
        started = true;
    }
}

/// Stop the server's extensions (in reverse order).
///

void QXmppServerPrivate::stopExtensions()
{
    if (started)
    {
        for (int i = extensions.size() - 1; i >= 0; --i)
            extensions[i]->stop();
        started = false;
    }
}

/// Constructs a new XMPP server instance.
///
/// \param parent

QXmppServer::QXmppServer(QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppServerPrivate)
{
    d->serverForClients = new QXmppSslServer(this);
    bool check = connect(d->serverForClients, SIGNAL(newConnection(QSslSocket*)),
                         this, SLOT(slotClientConnection(QSslSocket*)));
    Q_ASSERT(check);

    d->serverForServers = new QXmppSslServer(this);
    check = connect(d->serverForServers, SIGNAL(newConnection(QSslSocket*)),
                    this, SLOT(slotServerConnection(QSslSocket*)));
    Q_ASSERT(check);
}

/// Destroys an XMPP server instance.
///

QXmppServer::~QXmppServer()
{
    close();
    delete d;
}

/// Registers a new extension with the server.
///
/// \param extension

void QXmppServer::addExtension(QXmppServerExtension *extension)
{
    if (!extension || d->extensions.contains(extension))
        return;
    d->info(QString("Added extension %1").arg(extension->extensionName()));
    extension->setParent(this);
    extension->setServer(this);
    d->extensions << extension;
}

/// Returns the list of loaded extensions.
///

QList<QXmppServerExtension*> QXmppServer::extensions()
{
    d->loadExtensions(this);
    return d->extensions;
}

/// Returns the list of available resources for the given local JID.
///
/// \param bareJid

QList<QXmppPresence> QXmppServer::availablePresences(const QString &bareJid)
{
    return d->presences.value(bareJid).values();
}

/// Returns the server's domain.
///

QString QXmppServer::domain() const
{
    return d->domain;
}

/// Sets the server's domain.
///
/// \param domain

void QXmppServer::setDomain(const QString &domain)
{
    d->domain = domain;
}

/// Returns the QXmppLogger associated with the server.
///

QXmppLogger *QXmppServer::logger()
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the server.
///
/// \param logger

void QXmppServer::setLogger(QXmppLogger *logger)
{
    if (d->logger)
        QObject::disconnect(this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
                   d->logger, SLOT(log(QXmppLogger::MessageType, QString)));
    d->logger = logger;
    d->logger = logger;
    if (d->logger)
        connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                d->logger, SLOT(log(QXmppLogger::MessageType,QString)));
}

/// Returns the password checker used to verify client credentials.
///

QXmppPasswordChecker *QXmppServer::passwordChecker()
{
    return d->passwordChecker;
}

/// Sets the password checker used to verify client credentials.
///
/// \param checker
///

void QXmppServer::setPasswordChecker(QXmppPasswordChecker *checker)
{
    d->passwordChecker = checker;
}

/// Sets the path for additional SSL CA certificates.
///
/// \param path

void QXmppServer::addCaCertificates(const QString &path)
{
    if (!path.isEmpty() && !QFileInfo(path).isReadable())
        d->warning(QString("SSL CA certificates are not readable %1").arg(path));
    d->serverForClients->addCaCertificates(path);
    d->serverForServers->addCaCertificates(path);
}

/// Sets the path for the local SSL certificate.
///
/// \param path

void QXmppServer::setLocalCertificate(const QString &path)
{
    if (!path.isEmpty() && !QFileInfo(path).isReadable())
        d->warning(QString("SSL certificate is not readable %1").arg(path));
    d->serverForClients->setLocalCertificate(path);
    d->serverForServers->setLocalCertificate(path);
}

/// Sets the path for the local SSL private key.
///
/// \param path

void QXmppServer::setPrivateKey(const QString &path)
{
    if (!path.isEmpty() && !QFileInfo(path).isReadable())
        d->warning(QString("SSL key is not readable %1").arg(path));
    d->serverForClients->setPrivateKey(path);
    d->serverForServers->setPrivateKey(path);
}

/// Listen for incoming XMPP client connections.
///
/// \param address
/// \param port

bool QXmppServer::listenForClients(const QHostAddress &address, quint16 port)
{
    if (!d->serverForClients->listen(address, port))
    {
        d->warning(QString("Could not start listening for C2S on port %1").arg(QString::number(port)));
        return false;
    }

    // start extensions
    d->loadExtensions(this);
    d->startExtensions();
    return true;
}

/// Closes the server.
///

void QXmppServer::close()
{
    // prevent new connections
    d->serverForClients->close();
    d->serverForServers->close();

    // stop extensions
    d->stopExtensions();

    // close XMPP streams
    foreach (QXmppIncomingClient *stream, d->incomingClients)
       stream->disconnectFromHost();
    foreach (QXmppIncomingServer *stream, d->incomingServers)
       stream->disconnectFromHost();
    foreach (QXmppOutgoingServer *stream, d->outgoingServers)
       stream->disconnectFromHost();
}

/// Listen for incoming XMPP server connections.
///
/// \param address
/// \param port

bool QXmppServer::listenForServers(const QHostAddress &address, quint16 port)
{
    if (!d->serverForServers->listen(address, port))
    {
        d->warning(QString("Could not start listening for S2S on port %1").arg(QString::number(port)));
        return false;
    }

    // start extensions
    d->loadExtensions(this);
    d->startExtensions();
    return true;
}

QXmppOutgoingServer* QXmppServer::connectToDomain(const QString &domain)
{
    // initialise outgoing server-to-server
    QXmppOutgoingServer *stream = new QXmppOutgoingServer(d->domain, this);
    stream->setLocalStreamKey(generateStanzaHash().toAscii());

    bool check = connect(stream, SIGNAL(connected()),
                         this, SLOT(slotStreamConnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(disconnected()),
                         this, SLOT(slotStreamDisconnected()));
    Q_UNUSED(check);

    // add stream
    d->outgoingServers.append(stream);
    emit streamAdded(stream);

    // connect to remote server
    stream->connectToHost(domain);
    return stream;
}

/// Returns the XMPP streams for the given recipient.
///
/// \param to
///

QList<QXmppStream*> QXmppServer::getStreams(const QString &to)
{
    QList<QXmppStream*> found;
    if (to.isEmpty())
        return found;
    const QString toDomain = jidToDomain(to);
    if (toDomain == d->domain)
    {
        // look for a client connection
        foreach (QXmppIncomingClient *conn, d->incomingClients)
        {
            if (conn->jid() == to || jidToBareJid(conn->jid()) == to)
                found << conn;
        }
    } else if (toDomain.endsWith("." + d->domain)) {
        // refuse to route packets to sub-domains
        return found;
    } else {
        // look for an outgoing S2S connection
        foreach (QXmppOutgoingServer *conn, d->outgoingServers)
        {
            if (conn->remoteDomain() == toDomain)
            {
                found << conn;
                break;
            }
        }

        // if we did not find an outgoing server,
        // we need to establish the S2S connection
        if (found.isEmpty() && d->serverForServers->isListening())
            found << connectToDomain(toDomain);
    }
    return found;
}

/// Handles an incoming XML element.
///
/// \param stream
/// \param element

void QXmppServer::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    // try extensions
    foreach (QXmppServerExtension *extension, d->extensions)
        if (extension->handleStanza(stream, element))
            return;

    // default handlers
    const QString to = element.attribute("to");
    if (to == d->domain)
    {
        if (element.tagName() == "presence")
        {
            // presence to the local domain, broadcast it to subscribers
            if (element.attribute("type").isEmpty() || element.attribute("type") == "unavailable")
            {
                const QString from = element.attribute("from");
                const QString bareFrom = jidToBareJid(from);
                bool isInitial = false;

                // record the presence for future use
                QXmppPresence presence;
                presence.parse(element);
                if (presence.type() == QXmppPresence::Available) {
                    isInitial = !d->presences.value(bareFrom).contains(from);
                    d->presences[bareFrom][from] = presence;
                } else {
                    d->presences[bareFrom].remove(from);
                }

                // broadcast it to subscribers
                foreach (const QString &subscriber, d->presenceSubscribers(from))
                {
                    // avoid loop
                    if (subscriber == to)
                        continue;
                    QDomElement changed(element);
                    changed.setAttribute("to", subscriber);
                    handleStanza(stream, changed);
                }

                // get presences from subscriptions
                if (isInitial) {
                    foreach (const QString &subscription, d->presenceSubscriptions(from))
                    {
                        if (jidToDomain(subscription) != d->domain) {
                            QXmppPresence probe;
                            probe.setType(QXmppPresence::Probe);
                            probe.setFrom(from);
                            probe.setTo(subscription);
                            sendPacket(probe);
                        } else {
                            QXmppPresence push;
                            foreach (push, d->presences.value(subscription).values()) {
                                push.setTo(from);
                                sendPacket(push);
                            }
                        }
                    }
                }
            }
        }
        else if (element.tagName() == "iq")
        {
            // we do not support the given IQ
            QXmppIq request;
            request.parse(element);

            if (request.type() != QXmppIq::Error && request.type() != QXmppIq::Result)
            {
                QXmppIq response(QXmppIq::Error);
                response.setId(request.id());
                response.setFrom(domain());
                response.setTo(request.from());
                QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                    QXmppStanza::Error::FeatureNotImplemented);
                response.setError(error);
                stream->sendPacket(response);
            }
        }

    } else {

        if (element.tagName() == "presence")
        {
            // directed presence, update subscribers
            QXmppPresence presence;
            presence.parse(element);

            const QString from = presence.from();
            if (presence.type() == QXmppPresence::Available)
                d->subscribers[from].insert(to);
            else if (presence.type() == QXmppPresence::Unavailable)
                d->subscribers[from].remove(to);
        }

        // route element or reply on behalf of missing peer
        if (!sendElement(element) && element.tagName() == "iq")
        {
            QXmppIq request;
            request.parse(element);

            QXmppIq response(QXmppIq::Error);
            response.setId(request.id());
            response.setFrom(request.to());
            response.setTo(request.from());
            QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                QXmppStanza::Error::ServiceUnavailable);
            response.setError(error);
            stream->sendPacket(response);
        }
    }
}

/// Route an XMPP stanza.
///
/// \param element

bool QXmppServer::sendElement(const QDomElement &element)
{
    bool sent = false;
    const QString to = element.attribute("to");
    foreach (QXmppStream *conn, getStreams(to))
    {
        if (conn->isConnected() && conn->sendElement(element))
            sent = true;
        else
        {
            // queue packet
            QByteArray data;
            QXmlStreamWriter xmlStream(&data);
            const QStringList omitNamespaces = QStringList() << ns_client << ns_server;
            helperToXmlAddDomElement(&xmlStream, element, omitNamespaces);
            d->queues[conn] << data;
            sent = true;
        }
    }
    return sent;
}

/// Route an XMPP packet.
///
/// \param packet

bool QXmppServer::sendPacket(const QXmppStanza &packet)
{
    bool sent = false;
    foreach (QXmppStream *conn, getStreams(packet.to()))
    {
        if (conn->isConnected() && conn->sendPacket(packet))
            sent = true;
        else
        {
            // queue packet
            QByteArray data;
            QXmlStreamWriter xmlStream(&data);
            packet.toXml(&xmlStream);
            d->queues[conn] << data;
            sent = true;
        }
    }
    return sent;
}

/// Add a new incoming client stream.
///
/// \param stream

void QXmppServer::addIncomingClient(QXmppIncomingClient *stream)
{
    stream->setPasswordChecker(d->passwordChecker);

    bool check = connect(stream, SIGNAL(connected()),
                         this, SLOT(slotStreamConnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(disconnected()),
                    this, SLOT(slotStreamDisconnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(elementReceived(QDomElement)),
                    this, SLOT(slotElementReceived(QDomElement)));
    Q_ASSERT(check);

    // add stream
    d->incomingClients.append(stream);
    emit streamAdded(stream);
}

/// Handle a new incoming TCP connection from a client.
///
/// \param socket

void QXmppServer::slotClientConnection(QSslSocket *socket)
{
    QXmppIncomingClient *stream = new QXmppIncomingClient(socket, d->domain, this);
    stream->setInactivityTimeout(120);
    socket->setParent(stream);
    addIncomingClient(stream);
}

void QXmppServer::slotDialbackRequestReceived(const QXmppDialback &dialback)
{
    QXmppIncomingServer *stream = qobject_cast<QXmppIncomingServer *>(sender());
    if (!stream)
        return;

    if (dialback.command() == QXmppDialback::Verify)
    {
        // handle a verify request
        foreach (QXmppOutgoingServer *out, d->outgoingServers)
        {
            if (out->remoteDomain() != dialback.from())
                continue;

            bool isValid = dialback.key() == out->localStreamKey();
            QXmppDialback verify;
            verify.setCommand(QXmppDialback::Verify);
            verify.setId(dialback.id());
            verify.setTo(dialback.from());
            verify.setFrom(d->domain);
            verify.setType(isValid ? "valid" : "invalid");
            stream->sendPacket(verify);
            return;
        }
    }
}

/// Handle an incoming XML element.

void QXmppServer::slotElementReceived(const QDomElement &element)
{
    QXmppStream *incoming = qobject_cast<QXmppStream *>(sender());
    if (!incoming)
        return;
    handleStanza(incoming, element);
}

/// Handle a new incoming TCP connection from a server.
///
/// \param socket

void QXmppServer::slotServerConnection(QSslSocket *socket)
{
    QXmppIncomingServer *stream = new QXmppIncomingServer(socket, d->domain, this);
    socket->setParent(stream);

    bool check = connect(stream, SIGNAL(connected()),
                         this, SLOT(slotStreamConnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(disconnected()),
                         this, SLOT(slotStreamDisconnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(dialbackRequestReceived(QXmppDialback)),
                    this, SLOT(slotDialbackRequestReceived(QXmppDialback)));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(elementReceived(QDomElement)),
                    this, SLOT(slotElementReceived(QDomElement)));
    Q_ASSERT(check);

    // add stream
    d->incomingServers.append(stream);
    emit streamAdded(stream);
}

/// Handle a successful stream connection.
///

void QXmppServer::slotStreamConnected()
{
    QXmppStream *stream = qobject_cast<QXmppStream*>(sender());
    if (!stream)
        return;

    // handle incoming clients
    QXmppIncomingClient *client = qobject_cast<QXmppIncomingClient *>(stream);
    if (client)
    {
        // check whether the connection conflicts with another one
        foreach (QXmppIncomingClient *conn, d->incomingClients)
        {
            if (conn != client && conn->jid() == client->jid())
            {
                conn->sendData("<stream:error><conflict xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text xmlns='urn:ietf:params:xml:ns:xmpp-streams'>Replaced by new connection</text></stream:error>");
                conn->disconnectFromHost();
            }
        }
    }

    // flush queue
    if (d->queues.contains(stream))
    {
        foreach (const QByteArray &data, d->queues[stream])
            stream->sendData(data);
        d->queues.remove(stream);
    }

    // emit signal
    emit streamConnected(stream);
}

/// Handle a stream disconnection.
///

void QXmppServer::slotStreamDisconnected()
{
    // handle clients
    QXmppIncomingClient *stream = qobject_cast<QXmppIncomingClient *>(sender());
    if (stream && d->incomingClients.contains(stream))
    {
        const QString jid = stream->jid();

        // check the user exited cleanly
        if (!jid.isEmpty()) {
            QDomDocument doc;
            QDomElement presence = doc.createElement("presence");
            presence.setAttribute("from", stream->jid());
            presence.setAttribute("type", "unavailable");

            if (d->presences.value(jidToBareJid(jid)).contains(jid)) {
                // the client had sent an initial available presence but did
                // not sent an unavailable presence, synthesize it
                presence.setAttribute("to", d->domain);
                handleStanza(stream, presence);
            } else {
                // synthesize unavailable presence to directed presence receivers
                const QSet<QString> recipients = d->subscribers.value(jid);
                foreach (const QString &recipient, recipients) {
                    presence.setAttribute("to", recipient);
                    handleStanza(stream, presence);
                }
            }
        }

        // remove stream
        d->incomingClients.removeAll(stream);
        d->queues.remove(stream);
        emit streamRemoved(stream);
        stream->deleteLater();
        return;
    }

    // handle incoming streams
    QXmppIncomingServer *incoming = qobject_cast<QXmppIncomingServer *>(sender());
    if (incoming && d->incomingServers.contains(incoming))
    {
        d->incomingServers.removeAll(incoming);
        d->queues.remove(incoming);
        emit streamRemoved(incoming);
        incoming->deleteLater();
        return;
    }

    // handle outgoing streams
    QXmppOutgoingServer *outgoing = qobject_cast<QXmppOutgoingServer *>(sender());
    if (outgoing && d->outgoingServers.contains(outgoing))
    {
        d->outgoingServers.removeAll(outgoing);
        d->queues.remove(outgoing);
        emit streamRemoved(outgoing);
        outgoing->deleteLater();
        return;
    }
}

class QXmppSslServerPrivate
{
public:
    QString caCertificates;
    QString localCertificate;
    QString privateKey;
};

/// Constructs a new SSL server instance.
///
/// \param parent

QXmppSslServer::QXmppSslServer(QObject *parent)
    : QTcpServer(parent),
    d(new QXmppSslServerPrivate)
{
}

/// Destroys an SSL server instance.
///

QXmppSslServer::~QXmppSslServer()
{
    delete d;
}

void QXmppSslServer::incomingConnection(int socketDescriptor)
{
    QSslSocket *socket = new QSslSocket;
    socket->setSocketDescriptor(socketDescriptor);
    if (!d->localCertificate.isEmpty() && !d->privateKey.isEmpty())
    {
        socket->setProtocol(QSsl::AnyProtocol);
        socket->addCaCertificates(d->caCertificates);
        socket->setLocalCertificate(d->localCertificate);
        socket->setPrivateKey(d->privateKey);
    }
    emit newConnection(socket);
}

/// Adds the given certificates to the CA certificate database to be used
/// for incoming connnections.
///
/// \param caCertificates

void QXmppSslServer::addCaCertificates(const QString &caCertificates)
{
    d->caCertificates = caCertificates;
}

/// Sets the local certificate to be used for incoming connections.
///
/// \param localCertificate

void QXmppSslServer::setLocalCertificate(const QString &localCertificate)
{
    d->localCertificate = localCertificate;
}

/// Sets the local private key to be used for incoming connections.
///
/// \param privateKey

void QXmppSslServer::setPrivateKey(const QString &privateKey)
{
    d->privateKey = privateKey;
}

