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
#include "QXmppSrvInfo.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

class QXmppOutgoingServerPrivate
{
public:
    QString localDomain;
    QString localStreamKey;
    QString remoteDomain;
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
    : QXmppStream(parent),
    d(new QXmppOutgoingServerPrivate)
{
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    d->localDomain = domain;
    d->ready = false;

    bool check = connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                         this, SLOT(slotSslErrors(QList<QSslError>)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

/// Destroys the stream.
///

QXmppOutgoingServer::~QXmppOutgoingServer()
{
    delete d;
}

void QXmppOutgoingServer::connectToHost(const QString &domain)
{
    d->remoteDomain = domain;

    // lookup server for domain
    debug(QString("Looking up server for domain %1").arg(domain));
    QXmppSrvInfo::lookupService("_xmpp-server._tcp." + domain, this,
                                SLOT(connectHost(serviceInfo)));
}

void QXmppOutgoingServer::connectToHost(const QXmppSrvInfo &serviceInfo)
{
    QString host;
    quint16 port;

    if (!serviceInfo.records().isEmpty())
    {
        // take the first returned record
        host = serviceInfo.records().first().target();
        port = serviceInfo.records().first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2").arg(d->remoteDomain, serviceInfo.errorString()));
        host = d->remoteDomain;
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

void QXmppOutgoingServer::handleStream(const QDomElement &streamElement)
{
    Q_UNUSED(streamElement);
}

void QXmppOutgoingServer::handleStanza(const QDomElement &stanza)
{
    const QString ns = stanza.namespaceURI();

    if(QXmppStreamFeatures::isStreamFeatures(stanza))
    {
        QXmppStreamFeatures features;
        features.parse(stanza);

        if (!socket()->isEncrypted())
        {
            // check we can satisfy TLS constraints
            if (!socket()->supportsSsl() &&
                 features.tlsMode() == QXmppStreamFeatures::Required)
            {
                warning("Disconnecting as TLS is required, but SSL support is not available");
                disconnectFromHost();
                return;
            }

            // enable TLS if possible
            if (socket()->supportsSsl() &&
                features.tlsMode() != QXmppStreamFeatures::Disabled)
            {
                sendData("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
                return;
            }
        }

        if (!d->localStreamKey.isEmpty())
        {
            // send dialback key
            QXmppDialback dialback;
            dialback.setCommand(QXmppDialback::Result);
            dialback.setFrom(d->localDomain);
            dialback.setTo(d->remoteDomain);
            dialback.setKey(d->localStreamKey);
            sendPacket(dialback);
        }
        else if (!d->verifyId.isEmpty() && !d->verifyKey.isEmpty())
        {
            // send dialback verify
            QXmppDialback verify;
            verify.setCommand(QXmppDialback::Verify);
            verify.setId(d->verifyId);
            verify.setFrom(d->localDomain);
            verify.setTo(d->remoteDomain);
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
            response.to() != d->localDomain ||
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

QString QXmppOutgoingServer::remoteDomain() const
{
    return d->remoteDomain;
}

void QXmppOutgoingServer::slotSslErrors(const QList<QSslError> &errors)
{
    warning("SSL errors");
    for(int i = 0; i < errors.count(); ++i)
        warning(errors.at(i).errorString());
    socket()->ignoreSslErrors();
}

