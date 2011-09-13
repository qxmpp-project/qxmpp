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
    QSet<QString> authenticated;
    QString domain;
    QString localStreamId;
};

/// Constructs a new incoming server stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///

QXmppIncomingServer::QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent),
    d(new QXmppIncomingServerPrivate)
{
    d->domain = domain;

    if (socket) {
        info(QString("Incoming server connection from %1 %2").arg(
            socket->peerAddress().toString(),
            QString::number(socket->peerPort())));
        setSocket(socket);
    }
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

void QXmppIncomingServer::handleStream(const QDomElement &streamElement)
{
    const QString from = streamElement.attribute("from");
    if (!from.isEmpty())
        info(QString("Incoming server stream from %1").arg(from));

    // start stream
    d->localStreamId = generateStanzaHash().toAscii();
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
            warning("Invalid dialback received");
            return;
        }

        const QString domain = request.from();
        if (request.command() == QXmppDialback::Result)
        {
            debug(QString("Received a dialback result from %1").arg(domain));

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
            debug(QString("Received a dialback verify from %1").arg(domain));
            emit dialbackRequestReceived(request);
        }

    }
    else if (d->authenticated.contains(jidToDomain(stanza.attribute("from"))))
    {
        // relay stanza if the remote party is authenticated
        emit elementReceived(stanza);
    } else {
        warning(QString("Received an element from unverified domain %1").arg(jidToDomain(stanza.attribute("from"))));
        disconnectFromHost();
    }
}

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
        info(QString("Verified incoming domain %1").arg(dialback.from()));
        const bool wasConnected = !d->authenticated.isEmpty();
        d->authenticated.insert(dialback.from());
        if (!wasConnected)
            emit connected();
    } else {
        warning(QString("Failed to verify incoming domain %1").arg(dialback.from()));
        disconnectFromHost();
    }

    // disconnect dialback
    stream->disconnectFromHost();
    stream->deleteLater();
}

