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
#include "QXmppOutgoingServer.h"
#include "QXmppServiceInfo.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

class QXmppOutgoingServerPrivate
{
public:
    QString domain;
    QString localStreamKey;
    QString verifyId;
    QString verifyKey;
    bool ready;
};

/// Constructs a new outgoing server-to-server stream.
///
/// \param socket
/// \param domain the local domain
/// \param parent the parent object
///

QXmppOutgoingServer::QXmppOutgoingServer(const QString &domain, QObject *parent)
    : QXmppOutgoingClient(parent),
    d(new QXmppOutgoingServerPrivate)
{
    d->domain = domain;
    d->ready = false;
    configuration().setKeepAliveInterval(0);
}

/// Destroys the stream.
///

QXmppOutgoingServer::~QXmppOutgoingServer()
{
    delete d;
}

void QXmppOutgoingServer::connectToHost()
{
    const QString domain = configuration().domain();
    QString host;
    quint16 port;

    // lookup server for domain
    debug(QString("Looking up server for domain %1").arg(domain));
    QXmppServiceInfo serviceInfo = QXmppServiceInfo::fromName("_xmpp-server._tcp." + domain);
    if (!serviceInfo.records().isEmpty())
    {
        // take the first returned record
        host = serviceInfo.records().first().hostName();
        port = serviceInfo.records().first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2").arg(domain, serviceInfo.errorString()));
        host = domain;
        port = 5269;
    }

    // connect to server
    info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
    socket()->connectToHost(host, port);
}

void QXmppOutgoingServer::handleStart()
{
    QString data = QString("<?xml version='1.0'?><stream:stream"
        " xmlns='%1' xmlns:db='%2' xmlns:stream='%3' version='1.0'>").arg(
            ns_server,
            ns_server_dialback,
            ns_stream);
    sendData(data.toUtf8());
}

void QXmppOutgoingServer::handleStanza(const QDomElement &stanza)
{
    const QString ns = stanza.namespaceURI();

    if(QXmppStreamFeatures::isStreamFeatures(stanza))
    {
        QXmppStreamFeatures features;
        features.parse(stanza);
        if (features.tlsMode() != QXmppStreamFeatures::Disabled)
        {
            // let QXmppOutgoingClient handle TLS
            QXmppOutgoingClient::handleStanza(stanza);
        }
        else if (!d->localStreamKey.isEmpty())
        {
            // send dialback key
            QXmppDialback dialback;
            dialback.setCommand(QXmppDialback::Result);
            dialback.setFrom(d->domain);
            dialback.setTo(configuration().domain());
            dialback.setKey(d->localStreamKey);
            sendPacket(dialback);
        }
        else if (!d->verifyId.isEmpty() && !d->verifyKey.isEmpty())
        {
            // send dialback verify
            QXmppDialback verify;
            verify.setCommand(QXmppDialback::Verify);
            verify.setId(d->verifyId);
            verify.setTo(configuration().domain());
            verify.setFrom(d->domain);
            verify.setKey(d->verifyKey);
            sendPacket(verify);
        }
    }
    else if (ns == ns_tls)
    {
        if (stanza.tagName() == "proceed")
        {
            debug("Starting encryption");
            socket()->startClientEncryption();
            return;
        }
    }
    else if (QXmppDialback::isDialback(stanza))
    {
        QXmppDialback response;
        response.parse(stanza);

        // check the request is valid
        if (response.from().isEmpty() ||
            response.to() != d->domain ||
            response.type().isEmpty()) 
        {
            warning("Invalid dialback response received");
            return;
        }
        if (response.command() == QXmppDialback::Result)
        {
            if (response.type() == "valid")
            {
                info("Outgoing stream is ready");
                d->ready = true;
                emit connected();
            }
        }
        else if (response.command() == QXmppDialback::Verify)
        {
            emit dialbackResponseReceived(response);
        }

    }
}

/// Returns true if the socket is connected and authentication succeeded.
///

bool QXmppOutgoingServer::isConnected() const
{
    return QXmppStream::isConnected() && d->ready;
}

QString QXmppOutgoingServer::localStreamKey() const
{
    return d->localStreamKey;
}

void QXmppOutgoingServer::setLocalStreamKey(const QString &key)
{
    d->localStreamKey = key;
}

void QXmppOutgoingServer::setVerify(const QString &id, const QString &key)
{
    d->verifyId = id;
    d->verifyKey = key;
}

