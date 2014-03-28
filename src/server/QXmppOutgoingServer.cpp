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
#include <QSslKey>
#include <QSslSocket>
#include <QTimer>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QDnsLookup>
#else
#include "qdnslookup.h"
#endif

#include "QXmppConstants.h"
#include "QXmppDialback.h"
#include "QXmppOutgoingServer.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

class QXmppOutgoingServerPrivate
{
public:
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

/// Constructs a new outgoing server-to-server stream.
///
/// \param domain the local domain
/// \param parent the parent object
///

QXmppOutgoingServer::QXmppOutgoingServer(const QString &domain, QObject *parent)
    : QXmppStream(parent),
    d(new QXmppOutgoingServerPrivate)
{
    bool check;
    Q_UNUSED(check);

    // socket initialisation
    QSslSocket *socket = new QSslSocket(this);
    setSocket(socket);

    check = connect(socket, SIGNAL(disconnected()),
                    this, SLOT(_q_socketDisconnected()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    // DNS lookups
    check = connect(&d->dns, SIGNAL(finished()),
                    this, SLOT(_q_dnsLookupFinished()));
    Q_ASSERT(check);

    d->dialbackTimer = new QTimer(this);
    d->dialbackTimer->setInterval(5000);
    d->dialbackTimer->setSingleShot(true);
    check = connect(d->dialbackTimer, SIGNAL(timeout()),
                    this, SLOT(sendDialback()));
    Q_ASSERT(check);

    d->localDomain = domain;
    d->ready = false;

    check = connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                    this, SLOT(slotSslErrors(QList<QSslError>)));
    Q_ASSERT(check);
}

/// Destroys the stream.
///

QXmppOutgoingServer::~QXmppOutgoingServer()
{
    delete d;
}

/// Attempts to connect to an XMPP server for the specified \a domain.
///
/// \param domain

void QXmppOutgoingServer::connectToHost(const QString &domain)
{
    d->remoteDomain = domain;

    // lookup server for domain
    debug(QString("Looking up server for domain %1").arg(domain));
    d->dns.setName("_xmpp-server._tcp." + domain);
    d->dns.setType(QDnsLookup::SRV);
    d->dns.lookup();
}

void QXmppOutgoingServer::_q_dnsLookupFinished()
{
    QString host;
    quint16 port;

    if (d->dns.error() == QDnsLookup::NoError &&
        !d->dns.serviceRecords().isEmpty()) {
        // take the first returned record
        host = d->dns.serviceRecords().first().target();
        port = d->dns.serviceRecords().first().port();
    } else {
        // as a fallback, use domain as the host name
        warning(QString("Lookup for domain %1 failed: %2")
                .arg(d->dns.name(), d->dns.errorString()));
        host = d->remoteDomain;
        port = 5269;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    // set the name the SSL certificate should match
    socket()->setPeerVerifyName(d->remoteDomain);
#endif

    // connect to server
    info(QString("Connecting to %1:%2").arg(host, QString::number(port)));
    socket()->connectToHost(host, port);
}

void QXmppOutgoingServer::_q_socketDisconnected()
{
    debug("Socket disconnected");
    emit disconnected();
}

/// \cond

void QXmppOutgoingServer::handleStart()
{
    QXmppStream::handleStart();

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

    // gmail.com servers are broken: they never send <stream:features>,
    // so we schedule sending the dialback in a couple of seconds
    d->dialbackTimer->start();
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

        // send dialback if needed
        d->dialbackTimer->stop();
        sendDialback();
    }
    else if (ns == ns_tls)
    {
        if (stanza.tagName() == QLatin1String("proceed"))
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
            if (response.type() == QLatin1String("valid"))
            {
                info(QString("Outgoing server stream to %1 is ready").arg(response.from()));
                d->ready = true;

                // send queued data
                foreach (const QByteArray &data, d->dataQueue)
                    sendData(data);
                d->dataQueue.clear();

                // emit signal
                emit connected();
            }
        }
        else if (response.command() == QXmppDialback::Verify)
        {
            emit dialbackResponseReceived(response);
        }

    }
}
/// \endcond

/// Returns true if the socket is connected and authentication succeeded.
///

bool QXmppOutgoingServer::isConnected() const
{
    return QXmppStream::isConnected() && d->ready;
}

/// Returns the stream's local dialback key.

QString QXmppOutgoingServer::localStreamKey() const
{
    return d->localStreamKey;
}

/// Sets the stream's local dialback key.
///
/// \param key

void QXmppOutgoingServer::setLocalStreamKey(const QString &key)
{
    d->localStreamKey = key;
}

/// Sets the stream's verification information.
///
/// \param id
/// \param key

void QXmppOutgoingServer::setVerify(const QString &id, const QString &key)
{
    d->verifyId = id;
    d->verifyKey = key;
}

/// Sends or queues data until connected.
///
/// \param data

void QXmppOutgoingServer::queueData(const QByteArray &data)
{
    if (isConnected())
        sendData(data);
    else
        d->dataQueue.append(data);
}

/// Returns the remote server's domain.

QString QXmppOutgoingServer::remoteDomain() const
{
    return d->remoteDomain;
}

void QXmppOutgoingServer::sendDialback()
{
    if (!d->localStreamKey.isEmpty())
    {
        // send dialback key
        debug(QString("Sending dialback result to %1").arg(d->remoteDomain));
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
        debug(QString("Sending dialback verify to %1").arg(d->remoteDomain));
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
    warning("SSL errors");
    for(int i = 0; i < errors.count(); ++i)
        warning(errors.at(i).errorString());
    socket()->ignoreSslErrors();
}

void QXmppOutgoingServer::socketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    emit disconnected();
}

