/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QCoreApplication>
#include <QDomElement>
#include <QFileInfo>
#include <QPluginLoader>
#include <QSslCertificate>
#include <QSslKey>
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

static void helperToXmlAddDomElement(QXmlStreamWriter* stream, const QDomElement& element, const QStringList &omitNamespaces)
{
    stream->writeStartElement(element.tagName());

    /* attributes */
    QString xmlns = element.namespaceURI();
    if (!xmlns.isEmpty() && !omitNamespaces.contains(xmlns))
        stream->writeAttribute("xmlns", xmlns);
    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); i++)
    {
        QDomAttr attr = attrs.item(i).toAttr();
        stream->writeAttribute(attr.name(), attr.value());
    }

    /* children */
    QDomNode childNode = element.firstChild();
    while (!childNode.isNull())
    {
        if (childNode.isElement())
        {
            helperToXmlAddDomElement(stream, childNode.toElement(), QStringList() << xmlns);
        } else if (childNode.isText()) {
            stream->writeCharacters(childNode.toText().data());
        }
        childNode = childNode.nextSibling();
    }
    stream->writeEndElement();
}

class QXmppServerPrivate
{
public:
    QXmppServerPrivate(QXmppServer *qq);
    void loadExtensions(QXmppServer *server);
    bool routeData(const QString &to, const QByteArray &data);
    void startExtensions();
    void stopExtensions();

    void info(const QString &message);
    void warning(const QString &message);

    QString domain;
    QList<QXmppServerExtension*> extensions;
    QXmppLogger *logger;
    QXmppPasswordChecker *passwordChecker;

    // client-to-server
    QSet<QXmppIncomingClient*> incomingClients;
    QHash<QString, QXmppIncomingClient*> incomingClientsByJid;
    QHash<QString, QSet<QXmppIncomingClient*> > incomingClientsByBareJid;
    QSet<QXmppSslServer*> serversForClients;

    // server-to-server
    QSet<QXmppIncomingServer*> incomingServers;
    QSet<QXmppOutgoingServer*> outgoingServers;
    QSet<QXmppSslServer*> serversForServers;

    // ssl
    QList<QSslCertificate> caCertificates;
    QSslCertificate localCertificate;
    QSslKey privateKey;

private:
    bool loaded;
    bool started;
    QXmppServer *q;
};

QXmppServerPrivate::QXmppServerPrivate(QXmppServer *qq)
    : logger(0),
    passwordChecker(0),
    loaded(false),
    started(false),
    q(qq)
{
}

/// Routes XMPP data to the given recipient.
///
/// \param to
/// \param data
///

bool QXmppServerPrivate::routeData(const QString &to, const QByteArray &data)
{
    // refuse to route packets to empty destination, own domain or sub-domains
    const QString toDomain = QXmppUtils::jidToDomain(to);
    if (to.isEmpty() || to == domain || toDomain.endsWith("." + domain))
        return false;

    if (toDomain == domain) {

        // look for a client connection
        QList<QXmppIncomingClient*> found;
        if (QXmppUtils::jidToResource(to).isEmpty()) {
            foreach (QXmppIncomingClient *conn, incomingClientsByBareJid.value(to))
                found << conn;
        } else {
            QXmppIncomingClient *conn = incomingClientsByJid.value(to);
            if (conn)
                found << conn;
        }

        // send data
        foreach (QXmppStream *conn, found)
            QMetaObject::invokeMethod(conn, "sendData", Q_ARG(QByteArray, data));
        return !found.isEmpty();

    } else if (!serversForServers.isEmpty()) {

        bool check;
        Q_UNUSED(check);

        // look for an outgoing S2S connection
        foreach (QXmppOutgoingServer *conn, outgoingServers) {
            if (conn->remoteDomain() == toDomain) {
                // send or queue data
                QMetaObject::invokeMethod(conn, "queueData", Q_ARG(QByteArray, data));
                return true;
            }
        }

        // if we did not find an outgoing server,
        // we need to establish the S2S connection
        QXmppOutgoingServer *conn = new QXmppOutgoingServer(domain, 0);
        conn->setLocalStreamKey(QXmppUtils::generateStanzaHash().toLatin1());
        conn->moveToThread(q->thread());
        conn->setParent(q);

        check = QObject::connect(conn, SIGNAL(disconnected()),
                                 q, SLOT(_q_outgoingServerDisconnected()));
        Q_UNUSED(check);

        // add stream
        outgoingServers.insert(conn);
        q->setGauge("outgoing-server.count", outgoingServers.size());

        // queue data and connect to remote server
        QMetaObject::invokeMethod(conn, "queueData", Q_ARG(QByteArray, data));
        QMetaObject::invokeMethod(conn, "connectToHost", Q_ARG(QString, toDomain));
        return true;

    } else {

        // S2S is disabled, failed to route data
        return false;

    }
}

/// Handles an incoming XML element.
///
/// \param server
/// \param stream
/// \param element

static void handleStanza(QXmppServer *server, const QDomElement &element)
{
    // try extensions
    foreach (QXmppServerExtension *extension, server->extensions())
        if (extension->handleStanza(element))
            return;

    // default handlers
    const QString domain = server->domain();
    const QString to = element.attribute("to");
    if (to == domain) {
        if (element.tagName() == QLatin1String("iq")) {
            // we do not support the given IQ
            QXmppIq request;
            request.parse(element);

            if (request.type() != QXmppIq::Error && request.type() != QXmppIq::Result) {
                QXmppIq response(QXmppIq::Error);
                response.setId(request.id());
                response.setFrom(domain);
                response.setTo(request.from());
                QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                    QXmppStanza::Error::FeatureNotImplemented);
                response.setError(error);
                server->sendPacket(response);
            }
        }

    } else {

        // route element or reply on behalf of missing peer
        if (!server->sendElement(element) && element.tagName() == QLatin1String("iq")) {
            QXmppIq request;
            request.parse(element);

            QXmppIq response(QXmppIq::Error);
            response.setId(request.id());
            response.setFrom(request.to());
            response.setTo(request.from());
            QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                QXmppStanza::Error::ServiceUnavailable);
            response.setError(error);
            server->sendPacket(response);
        }
    }
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
    if (!loaded) {
        QObjectList plugins = QPluginLoader::staticInstances();
        foreach (QObject *object, plugins) {
            QXmppServerPlugin *plugin = qobject_cast<QXmppServerPlugin*>(object);
            if (!plugin)
                continue;

            foreach (const QString &key, plugin->keys())
                server->addExtension(plugin->create(key));
        }
        loaded = true;
    }
}

/// Start the server's extensions.

void QXmppServerPrivate::startExtensions()
{
    if (!started) {
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
    if (started) {
        for (int i = extensions.size() - 1; i >= 0; --i)
            extensions[i]->stop();
        started = false;
    }
}

/// Constructs a new XMPP server instance.
///
/// \param parent

QXmppServer::QXmppServer(QObject *parent)
    : QXmppLoggable(parent)
    , d(new QXmppServerPrivate(this))
{
    qRegisterMetaType<QDomElement>("QDomElement");
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

    // keep extensions sorted by priority
    for (int i = 0; i < d->extensions.size(); ++i) {
        QXmppServerExtension *other = d->extensions[i];
        if (other->extensionPriority() < extension->extensionPriority()) {
            d->extensions.insert(i, extension);
            return;
        }
    }
    d->extensions << extension;
}

/// Returns the list of loaded extensions.
///

QList<QXmppServerExtension*> QXmppServer::extensions()
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

/// Returns the statistics for the server.

QVariantMap QXmppServer::statistics() const
{
    QVariantMap stats;
    stats["version"] = qApp->applicationVersion();
    stats["incoming-clients"] = d->incomingClients.size();
    stats["incoming-servers"] = d->incomingServers.size();
    stats["outgoing-servers"] = d->outgoingServers.size();
    return stats;
}

/// Sets the path for additional SSL CA certificates.
///
/// \param path

void QXmppServer::addCaCertificates(const QString &path)
{
    // load certificates
    if (path.isEmpty()) {
        d->caCertificates = QList<QSslCertificate>();
    } else if (QFileInfo(path).isReadable()) {
        d->caCertificates = QSslCertificate::fromPath(path);
    } else {
        d->warning(QString("SSL CA certificates are not readable %1").arg(path));
        d->caCertificates = QList<QSslCertificate>();
    }

    // reconfigure servers
    foreach (QXmppSslServer *server, d->serversForClients + d->serversForServers)
        server->addCaCertificates(d->caCertificates);
}

/// Sets the path for the local SSL certificate.
///
/// \param path

void QXmppServer::setLocalCertificate(const QString &path)
{
    // load certificate
    QSslCertificate certificate;
    QFile file(path);
    if (path.isEmpty()) {
        d->localCertificate = QSslCertificate();
    } else if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        d->localCertificate = QSslCertificate(file.readAll());
    } else {
        d->warning(QString("SSL certificate is not readable %1").arg(path));
        d->localCertificate = QSslCertificate();
    }

    // reconfigure servers
    foreach (QXmppSslServer *server, d->serversForClients + d->serversForServers)
        server->setLocalCertificate(d->localCertificate);
}

/// Sets the path for the local SSL private key.
///
/// \param path

void QXmppServer::setPrivateKey(const QString &path)
{
    // load key
    QSslKey key;
    QFile file(path);
    if (path.isEmpty()) {
        d->privateKey = QSslKey();
    } else if (file.open(QIODevice::ReadOnly)) {
        d->privateKey = QSslKey(file.readAll(), QSsl::Rsa);
    } else {
        d->warning(QString("SSL key is not readable %1").arg(path));
        d->privateKey = QSslKey();
    }

    // reconfigure servers
    foreach (QXmppSslServer *server, d->serversForClients + d->serversForServers)
        server->setPrivateKey(d->privateKey);
}

/// Listen for incoming XMPP client connections.
///
/// \param address
/// \param port

bool QXmppServer::listenForClients(const QHostAddress &address, quint16 port)
{
    bool check;
    Q_UNUSED(check);

    if (d->domain.isEmpty()) {
        d->warning("No domain was specified!");
        return false;
    }

    // create new server
    QXmppSslServer *server = new QXmppSslServer(this);
    server->addCaCertificates(d->caCertificates);
    server->setLocalCertificate(d->localCertificate);
    server->setPrivateKey(d->privateKey);

    check = connect(server, SIGNAL(newConnection(QSslSocket*)),
                    this, SLOT(_q_clientConnection(QSslSocket*)));
    Q_ASSERT(check);

    if (!server->listen(address, port)) {
        d->warning(QString("Could not start listening for C2S on %1 %2").arg(address.toString(), QString::number(port)));
        delete server;
        return false;
    }
    d->serversForClients.insert(server);

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
    foreach (QXmppSslServer *server, d->serversForClients + d->serversForServers) {
        server->close();
        delete server;
    }
    d->serversForClients.clear();
    d->serversForServers.clear();

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
    bool check;
    Q_UNUSED(check);

    if (d->domain.isEmpty()) {
        d->warning("No domain was specified!");
        return false;
    }

    // create new server
    QXmppSslServer *server = new QXmppSslServer(this);
    server->addCaCertificates(d->caCertificates);
    server->setLocalCertificate(d->localCertificate);
    server->setPrivateKey(d->privateKey);

    check = connect(server, SIGNAL(newConnection(QSslSocket*)),
                    this, SLOT(_q_serverConnection(QSslSocket*)));
    Q_ASSERT(check);

    if (!server->listen(address, port)) {
        d->warning(QString("Could not start listening for S2S on %1 %2").arg(address.toString(), QString::number(port)));
        delete server;
        return false;
    }
    d->serversForServers.insert(server);

    // start extensions
    d->loadExtensions(this);
    d->startExtensions();
    return true;
}

/// Route an XMPP stanza.
///
/// \param element

bool QXmppServer::sendElement(const QDomElement &element)
{
    // serialize data
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    const QStringList omitNamespaces = QStringList() << ns_client << ns_server;
    helperToXmlAddDomElement(&xmlStream, element, omitNamespaces);

    // route data
    return d->routeData(element.attribute("to"), data);
}

/// Route an XMPP packet.
///
/// \param packet

bool QXmppServer::sendPacket(const QXmppStanza &packet)
{
    // serialize data
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    packet.toXml(&xmlStream);

    // route data
    return d->routeData(packet.to(), data);
}

/// Add a new incoming client \a stream.
///
/// This method can be used for instance to implement BOSH support
/// as a server extension.

void QXmppServer::addIncomingClient(QXmppIncomingClient *stream)
{
    bool check;
    Q_UNUSED(check);

    stream->setPasswordChecker(d->passwordChecker);

    check = connect(stream, SIGNAL(connected()),
                    this, SLOT(_q_clientConnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(disconnected()),
                    this, SLOT(_q_clientDisconnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(elementReceived(QDomElement)),
                    this, SLOT(handleElement(QDomElement)));
    Q_ASSERT(check);

    // add stream
    d->incomingClients.insert(stream);
    setGauge("incoming-client.count", d->incomingClients.size());
}

/// Handle a new incoming TCP connection from a client.
///
/// \param socket

void QXmppServer::_q_clientConnection(QSslSocket *socket)
{
    // check the socket didn't die since the signal was emitted
    if (socket->state() != QAbstractSocket::ConnectedState) {
        delete socket;
        return;
    }

    QXmppIncomingClient *stream = new QXmppIncomingClient(socket, d->domain, this);
    stream->setInactivityTimeout(120);
    socket->setParent(stream);
    addIncomingClient(stream);
}

/// Handle a successful stream connection for a client.
///

void QXmppServer::_q_clientConnected()
{
    QXmppIncomingClient *client = qobject_cast<QXmppIncomingClient*>(sender());
    if (!client)
        return;

    // FIXME: at this point the JID must contain a resource, assert it?
    const QString jid = client->jid();

    // check whether the connection conflicts with another one
    QXmppIncomingClient *old = d->incomingClientsByJid.value(jid);
    if (old && old != client) {
        old->sendData("<stream:error><conflict xmlns='urn:ietf:params:xml:ns:xmpp-streams'/><text xmlns='urn:ietf:params:xml:ns:xmpp-streams'>Replaced by new connection</text></stream:error>");
        old->disconnectFromHost();
    }
    d->incomingClientsByJid.insert(jid, client);
    d->incomingClientsByBareJid[QXmppUtils::jidToBareJid(jid)].insert(client);

    // emit signal
    emit clientConnected(jid);
}

/// Handle a stream disconnection for a client.

void QXmppServer::_q_clientDisconnected()
{
    QXmppIncomingClient *client  = qobject_cast<QXmppIncomingClient *>(sender());
    if (!client)
        return;

    if (d->incomingClients.remove(client)) {
        // remove stream from routing tables
        const QString jid = client->jid();
        if (!jid.isEmpty()) {
            if (d->incomingClientsByJid.value(jid) == client)
                d->incomingClientsByJid.remove(jid);
            const QString bareJid = QXmppUtils::jidToBareJid(jid);
            if (d->incomingClientsByBareJid.contains(bareJid)) {
                d->incomingClientsByBareJid[bareJid].remove(client);
                if (d->incomingClientsByBareJid[bareJid].isEmpty())
                    d->incomingClientsByBareJid.remove(bareJid);
            }
        }

        // destroy client
        client->deleteLater();

        // emit signal
        if (!jid.isEmpty())
            emit clientDisconnected(jid);

        // update counter
        setGauge("incoming-client.count", d->incomingClients.size());
    }
}

void QXmppServer::_q_dialbackRequestReceived(const QXmppDialback &dialback)
{
    QXmppIncomingServer *stream = qobject_cast<QXmppIncomingServer *>(sender());
    if (!stream)
        return;

    if (dialback.command() == QXmppDialback::Verify)
    {
        // handle a verify request
        foreach (QXmppOutgoingServer *out, d->outgoingServers) {
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

void QXmppServer::handleElement(const QDomElement &element)
{
    handleStanza(this, element);
}

/// Handle a stream disconnection for an outgoing server.

void QXmppServer::_q_outgoingServerDisconnected()
{
    QXmppOutgoingServer *outgoing = qobject_cast<QXmppOutgoingServer *>(sender());
    if (!outgoing)
        return;

    if (d->outgoingServers.remove(outgoing)) {
        outgoing->deleteLater();
        setGauge("outgoing-server.count", d->outgoingServers.size());
    }
}

/// Handle a new incoming TCP connection from a server.
///
/// \param socket

void QXmppServer::_q_serverConnection(QSslSocket *socket)
{
    bool check;
    Q_UNUSED(check);

    // check the socket didn't die since the signal was emitted
    if (socket->state() != QAbstractSocket::ConnectedState) {
        delete socket;
        return;
    }

    QXmppIncomingServer *stream = new QXmppIncomingServer(socket, d->domain, this);
    socket->setParent(stream);

    check = connect(stream, SIGNAL(disconnected()),
                    this, SLOT(_q_serverDisconnected()));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(dialbackRequestReceived(QXmppDialback)),
                    this, SLOT(_q_dialbackRequestReceived(QXmppDialback)));
    Q_ASSERT(check);

    check = connect(stream, SIGNAL(elementReceived(QDomElement)),
                    this, SLOT(handleElement(QDomElement)));
    Q_ASSERT(check);

    // add stream
    d->incomingServers.insert(stream);
    setGauge("incoming-server.count", d->incomingServers.size());
}

/// Handle a stream disconnection for an incoming server.

void QXmppServer::_q_serverDisconnected()
{
    QXmppIncomingServer *incoming = qobject_cast<QXmppIncomingServer *>(sender());
    if (!incoming)
        return;

    if (d->incomingServers.remove(incoming)) {
        incoming->deleteLater();
        setGauge("incoming-server.count", d->incomingServers.size());
    }
}

class QXmppSslServerPrivate
{
public:
    QList<QSslCertificate> caCertificates;
    QSslCertificate localCertificate;
    QSslKey privateKey;
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

#if QT_VERSION < 0x050000
void QXmppSslServer::incomingConnection(int socketDescriptor)
#else
void QXmppSslServer::incomingConnection(qintptr socketDescriptor)
#endif
{
    QSslSocket *socket = new QSslSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        delete socket;
        return;
    }

    if (!d->localCertificate.isNull() && !d->privateKey.isNull()) {
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
/// \param certificates

void QXmppSslServer::addCaCertificates(const QList<QSslCertificate> &certificates)
{
    d->caCertificates += certificates;
}

/// Sets the local certificate to be used for incoming connections.
///
/// \param certificate

void QXmppSslServer::setLocalCertificate(const QSslCertificate &certificate)
{
    d->localCertificate = certificate;
}

/// Sets the local private key to be used for incoming connections.
///
/// \param key

void QXmppSslServer::setPrivateKey(const QSslKey &key)
{
    d->privateKey = key;
}

