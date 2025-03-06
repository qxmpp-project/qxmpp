// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOutgoingServer.h"

#include "QXmppConstants_p.h"
#include "QXmppDialback.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Stream.h"
#include "StringLiterals.h"
#include "XmppSocket.h"

#include <chrono>

#include <QDnsLookup>
#include <QDomElement>
#include <QList>
#include <QSslError>
#include <QSslKey>
#include <QSslSocket>
#include <QTimer>

using namespace std::chrono_literals;
using namespace QXmpp::Private;

class QXmppOutgoingServerPrivate
{
public:
    explicit QXmppOutgoingServerPrivate(QObject *q);

    XmppSocket socket;
    QList<QByteArray> dataQueue;
    QDnsLookup dns;
    QString localDomain;
    QString localStreamKey;
    QString remoteDomain;
    QString verifyId;
    QString verifyKey;
    QTimer *dialbackTimer;
    bool ready;
};

QXmppOutgoingServerPrivate::QXmppOutgoingServerPrivate(QObject *q)
    : socket(q)
{
}

///
/// Constructs a new outgoing server-to-server stream.
///
/// \param domain the local domain
/// \param parent the parent object
///
QXmppOutgoingServer::QXmppOutgoingServer(const QString &domain, QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppOutgoingServerPrivate>(this))
{
    // socket initialisation
    auto *socket = new QSslSocket(this);
    d->socket.setSocket(socket);

    connect(&d->socket, &XmppSocket::started, this, &QXmppOutgoingServer::handleStart);
    connect(&d->socket, &XmppSocket::stanzaReceived, this, &QXmppOutgoingServer::handleStanza);
    connect(&d->socket, &XmppSocket::streamReceived, this, &QXmppOutgoingServer::handleStream);
    connect(&d->socket, &XmppSocket::streamClosed, this, &QXmppOutgoingServer::disconnectFromHost);
    connect(socket, &QAbstractSocket::disconnected, this, &QXmppOutgoingServer::onSocketDisconnected);
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppOutgoingServer::socketError);

    // DNS lookups
    connect(&d->dns, &QDnsLookup::finished, this, &QXmppOutgoingServer::onDnsLookupFinished);

    d->dialbackTimer = new QTimer(this);
    d->dialbackTimer->setInterval(5s);
    d->dialbackTimer->setSingleShot(true);
    connect(d->dialbackTimer, &QTimer::timeout, this, &QXmppOutgoingServer::sendDialback);

    d->localDomain = domain;
    d->ready = false;

    connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &QXmppOutgoingServer::slotSslErrors);
}

QXmppOutgoingServer::~QXmppOutgoingServer() = default;

///
/// Attempts to connect to an XMPP server for the specified \a domain.
///
/// \param domain
///
void QXmppOutgoingServer::connectToHost(const QString &domain)
{
    d->remoteDomain = domain;

    // lookup server for domain
    debug(u"Looking up server for domain %1"_s.arg(domain));
    d->dns.setName(u"_xmpp-server._tcp."_s + domain);
    d->dns.setType(QDnsLookup::SRV);
    d->dns.lookup();
}

void QXmppOutgoingServer::onDnsLookupFinished()
{
    QString host;
    quint16 port = 0;

    const auto dnsRecords = d->dns.serviceRecords();
    if (d->dns.error() == QDnsLookup::NoError && !dnsRecords.isEmpty()) {
        // take the first returned record
        host = dnsRecords.first().target();
        port = dnsRecords.first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(u"Lookup for domain %1 failed: %2"_s
                    .arg(d->dns.name(), d->dns.errorString()));
        host = d->remoteDomain;
        port = XMPP_SERVER_DEFAULT_PORT;
    }

    // set the name the SSL certificate should match
    d->socket.socket()->setPeerVerifyName(d->remoteDomain);

    // connect to server
    info(u"Connecting to %1:%2"_s.arg(host, QString::number(port)));
    d->socket.socket()->connectToHost(host, port);
}

void QXmppOutgoingServer::onSocketDisconnected()
{
    debug(u"Socket disconnected"_s);
    Q_EMIT disconnected();
}

void QXmppOutgoingServer::handleStart()
{
    QString data = u"<?xml version='1.0'?><stream:stream"
                   " xmlns='%1' xmlns:db='%2' xmlns:stream='%3' version='1.0'"
                   " from='%4' to='%5'>"_s
                       .arg(
                           ns_server,
                           ns_server_dialback,
                           ns_stream,
                           d->localDomain,
                           d->remoteDomain);
    sendData(data.toUtf8());
}

void QXmppOutgoingServer::handleStream(const StreamOpen &)
{
    // gmail.com servers are broken: they never send <stream:features>,
    // so we schedule sending the dialback in a couple of seconds
    d->dialbackTimer->start();
}

void QXmppOutgoingServer::handleStanza(const QDomElement &stanza)
{
    if (QXmppStreamFeatures::isStreamFeatures(stanza)) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        if (!d->socket.socket()->isEncrypted()) {
            // check we can satisfy TLS constraints
            if (!QSslSocket::supportsSsl() &&
                features.tlsMode() == QXmppStreamFeatures::Required) {
                warning(u"Disconnecting as TLS is required, but SSL support is not available"_s);
                disconnectFromHost();
                return;
            }

            // enable TLS if possible
            if (QSslSocket::supportsSsl() &&
                features.tlsMode() != QXmppStreamFeatures::Disabled) {
                sendData(serializeXml(StarttlsRequest()));
                return;
            }
        }

        // send dialback if needed
        d->dialbackTimer->stop();
        sendDialback();
    } else if (StarttlsProceed::fromDom(stanza)) {
        debug(u"Starting encryption"_s);
        d->socket.socket()->startClientEncryption();
        return;
    } else if (QXmppDialback::isDialback(stanza)) {
        QXmppDialback response;
        response.parse(stanza);

        // check the request is valid
        if (response.from().isEmpty() ||
            response.to() != d->localDomain ||
            response.type().isEmpty()) {
            warning(u"Invalid dialback response received"_s);
            return;
        }
        if (response.command() == QXmppDialback::Result) {
            if (response.type() == u"valid") {
                info(u"Outgoing server stream to %1 is ready"_s.arg(response.from()));
                d->ready = true;

                // send queued data
                for (const auto &data : std::as_const(d->dataQueue)) {
                    sendData(data);
                }
                d->dataQueue.clear();

                // emit signal
                Q_EMIT connected();
            }
        } else if (response.command() == QXmppDialback::Verify) {
            Q_EMIT dialbackResponseReceived(response);
        }
    }
}

/// Returns true if the socket is connected and authentication succeeded.
bool QXmppOutgoingServer::isConnected() const
{
    return d->socket.isConnected() && d->ready;
}

/// Disconnects from the remote host.
void QXmppOutgoingServer::disconnectFromHost()
{
    d->socket.disconnectFromHost();
}

/// Sends raw data to the peer.
bool QXmppOutgoingServer::sendData(const QByteArray &data)
{
    return d->socket.sendData(data);
}

/// Sends an XMPP packet to the peer.
bool QXmppOutgoingServer::sendPacket(const QXmppNonza &nonza)
{
    return d->socket.sendData(serializeXml(nonza));
}

/// Returns the stream's local dialback key.
QString QXmppOutgoingServer::localStreamKey() const
{
    return d->localStreamKey;
}

/// Sets the stream's local dialback key.
void QXmppOutgoingServer::setLocalStreamKey(const QString &key)
{
    d->localStreamKey = key;
}

/// Sets the stream's verification information.
void QXmppOutgoingServer::setVerify(const QString &id, const QString &key)
{
    d->verifyId = id;
    d->verifyKey = key;
}

/// Sends or queues data until connected.
void QXmppOutgoingServer::queueData(const QByteArray &data)
{
    if (isConnected()) {
        sendData(data);
    } else {
        d->dataQueue.append(data);
    }
}

/// Returns the remote server's domain.
QString QXmppOutgoingServer::remoteDomain() const
{
    return d->remoteDomain;
}

void QXmppOutgoingServer::sendDialback()
{
    if (!d->localStreamKey.isEmpty()) {
        // send dialback key
        debug(u"Sending dialback result to %1"_s.arg(d->remoteDomain));
        QXmppDialback dialback;
        dialback.setCommand(QXmppDialback::Result);
        dialback.setFrom(d->localDomain);
        dialback.setTo(d->remoteDomain);
        dialback.setKey(d->localStreamKey);
        sendPacket(dialback);
    } else if (!d->verifyId.isEmpty() && !d->verifyKey.isEmpty()) {
        // send dialback verify
        debug(u"Sending dialback verify to %1"_s.arg(d->remoteDomain));
        QXmppDialback verify;
        verify.setCommand(QXmppDialback::Verify);
        verify.setId(d->verifyId);
        verify.setFrom(d->localDomain);
        verify.setTo(d->remoteDomain);
        verify.setKey(d->verifyKey);
        sendPacket(verify);
    }
}

void QXmppOutgoingServer::slotSslErrors(const QList<QSslError> &errors)
{
    warning(u"SSL errors"_s);
    for (int i = 0; i < errors.count(); ++i) {
        warning(errors.at(i).errorString());
    }
    d->socket.socket()->ignoreSslErrors();
}

void QXmppOutgoingServer::socketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    Q_EMIT disconnected();
}
