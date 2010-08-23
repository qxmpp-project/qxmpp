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
    bool authenticated;
    QString domain;
    QString localStreamId;
};

QXmppIncomingServer::QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent),
    d(new QXmppIncomingServerPrivate)
{
    setObjectName("S2S-in");
    setSocket(socket);
    d->authenticated = false;
    d->domain = domain;
}

QXmppIncomingServer::~QXmppIncomingServer()
{
    delete d;
}

void QXmppIncomingServer::handleStream(const QDomElement &streamElement)
{
    if (!streamElement.attribute("from").isEmpty())
        setObjectName("S2S-in-" + streamElement.attribute("from"));

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
        features.setSecurityMode(QXmppConfiguration::TLSEnabled);
    sendPacket(features);
}

void QXmppIncomingServer::handleStanza(const QDomElement &stanza)
{
    const QString ns = stanza.namespaceURI();

    if (ns == ns_tls && stanza.tagName() == "starttls")
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
        setObjectName("S2S-in-" + domain);

        if (request.command() == QXmppDialback::Result)
        {
            // etablish dialback connection
            QXmppOutgoingServer *stream = new QXmppOutgoingServer(d->domain, this);
            stream->setLogger(logger());
            stream->setObjectName("S2S-dialback-" + domain);
            stream->configuration().setDomain(domain);
            stream->configuration().setHost(domain);
            stream->configuration().setPort(5269);
            bool check = connect(stream, SIGNAL(dialbackResponseReceived(QXmppDialback)),
                                 this, SLOT(slotDialbackResponseReceived(QXmppDialback)));
            Q_ASSERT(check);
            stream->setVerify(d->localStreamId, request.key());
            stream->connectToHost();
        } else {
            emit dialbackRequestReceived(request);
        }

    } else if (d->authenticated) {
        // relay packets if the remote party is authenticated
        bool handled = false;
        emit elementReceived(stanza, handled);
    } else {
        warning("Received an element, but remote party is not authenticated");
    }
}

QString QXmppIncomingServer::localStreamId() const
{
    return d->localStreamId;
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
        dialback.id() != d->localStreamId)
        return;

    // relay verify response
    QXmppDialback response;
    response.setCommand(QXmppDialback::Result);
    response.setTo(dialback.from());
    response.setFrom(d->domain);
    response.setType(dialback.type());
    sendPacket(response);

    // check for success
    if (response.type() == "valid")
    {
        info("Incoming stream is ready");
        d->authenticated = true;
        emit connected();
    } else {
        disconnectFromHost();
    }

    // disconnect dialback
    stream->disconnectFromHost();
    stream->deleteLater();
}


