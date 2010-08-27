/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#include "QXmppDialback.h"
#include "QXmppIq.h"
#include "QXmppIncomingClient.h"
#include "QXmppIncomingServer.h"
#include "QXmppOutgoingServer.h"
#include "QXmppPingIq.h"
#include "QXmppServer.h"
#include "QXmppServerExtension.h"
#include "QXmppServerPlugin.h"
#include "QXmppUtils.h"

class QXmppServerPrivate
{
public:
    QXmppServerPrivate();
    void loadExtensions(QXmppServer *server);
    void startExtensions(QXmppServer *server);
    void stopExtensions();

    void info(const QString &message);
    void warning(const QString &message);

    QString domain;
    QList<QXmppServerExtension*> extensions;
    QMap<QString, QStringList> subscribers;
    QXmppLogger *logger;
    QXmppPasswordChecker *passwordChecker;

    // client-to-server
    QXmppSslServer *serverForClients;
    QList<QXmppIncomingClient*> incomingClients;

    // server-to-server
    QList<QXmppIncomingServer*> incomingServers;
    QList<QXmppOutgoingServer*> outgoingServers;
    QXmppSslServer *serverForServers;

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
            {
                QXmppServerExtension *extension;
                server->addExtension(plugin->create(key));
            }
        }
        loaded = true;
    }
}

/// Start the server's extensions.
///
/// \param server

void QXmppServerPrivate::startExtensions(QXmppServer *server)
{
    if (!started)
    {
        foreach (QXmppServerExtension *extension, extensions)
            if (!extension->start(server))
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
    : QObject(parent),
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
    d->extensions << extension;
}

/// Returns the list of loaded extensions.
///

QList<QXmppServerExtension*> QXmppServer::loadedExtensions()
{
    d->loadExtensions(this);
    return d->extensions;
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

QXmppLogger *QXmppServer::logger()
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the server.

void QXmppServer::setLogger(QXmppLogger *logger)
{
    d->logger = logger;
}

/// Returns the password checker used to verify client credentials.
///
/// \param checker
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
    d->startExtensions(this);
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
    d->startExtensions(this);
    return true;
}

QXmppOutgoingServer* QXmppServer::connectToDomain(const QString &domain)
{
    // initialise outgoing server-to-server
    QXmppOutgoingServer *stream = new QXmppOutgoingServer(d->domain, this);
    stream->setObjectName("S2S-out-" + domain);
    stream->setLocalStreamKey(generateStanzaHash().toAscii());
    stream->setLogger(d->logger);
    stream->configuration().setDomain(domain);
    stream->configuration().setHost(domain);
    stream->configuration().setPort(5269);

    bool check = connect(stream, SIGNAL(disconnected()),
                         this, SLOT(slotServerDisconnected()));
    Q_ASSERT(check);
    Q_UNUSED(check);

    // add stream
    d->outgoingServers.append(stream);
    emit streamAdded(stream);

    // connect to remote server
    stream->connectToHost();
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
    } else {
        // look for an outgoing S2S connection
        foreach (QXmppOutgoingServer *conn, d->outgoingServers)
        {
            if (conn->configuration().domain() == toDomain && conn->isConnected())
            {
                found << conn;
                break;
            }
        }

        // if we did not find an outgoing server,
        // we need to establish the S2S connection
        if (found.isEmpty())
        {
            connectToDomain(toDomain);

            // FIXME : the current packet will not be delivered
        }
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
                foreach (QString subscriber, subscribers(from))
                {
                    QDomElement changed(element);
                    changed.setAttribute("to", subscriber);
                    sendElement(changed);
                }
            }
        }
        else if (element.tagName() == "iq")
        {
            // XEP-0199: XMPP Ping
            if (QXmppPingIq::isPingIq(element))
            {
                QXmppPingIq request;
                request.parse(element);

                QXmppIq response(QXmppIq::Result);
                response.setId(element.attribute("id"));
                response.setFrom(d->domain);
                response.setTo(request.from());
                stream->sendPacket(response);
            }
            // Other IQs
            else
            {
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
        }

    } else {

        if (element.tagName() == "presence")
        {
            // directed presence, update subscribers
            QXmppPresence presence;
            presence.parse(element);

            const QString from = presence.from();
            QStringList subscribers = d->subscribers.value(from);
            if (presence.type() == QXmppPresence::Available)
            {
                subscribers.append(to);
                d->subscribers[from] = subscribers;
            } else if (presence.type() == QXmppPresence::Unavailable) {
                subscribers.removeAll(to);
                d->subscribers[from] = subscribers;
            }
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

QStringList QXmppServer::subscribers(const QString &jid)
{
    QStringList recipients = d->subscribers.value(jid);

    // try extensions
    foreach (QXmppServerExtension *extension, d->extensions)
        recipients += extension->presenceSubscribers(jid);

    return recipients;
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
        if (conn->sendElement(element))
            sent = true;
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
        if (conn->sendPacket(packet))
            sent = true;
    }
    return sent;
}

/// Handle a new incoming TCP connection from a client.
///
/// \param socket

void QXmppServer::slotClientConnection(QSslSocket *socket)
{
    QXmppIncomingClient *stream = new QXmppIncomingClient(socket, d->domain, this);
    socket->setParent(stream);
    stream->setLogger(d->logger);
    stream->setPasswordChecker(d->passwordChecker);

    bool check = connect(stream, SIGNAL(connected()),
                         this, SLOT(slotClientConnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(disconnected()),
                    this, SLOT(slotClientDisconnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(elementReceived(QDomElement)),
                    this, SLOT(slotElementReceived(QDomElement)));
    Q_ASSERT(check);

    // add stream
    d->incomingClients.append(stream);
    emit streamAdded(stream);
}

/// Handle a successful client authentication.
///

void QXmppServer::slotClientConnected()
{
    QXmppIncomingClient *stream = qobject_cast<QXmppIncomingClient *>(sender());
    if (!stream || !d->incomingClients.contains(stream))
        return;

    // check whether the connection conflicts with another one
    foreach (QXmppIncomingClient *conn, d->incomingClients)
    {
        if (conn != stream && conn->jid() == stream->jid())
        {
            conn->sendData("<stream:error><conflict xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text xmlns='urn:ietf:params:xml:ns:xmpp-streams'>Replaced by new connection</text></stream:error>");
            conn->disconnectFromHost();
        }
    }
}

/// Handle a disconnection from a client.
///

void QXmppServer::slotClientDisconnected()
{
    QXmppIncomingClient *stream = qobject_cast<QXmppIncomingClient *>(sender());
    if (!stream || !d->incomingClients.contains(stream))
        return;

    // notify subscribed peers of disconnection
    if (!stream->jid().isEmpty())
    {
        foreach (QString subscriber, d->subscribers.value(stream->jid()))
        {
            QXmppPresence presence;
            presence.setFrom(stream->jid());
            presence.setTo(subscriber);
            presence.setType(QXmppPresence::Unavailable);
            sendPacket(presence);
        }
    }

    // remove stream
    d->incomingClients.removeAll(stream);
    emit streamRemoved(stream);
    stream->deleteLater();
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
            if (out->configuration().domain() != dialback.from())
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
    stream->setLogger(d->logger);

    bool check = connect(stream, SIGNAL(disconnected()),
                         this, SLOT(slotServerDisconnected()));
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

/// Handle a disconnection from a server.
///

void QXmppServer::slotServerDisconnected()
{
    // handle incoming streams
    QXmppIncomingServer *incoming = qobject_cast<QXmppIncomingServer *>(sender());
    if (incoming && d->incomingServers.contains(incoming))
    {
        d->incomingServers.removeAll(incoming);
        emit streamRemoved(incoming);
        incoming->deleteLater();
        return;
    }

    // handle outgoing streams
    QXmppOutgoingServer *outgoing = qobject_cast<QXmppOutgoingServer *>(sender());
    if (outgoing && d->outgoingServers.contains(outgoing))
    {
        d->outgoingServers.removeAll(outgoing);
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

void QXmppSslServer::addCaCertificates(const QString &caCertificates)
{
    d->caCertificates = caCertificates;
}

void QXmppSslServer::setLocalCertificate(const QString &localCertificate)
{
    d->localCertificate = localCertificate;
}

void QXmppSslServer::setPrivateKey(const QString &privateKey)
{
    d->privateKey = privateKey;
}

