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

#include <QDomElement>
#include <QHostAddress>
#include <QSslKey>
#include <QSslSocket>

#include "QXmppConstants.h"
#include "QXmppDialback.h"
#include "QXmppIncomingServer.h"
#include "QXmppOutgoingServer.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

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
    if (socket)
        return socket->peerAddress().toString() + " " + QString::number(socket->peerPort());
    else
        return "<unknown>";
}

/// Constructs a new incoming server stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///

QXmppIncomingServer::QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent)
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppIncomingServerPrivate(this);
    d->domain = domain;

    if (socket) {
        check = connect(socket, SIGNAL(disconnected()),
                        this, SLOT(slotSocketDisconnected()));
        Q_ASSERT(check);

        setSocket(socket);
    }

    info(QString("Incoming server connection from %1").arg(d->origin()));
}

/// Destroys the current stream.

QXmppIncomingServer::~QXmppIncomingServer()
{
    delete d;
}

/// Returns the stream's identifier.
///

QString QXmppIncomingServer::localStreamId() const
{
    return d->localStreamId;
}

/// \cond
void QXmppIncomingServer::handleStream(const QDomElement &streamElement)
{
    const QString from = streamElement.attribute("from");
    if (!from.isEmpty())
        info(QString("Incoming server stream from %1 on %2").arg(from, d->origin()));

    // start stream
    d->localStreamId = QXmppUtils::generateStanzaHash().toLatin1();
    QString data = QString("<?xml version='1.0'?><stream:stream"
        " xmlns='%1' xmlns:db='%2' xmlns:stream='%3'"
        " id='%4' version=\"1.0\">").arg(
            ns_server,
            ns_server_dialback,
            ns_stream,
            d->localStreamId);
    sendData(data.toUtf8());

    // send stream features
    QXmppStreamFeatures features;
    if (!socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull())
        features.setTlsMode(QXmppStreamFeatures::Enabled);
    sendPacket(features);
}

void QXmppIncomingServer::handleStanza(const QDomElement &stanza)
{
    const QString ns = stanza.namespaceURI();

    if (ns == ns_tls && stanza.tagName() == QLatin1String("starttls"))
    {
        sendData("<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
        socket()->flush();
        socket()->startServerEncryption();
        return;
    }
    else if (QXmppDialback::isDialback(stanza))
    {
        QXmppDialback request;
        request.parse(stanza);
        // check the request is valid
        if (!request.type().isEmpty() ||
            request.from().isEmpty() ||
            request.to() != d->domain ||
            request.key().isEmpty())
        {
            warning(QString("Invalid dialback received on %1").arg(d->origin()));
            return;
        }

        const QString domain = request.from();
        if (request.command() == QXmppDialback::Result)
        {
            debug(QString("Received a dialback result from '%1' on %2").arg(domain, d->origin()));

            // establish dialback connection
            QXmppOutgoingServer *stream = new QXmppOutgoingServer(d->domain, this);
            bool check = connect(stream, SIGNAL(dialbackResponseReceived(QXmppDialback)),
                                 this, SLOT(slotDialbackResponseReceived(QXmppDialback)));
            Q_ASSERT(check);
            Q_UNUSED(check);
            stream->setVerify(d->localStreamId, request.key());
            stream->connectToHost(domain);
        }
        else if (request.command() == QXmppDialback::Verify)
        {
            debug(QString("Received a dialback verify from '%1' on %2").arg(domain, d->origin()));
            emit dialbackRequestReceived(request);
        }

    }
    else if (d->authenticated.contains(QXmppUtils::jidToDomain(stanza.attribute("from"))))
    {
        // relay stanza if the remote party is authenticated
        emit elementReceived(stanza);
    } else {
        warning(QString("Received an element from unverified domain '%1' on %2").arg(QXmppUtils::jidToDomain(stanza.attribute("from")), d->origin()));
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
    QXmppOutgoingServer *stream = qobject_cast<QXmppOutgoingServer*>(sender());
    if (!stream ||
        dialback.command() != QXmppDialback::Verify ||
        dialback.id() != d->localStreamId ||
        dialback.from() != stream->remoteDomain())
        return;

    // relay verify response
    QXmppDialback response;
    response.setCommand(QXmppDialback::Result);
    response.setTo(dialback.from());
    response.setFrom(d->domain);
    response.setType(dialback.type());
    sendPacket(response);

    // check for success
    if (response.type() == QLatin1String("valid"))
    {
        info(QString("Verified incoming domain '%1' on %2").arg(dialback.from(), d->origin()));
        const bool wasConnected = !d->authenticated.isEmpty();
        d->authenticated.insert(dialback.from());
        if (!wasConnected)
            emit connected();
    } else {
        warning(QString("Failed to verify incoming domain '%1' on %2").arg(dialback.from(), d->origin()));
        disconnectFromHost();
    }

    // disconnect dialback
    stream->disconnectFromHost();
    stream->deleteLater();
}

void QXmppIncomingServer::slotSocketDisconnected()
{
    info(QString("Socket disconnected from %1").arg(d->origin()));
    emit disconnected();
}
