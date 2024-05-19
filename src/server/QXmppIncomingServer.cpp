// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIncomingServer.h"

#include "QXmppConstants_p.h"
#include "QXmppDialback.h"
#include "QXmppOutgoingServer.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Stream.h"
#include "StringLiterals.h"

#include <QDomElement>
#include <QHostAddress>
#include <QSslKey>
#include <QSslSocket>

using namespace QXmpp::Private;

class QXmppIncomingServerPrivate
{
public:
    QXmppIncomingServerPrivate(QXmppIncomingServer *qq);
    QString origin() const;

    QSet<QString> authenticated;
    QString domain;
    QString localStreamId;

private:
    QXmppIncomingServer *q;
};

QXmppIncomingServerPrivate::QXmppIncomingServerPrivate(QXmppIncomingServer *qq)
    : q(qq)
{
}

QString QXmppIncomingServerPrivate::origin() const
{
    QSslSocket *socket = q->socket();
    if (socket) {
        return socket->peerAddress().toString() + u' ' + QString::number(socket->peerPort());
    } else {
        return u"<unknown>"_s;
    }
}

///
/// Constructs a new incoming server stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///
QXmppIncomingServer::QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent),
      d(std::make_unique<QXmppIncomingServerPrivate>(this))
{
    d->domain = domain;

    if (socket) {
        connect(socket, &QAbstractSocket::disconnected,
                this, &QXmppIncomingServer::slotSocketDisconnected);

        setSocket(socket);
    }

    info(u"Incoming server connection from %1"_s.arg(d->origin()));
}

QXmppIncomingServer::~QXmppIncomingServer() = default;

///
/// Returns the stream's identifier.
///
QString QXmppIncomingServer::localStreamId() const
{
    return d->localStreamId;
}

/// \cond
void QXmppIncomingServer::handleStream(const QDomElement &streamElement)
{
    const QString from = streamElement.attribute(u"from"_s);
    if (!from.isEmpty()) {
        info(u"Incoming server stream from %1 on %2"_s.arg(from, d->origin()));
    }

    // start stream
    d->localStreamId = QXmppUtils::generateStanzaHash();
    QString data = u"<?xml version='1.0'?><stream:stream"
                   " xmlns='%1' xmlns:db='%2' xmlns:stream='%3'"
                   " id='%4' version=\"1.0\">"_s
                       .arg(
                           ns_server,
                           ns_server_dialback,
                           ns_stream,
                           d->localStreamId);
    sendData(data.toUtf8());

    // send stream features
    QXmppStreamFeatures features;
    if (!socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull()) {
        features.setTlsMode(QXmppStreamFeatures::Enabled);
    }
    sendPacket(features);
}

void QXmppIncomingServer::handleStanza(const QDomElement &stanza)
{
    if (StarttlsRequest::fromDom(stanza)) {
        sendData(serializeXml(StarttlsProceed()));
        socket()->flush();
        socket()->startServerEncryption();
        return;
    } else if (QXmppDialback::isDialback(stanza)) {
        QXmppDialback request;
        request.parse(stanza);
        // check the request is valid
        if (!request.type().isEmpty() ||
            request.from().isEmpty() ||
            request.to() != d->domain ||
            request.key().isEmpty()) {
            warning(u"Invalid dialback received on %1"_s.arg(d->origin()));
            return;
        }

        const QString domain = request.from();
        if (request.command() == QXmppDialback::Result) {
            debug(u"Received a dialback result from '%1' on %2"_s.arg(domain, d->origin()));

            // establish dialback connection
            auto *stream = new QXmppOutgoingServer(d->domain, this);
            connect(stream, &QXmppOutgoingServer::dialbackResponseReceived,
                    this, &QXmppIncomingServer::slotDialbackResponseReceived);
            stream->setVerify(d->localStreamId, request.key());
            stream->connectToHost(domain);
        } else if (request.command() == QXmppDialback::Verify) {
            debug(u"Received a dialback verify from '%1' on %2"_s.arg(domain, d->origin()));
            Q_EMIT dialbackRequestReceived(request);
        }

    } else if (d->authenticated.contains(QXmppUtils::jidToDomain(stanza.attribute(u"from"_s)))) {
        // relay stanza if the remote party is authenticated
        Q_EMIT elementReceived(stanza);
    } else {
        warning(u"Received an element from unverified domain '%1' on %2"_s.arg(QXmppUtils::jidToDomain(stanza.attribute(u"from"_s)), d->origin()));
        disconnectFromHost();
    }
}
/// \endcond

/// Returns true if the socket is connected and the remote server is
/// authenticated.
///

bool QXmppIncomingServer::isConnected() const
{
    return QXmppStream::isConnected() && !d->authenticated.isEmpty();
}

/// Handles a dialback response received from the authority server.
///
/// \param response
///

void QXmppIncomingServer::slotDialbackResponseReceived(const QXmppDialback &dialback)
{
    auto *stream = qobject_cast<QXmppOutgoingServer *>(sender());
    if (!stream ||
        dialback.command() != QXmppDialback::Verify ||
        dialback.id() != d->localStreamId ||
        dialback.from() != stream->remoteDomain()) {
        return;
    }

    // relay verify response
    QXmppDialback response;
    response.setCommand(QXmppDialback::Result);
    response.setTo(dialback.from());
    response.setFrom(d->domain);
    response.setType(dialback.type());
    sendPacket(response);

    // check for success
    if (response.type() == QLatin1String("valid")) {
        info(u"Verified incoming domain '%1' on %2"_s.arg(dialback.from(), d->origin()));
        const bool wasConnected = !d->authenticated.isEmpty();
        d->authenticated.insert(dialback.from());
        if (!wasConnected) {
            Q_EMIT connected();
        }
    } else {
        warning(u"Failed to verify incoming domain '%1' on %2"_s.arg(dialback.from(), d->origin()));
        disconnectFromHost();
    }

    // disconnect dialback
    stream->disconnectFromHost();
    stream->deleteLater();
}

void QXmppIncomingServer::slotSocketDisconnected()
{
    info(u"Socket disconnected from %1"_s.arg(d->origin()));
    Q_EMIT disconnected();
}
