// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppServer.h"

#include "QXmppConstants_p.h"
#include "QXmppDialback.h"
#include "QXmppIncomingClient.h"
#include "QXmppIncomingServer.h"
#include "QXmppIq.h"
#include "QXmppOutgoingServer.h"
#include "QXmppServerExtension.h"
#include "QXmppServerPlugin.h"
#include "QXmppUtils.h"

#include "StringLiterals.h"

#include <QCoreApplication>
#include <QDomElement>
#include <QFileInfo>
#include <QPluginLoader>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>

static void helperToXmlAddDomElement(QXmlStreamWriter *stream, const QDomElement &element, const QVector<QStringView> &omitNamespaces)
{
    stream->writeStartElement(element.tagName());

    /* attributes */
    QString xmlns = element.namespaceURI();
    if (!xmlns.isEmpty() && !omitNamespaces.contains(xmlns)) {
        stream->writeDefaultNamespace(xmlns);
    }
    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); i++) {
        QDomAttr attr = attrs.item(i).toAttr();
        stream->writeAttribute(attr.name(), attr.value());
    }

    /* children */
    QDomNode childNode = element.firstChild();
    while (!childNode.isNull()) {
        if (childNode.isElement()) {
            helperToXmlAddDomElement(stream, childNode.toElement(), { xmlns });
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
    QList<QXmppServerExtension *> extensions;
    QXmppLogger *logger;
    QXmppPasswordChecker *passwordChecker;

    // client-to-server
    QSet<QXmppIncomingClient *> incomingClients;
    QHash<QString, QXmppIncomingClient *> incomingClientsByJid;
    QHash<QString, QSet<QXmppIncomingClient *>> incomingClientsByBareJid;
    QSet<QXmppSslServer *> serversForClients;

    // server-to-server
    QSet<QXmppIncomingServer *> incomingServers;
    QSet<QXmppOutgoingServer *> outgoingServers;
    QSet<QXmppSslServer *> serversForServers;

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
    : logger(nullptr),
      passwordChecker(nullptr),
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
    if (to.isEmpty() || to == domain || toDomain.endsWith(QString(u"." + domain))) {
        return false;
    }

    if (toDomain == domain) {
        // look for a client connection
        QList<QXmppIncomingClient *> found;
        if (QXmppUtils::jidToResource(to).isEmpty()) {
            const auto &connections = incomingClientsByBareJid.value(to);
            for (auto *conn : connections) {
                found << conn;
            }
        } else {
            QXmppIncomingClient *conn = incomingClientsByJid.value(to);
            if (conn) {
                found << conn;
            }
        }

        // send data
        for (auto *conn : std::as_const(found)) {
            QMetaObject::invokeMethod(conn, "sendData", Q_ARG(QByteArray, data));
        }
        return !found.isEmpty();

    } else if (!serversForServers.isEmpty()) {

        // look for an outgoing S2S connection
        for (auto *conn : std::as_const(outgoingServers)) {
            if (conn->remoteDomain() == toDomain) {
                // send or queue data
                QMetaObject::invokeMethod(conn, "queueData", Q_ARG(QByteArray, data));
                return true;
            }
        }

        // if we did not find an outgoing server,
        // we need to establish the S2S connection
        auto *conn = new QXmppOutgoingServer(domain, nullptr);
        conn->setLocalStreamKey(QXmppUtils::generateStanzaHash());
        conn->moveToThread(q->thread());
        conn->setParent(q);

        QObject::connect(conn, &QXmppOutgoingServer::disconnected,
                         q, &QXmppServer::_q_outgoingServerDisconnected);

        // add stream
        outgoingServers.insert(conn);
        Q_EMIT q->setGauge(u"outgoing-server.count"_s, outgoingServers.size());

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
static void handleStanza(QXmppServer *server, const QDomElement &element)
{
    // try extensions
    const auto &extensions = server->extensions();
    for (auto *extension : extensions) {
        if (extension->handleStanza(element)) {
            return;
        }
    }

    // default handlers
    const QString domain = server->domain();
    const QString to = element.attribute(u"to"_s);
    if (to == domain) {
        if (element.tagName() == u"iq") {
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
        if (!server->sendElement(element) && element.tagName() == u"iq") {
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
    if (logger) {
        logger->log(QXmppLogger::InformationMessage, message);
    }
}

void QXmppServerPrivate::warning(const QString &message)
{
    if (logger) {
        logger->log(QXmppLogger::WarningMessage, message);
    }
}

/// Load the server's extensions.
void QXmppServerPrivate::loadExtensions(QXmppServer *server)
{
    if (!loaded) {
        const auto pluginLoaders = QPluginLoader::staticInstances();
        for (auto *object : pluginLoaders) {
            auto *plugin = qobject_cast<QXmppServerPlugin *>(object);
            if (!plugin) {
                continue;
            }

            const auto &keys = plugin->keys();
            for (const auto &key : keys) {
                server->addExtension(plugin->create(key));
            }
        }
        loaded = true;
    }
}

/// Start the server's extensions.
void QXmppServerPrivate::startExtensions()
{
    if (!started) {
        for (auto *extension : std::as_const(extensions)) {
            if (!extension->start()) {
                warning(u"Could not start extension %1"_s.arg(extension->extensionName()));
            }
        }
        started = true;
    }
}

/// Stop the server's extensions (in reverse order).
void QXmppServerPrivate::stopExtensions()
{
    if (started) {
        for (int i = extensions.size() - 1; i >= 0; --i) {
            extensions[i]->stop();
        }
        started = false;
    }
}

/// Constructs a new XMPP server instance.
QXmppServer::QXmppServer(QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppServerPrivate>(this))
{
    qRegisterMetaType<QDomElement>("QDomElement");
}

QXmppServer::~QXmppServer()
{
    close();
}

/// Registers a new extension with the server.
void QXmppServer::addExtension(QXmppServerExtension *extension)
{
    if (!extension || d->extensions.contains(extension)) {
        return;
    }
    d->info(u"Added extension %1"_s.arg(extension->extensionName()));
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
QList<QXmppServerExtension *> QXmppServer::extensions()
{
    d->loadExtensions(this);
    return d->extensions;
}

/// Returns the server's domain.
QString QXmppServer::domain() const
{
    return d->domain;
}

/// Sets the server's domain.
void QXmppServer::setDomain(const QString &domain)
{
    d->domain = domain;
}

QXmppLogger *QXmppServer::logger()
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the server.
void QXmppServer::setLogger(QXmppLogger *logger)
{
    if (logger != d->logger) {
        if (d->logger) {
            disconnect(this, &QXmppLoggable::logMessage,
                       d->logger, &QXmppLogger::log);
            disconnect(this, &QXmppLoggable::setGauge,
                       d->logger, &QXmppLogger::setGauge);
            disconnect(this, &QXmppLoggable::updateCounter,
                       d->logger, &QXmppLogger::updateCounter);
        }

        d->logger = logger;
        if (d->logger) {
            connect(this, &QXmppLoggable::logMessage,
                    d->logger, &QXmppLogger::log);
            connect(this, &QXmppLoggable::setGauge,
                    d->logger, &QXmppLogger::setGauge);
            connect(this, &QXmppLoggable::updateCounter,
                    d->logger, &QXmppLogger::updateCounter);
        }

        Q_EMIT loggerChanged(d->logger);
    }
}

/// Returns the password checker used to verify client credentials.
QXmppPasswordChecker *QXmppServer::passwordChecker()
{
    return d->passwordChecker;
}

/// Sets the password checker used to verify client credentials.
void QXmppServer::setPasswordChecker(QXmppPasswordChecker *checker)
{
    d->passwordChecker = checker;
}

/// Returns the statistics for the server.
QVariantMap QXmppServer::statistics() const
{
    QVariantMap stats;
    stats[u"version"_s] = qApp->applicationVersion();
    stats[u"incoming-clients"_s] = d->incomingClients.size();
    stats[u"incoming-servers"_s] = d->incomingServers.size();
    stats[u"outgoing-servers"_s] = d->outgoingServers.size();
    return stats;
}

/// Sets the path for additional SSL CA certificates.
void QXmppServer::addCaCertificates(const QString &path)
{
    // load certificates
    if (path.isEmpty()) {
        d->caCertificates = QList<QSslCertificate>();
    } else if (QFileInfo(path).isReadable()) {
        d->caCertificates = QSslCertificate::fromPath(path);
    } else {
        d->warning(u"SSL CA certificates are not readable %1"_s.arg(path));
        d->caCertificates = QList<QSslCertificate>();
    }

    // reconfigure servers
    for (auto *server : std::as_const(d->serversForClients)) {
        server->addCaCertificates(d->caCertificates);
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->addCaCertificates(d->caCertificates);
    }
}

/// Sets the path for the local SSL certificate.
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
        d->warning(u"SSL certificate is not readable %1"_s.arg(path));
        d->localCertificate = QSslCertificate();
    }

    // reconfigure servers
    for (auto *server : std::as_const(d->serversForClients)) {
        server->setLocalCertificate(d->localCertificate);
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->setLocalCertificate(d->localCertificate);
    }
}

///
/// Sets the local SSL certificate
///
/// \param certificate
///
/// \since QXmpp 0.9
///
void QXmppServer::setLocalCertificate(const QSslCertificate &certificate)
{
    d->localCertificate = certificate;

    // reconfigure servers
    for (auto *server : std::as_const(d->serversForClients)) {
        server->setLocalCertificate(d->localCertificate);
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->setLocalCertificate(d->localCertificate);
    }
}

/// Sets the path for the local SSL private key.
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
        d->warning(u"SSL key is not readable %1"_s.arg(path));
        d->privateKey = QSslKey();
    }

    // reconfigure servers
    for (auto *server : std::as_const(d->serversForClients)) {
        server->setPrivateKey(d->privateKey);
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->setPrivateKey(d->privateKey);
    }
}

///
/// Sets the local SSL private key.
///
/// \param key
///
/// \since QXmpp 0.9
///
void QXmppServer::setPrivateKey(const QSslKey &key)
{
    d->privateKey = key;

    // reconfigure servers
    for (auto *server : std::as_const(d->serversForClients)) {
        server->setPrivateKey(d->privateKey);
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->setPrivateKey(d->privateKey);
    }
}

/// Listen for incoming XMPP client connections.
bool QXmppServer::listenForClients(const QHostAddress &address, quint16 port)
{
    bool check;
    Q_UNUSED(check)

    if (d->domain.isEmpty()) {
        d->warning(u"No domain was specified!"_s);
        return false;
    }

    // create new server
    auto *server = new QXmppSslServer(this);
    server->addCaCertificates(d->caCertificates);
    server->setLocalCertificate(d->localCertificate);
    server->setPrivateKey(d->privateKey);

    check = connect(server, SIGNAL(newConnection(QSslSocket *)),
                    this, SLOT(_q_clientConnection(QSslSocket *)));
    Q_ASSERT(check);

    if (!server->listen(address, port)) {
        d->warning(u"Could not start listening for C2S on %1 %2"_s.arg(address.toString(), QString::number(port)));
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
void QXmppServer::close()
{
    // prevent new connections
    for (auto *server : std::as_const(d->serversForClients)) {
        server->close();
    }
    for (auto *server : std::as_const(d->serversForServers)) {
        server->close();
    }
    qDeleteAll(d->serversForClients);
    qDeleteAll(d->serversForServers);
    d->serversForClients.clear();
    d->serversForServers.clear();

    // stop extensions
    d->stopExtensions();

    // close XMPP streams
    QSetIterator<QXmppIncomingClient *> itr(d->incomingClients);
    while (itr.hasNext()) {
        itr.next()->disconnectFromHost();
    }
    for (auto *stream : std::as_const(d->incomingServers)) {
        stream->disconnectFromHost();
    }
    for (auto *stream : std::as_const(d->outgoingServers)) {
        stream->disconnectFromHost();
    }
}

/// Listen for incoming XMPP server connections.
bool QXmppServer::listenForServers(const QHostAddress &address, quint16 port)
{
    bool check;
    Q_UNUSED(check)

    if (d->domain.isEmpty()) {
        d->warning(u"No domain was specified!"_s);
        return false;
    }

    // create new server
    auto *server = new QXmppSslServer(this);
    server->addCaCertificates(d->caCertificates);
    server->setLocalCertificate(d->localCertificate);
    server->setPrivateKey(d->privateKey);

    check = connect(server, SIGNAL(newConnection(QSslSocket *)),
                    this, SLOT(_q_serverConnection(QSslSocket *)));
    Q_ASSERT(check);

    if (!server->listen(address, port)) {
        d->warning(u"Could not start listening for S2S on %1 %2"_s.arg(address.toString(), QString::number(port)));
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
bool QXmppServer::sendElement(const QDomElement &element)
{
    // serialize data
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    helperToXmlAddDomElement(&xmlStream, element, { ns_client, ns_server });

    // route data
    return d->routeData(element.attribute(u"to"_s), data);
}

/// Route an XMPP packet.
bool QXmppServer::sendPacket(const QXmppStanza &packet)
{
    // serialize data
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    packet.toXml(&xmlStream);

    // route data
    return d->routeData(packet.to(), data);
}

///
/// Add a new incoming client \a stream.
///
/// This method can be used for instance to implement BOSH support
/// as a server extension.
///
void QXmppServer::addIncomingClient(QXmppIncomingClient *stream)
{

    stream->setPasswordChecker(d->passwordChecker);

    connect(stream, &QXmppIncomingClient::connected, this, &QXmppServer::_q_clientConnected);
    connect(stream, &QXmppIncomingClient::disconnected, this, &QXmppServer::_q_clientDisconnected);
    connect(stream, &QXmppIncomingClient::elementReceived, this, &QXmppServer::handleElement);

    // add stream
    d->incomingClients.insert(stream);
    Q_EMIT setGauge(u"incoming-client.count"_s, d->incomingClients.size());
}

/// Handle a new incoming TCP connection from a client.
void QXmppServer::_q_clientConnection(QSslSocket *socket)
{
    // check the socket didn't die since the signal was emitted
    if (socket->state() != QAbstractSocket::ConnectedState) {
        delete socket;
        return;
    }

    auto *stream = new QXmppIncomingClient(socket, d->domain, this);
    stream->setInactivityTimeout(120);
    socket->setParent(stream);
    addIncomingClient(stream);
}

/// Handle a successful stream connection for a client.
void QXmppServer::_q_clientConnected()
{
    auto *client = qobject_cast<QXmppIncomingClient *>(sender());
    if (!client) {
        return;
    }

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
    Q_EMIT clientConnected(jid);
}

/// Handle a stream disconnection for a client.
void QXmppServer::_q_clientDisconnected()
{
    auto *client = qobject_cast<QXmppIncomingClient *>(sender());
    if (!client) {
        return;
    }

    if (d->incomingClients.remove(client)) {
        // remove stream from routing tables
        const QString jid = client->jid();
        if (!jid.isEmpty()) {
            if (d->incomingClientsByJid.value(jid) == client) {
                d->incomingClientsByJid.remove(jid);
            }
            const QString bareJid = QXmppUtils::jidToBareJid(jid);
            if (d->incomingClientsByBareJid.contains(bareJid)) {
                d->incomingClientsByBareJid[bareJid].remove(client);
                if (d->incomingClientsByBareJid[bareJid].isEmpty()) {
                    d->incomingClientsByBareJid.remove(bareJid);
                }
            }
        }

        // destroy client
        client->deleteLater();

        // emit signal
        if (!jid.isEmpty()) {
            Q_EMIT clientDisconnected(jid);
        }

        // update counter
        Q_EMIT setGauge(u"incoming-client.count"_s, d->incomingClients.size());
    }
}

void QXmppServer::_q_dialbackRequestReceived(const QXmppDialback &dialback)
{
    auto *stream = qobject_cast<QXmppIncomingServer *>(sender());
    if (!stream) {
        return;
    }

    if (dialback.command() == QXmppDialback::Verify) {
        // handle a verify request
        for (auto *out : std::as_const(d->outgoingServers)) {
            if (out->remoteDomain() != dialback.from()) {
                continue;
            }

            bool isValid = dialback.key() == out->localStreamKey();
            QXmppDialback verify;
            verify.setCommand(QXmppDialback::Verify);
            verify.setId(dialback.id());
            verify.setTo(dialback.from());
            verify.setFrom(d->domain);
            verify.setType(isValid ? u"valid"_s : u"invalid"_s);
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
    auto *outgoing = qobject_cast<QXmppOutgoingServer *>(sender());
    if (!outgoing) {
        return;
    }

    if (d->outgoingServers.remove(outgoing)) {
        outgoing->deleteLater();
        Q_EMIT setGauge(u"outgoing-server.count"_s, d->outgoingServers.size());
    }
}

/// Handle a new incoming TCP connection from a server.
void QXmppServer::_q_serverConnection(QSslSocket *socket)
{

    // check the socket didn't die since the signal was emitted
    if (socket->state() != QAbstractSocket::ConnectedState) {
        delete socket;
        return;
    }

    auto *stream = new QXmppIncomingServer(socket, d->domain, this);
    socket->setParent(stream);

    connect(stream, &QXmppIncomingServer::disconnected,
            this, &QXmppServer::_q_serverDisconnected);

    connect(stream, &QXmppIncomingServer::dialbackRequestReceived,
            this, &QXmppServer::_q_dialbackRequestReceived);

    connect(stream, &QXmppIncomingServer::elementReceived,
            this, &QXmppServer::handleElement);

    // add stream
    d->incomingServers.insert(stream);
    Q_EMIT setGauge(u"incoming-server.count"_s, d->incomingServers.size());
}

/// Handle a stream disconnection for an incoming server.
void QXmppServer::_q_serverDisconnected()
{
    auto *incoming = qobject_cast<QXmppIncomingServer *>(sender());
    if (!incoming) {
        return;
    }

    if (d->incomingServers.remove(incoming)) {
        incoming->deleteLater();
        Q_EMIT setGauge(u"incoming-server.count"_s, d->incomingServers.size());
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
QXmppSslServer::QXmppSslServer(QObject *parent)
    : QTcpServer(parent),
      d(std::make_unique<QXmppSslServerPrivate>())
{
}

QXmppSslServer::~QXmppSslServer() = default;

void QXmppSslServer::incomingConnection(qintptr socketDescriptor)
{
    auto *socket = new QSslSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        delete socket;
        return;
    }

    if (!d->localCertificate.isNull() && !d->privateKey.isNull()) {
        auto sslConfig = socket->sslConfiguration();
        sslConfig.setCaCertificates(sslConfig.caCertificates() + d->caCertificates);
        socket->setSslConfiguration(sslConfig);

        socket->setProtocol(QSsl::AnyProtocol);
        socket->setLocalCertificate(d->localCertificate);
        socket->setPrivateKey(d->privateKey);
    }
    Q_EMIT newConnection(socket);
}

///
/// Adds the given certificates to the CA certificate database to be used
/// for incoming connections.
///
void QXmppSslServer::addCaCertificates(const QList<QSslCertificate> &certificates)
{
    d->caCertificates += certificates;
}

/// Sets the local certificate to be used for incoming connections.
void QXmppSslServer::setLocalCertificate(const QSslCertificate &certificate)
{
    d->localCertificate = certificate;
}

/// Sets the local private key to be used for incoming connections.
void QXmppSslServer::setPrivateKey(const QSslKey &key)
{
    d->privateKey = key;
}
