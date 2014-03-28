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
#include <QTimer>

#include "QXmppBindIq.h"
#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppPasswordChecker.h"
#include "QXmppSasl_p.h"
#include "QXmppSessionIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

#include "QXmppIncomingClient.h"

class QXmppIncomingClientPrivate
{
public:
    QXmppIncomingClientPrivate(QXmppIncomingClient *qq);
    QTimer *idleTimer;

    QString domain;
    QString jid;
    QString resource;
    QXmppPasswordChecker *passwordChecker;
    QXmppSaslServer *saslServer;

    void checkCredentials(const QByteArray &response);
    QString origin() const;

private:
    QXmppIncomingClient *q;
};

QXmppIncomingClientPrivate::QXmppIncomingClientPrivate(QXmppIncomingClient *qq)
    : idleTimer(0)
    , passwordChecker(0)
    , saslServer(0)
    , q(qq)
{
}

void QXmppIncomingClientPrivate::checkCredentials(const QByteArray &response)
{
    QXmppPasswordRequest request;
    request.setDomain(domain);
    request.setUsername(saslServer->username());

    if (saslServer->mechanism() == "PLAIN") {
        request.setPassword(saslServer->password());

        QXmppPasswordReply *reply = passwordChecker->checkPassword(request);
        reply->setParent(q);
        reply->setProperty("__sasl_raw", response);
        QObject::connect(reply, SIGNAL(finished()),
                         q, SLOT(onPasswordReply()));
    } else if (saslServer->mechanism() == "DIGEST-MD5") {
        QXmppPasswordReply *reply = passwordChecker->getDigest(request);
        reply->setParent(q);
        reply->setProperty("__sasl_raw", response);
        QObject::connect(reply, SIGNAL(finished()),
                         q, SLOT(onDigestReply()));
    }
}

QString QXmppIncomingClientPrivate::origin() const
{
    QSslSocket *socket = q->socket();
    if (socket)
        return socket->peerAddress().toString() + " " + QString::number(socket->peerPort());
    else
        return "<unknown>";
}

/// Constructs a new incoming client stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///

QXmppIncomingClient::QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent)
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppIncomingClientPrivate(this);
    d->domain = domain;

    if (socket) {
        check = connect(socket, SIGNAL(disconnected()),
                        this, SLOT(onSocketDisconnected()));
        Q_ASSERT(check);

        setSocket(socket);
    }

    info(QString("Incoming client connection from %1").arg(d->origin()));

    // create inactivity timer
    d->idleTimer = new QTimer(this);
    d->idleTimer->setSingleShot(true);
    check = connect(d->idleTimer, SIGNAL(timeout()),
                    this, SLOT(onTimeout()));
    Q_ASSERT(check);
}

/// Destroys the current stream.
///

QXmppIncomingClient::~QXmppIncomingClient()
{
    delete d;
}

/// Returns true if the socket is connected, the client is authenticated
/// and a resource is bound.
///

bool QXmppIncomingClient::isConnected() const
{
    return QXmppStream::isConnected() &&
           !d->jid.isEmpty() &&
           !d->resource.isEmpty();
}

/// Returns the client's JID.
///

QString QXmppIncomingClient::jid() const
{
    return d->jid;
}

/// Sets the number of seconds after which a client will be disconnected
/// for inactivity.

void QXmppIncomingClient::setInactivityTimeout(int secs)
{
    d->idleTimer->stop();
    d->idleTimer->setInterval(secs * 1000);
    if (d->idleTimer->interval())
        d->idleTimer->start();
}

/// Sets the password checker used to verify client credentials.
///
/// \param checker
///

void QXmppIncomingClient::setPasswordChecker(QXmppPasswordChecker *checker)
{
    d->passwordChecker = checker;
}

/// \cond
void QXmppIncomingClient::handleStream(const QDomElement &streamElement)
{
    if (d->idleTimer->interval())
        d->idleTimer->start();
    if (d->saslServer != 0) {
        delete d->saslServer;
        d->saslServer = 0;
    }

    // start stream
    const QByteArray sessionId = QXmppUtils::generateStanzaHash().toLatin1();
    QString response = QString("<?xml version='1.0'?><stream:stream"
        " xmlns=\"%1\" xmlns:stream=\"%2\""
        " id=\"%3\" from=\"%4\" version=\"1.0\" xml:lang=\"en\">").arg(
        ns_client,
        ns_stream,
        sessionId,
        d->domain.toLatin1());
    sendData(response.toUtf8());

    // check requested domain
    if (streamElement.attribute("to") != d->domain)
    {
        QString response = QString("<stream:error>"
            "<host-unknown xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
            "<text xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\">"
                "This server does not serve %1"
            "</text>"
            "</stream:error>").arg(streamElement.attribute("to"));
        sendData(response.toUtf8());
        disconnectFromHost();
        return;
    }

    // send stream features
    QXmppStreamFeatures features;
    if (socket() && !socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull())
        features.setTlsMode(QXmppStreamFeatures::Enabled);
    if (!d->jid.isEmpty())
    {
        features.setBindMode(QXmppStreamFeatures::Required);
        features.setSessionMode(QXmppStreamFeatures::Enabled);
    }
    else if (d->passwordChecker)
    {
        QStringList mechanisms;
        mechanisms << "PLAIN";
        if (d->passwordChecker->hasGetPassword())
            mechanisms << "DIGEST-MD5";
        features.setAuthMechanisms(mechanisms);
    }
    sendPacket(features);
}

void QXmppIncomingClient::handleStanza(const QDomElement &nodeRecv)
{
    const QString ns = nodeRecv.namespaceURI();

    if (d->idleTimer->interval())
        d->idleTimer->start();

    if (ns == ns_tls && nodeRecv.tagName() == QLatin1String("starttls"))
    {
        sendData("<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
        socket()->flush();
        socket()->startServerEncryption();
        return;
    }
    else if (ns == ns_sasl)
    {
        if (!d->passwordChecker) {
            warning("Cannot perform authentication, no password checker");
            sendPacket(QXmppSaslFailure("temporary-auth-failure"));
            disconnectFromHost();
            return;
        }

        if (nodeRecv.tagName() == QLatin1String("auth")) {
            QXmppSaslAuth auth;
            auth.parse(nodeRecv);

            d->saslServer = QXmppSaslServer::create(auth.mechanism(), this);
            if (!d->saslServer) {
                sendPacket(QXmppSaslFailure("invalid-mechanism"));
                disconnectFromHost();
                return;
            }

            d->saslServer->setRealm(d->domain.toUtf8());

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(auth.value(), challenge);

            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(auth.value());
            } else if (result == QXmppSaslServer::Challenge) {
                sendPacket(QXmppSaslChallenge(challenge));
            } else {
                // FIXME: what condition?
                sendPacket(QXmppSaslFailure());
                disconnectFromHost();
                return;
            }
        }
        else if (nodeRecv.tagName() == QLatin1String("response"))
        {
            QXmppSaslResponse response;
            response.parse(nodeRecv);

            if (!d->saslServer) {
                warning("SASL response received, but no mechanism selected");
                sendPacket(QXmppSaslFailure());
                disconnectFromHost();
                return;
            }

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(response.value(), challenge);
            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(response.value());
            } else if (result == QXmppSaslServer::Succeeded) {
                // authentication succeeded
                d->jid = QString("%1@%2").arg(d->saslServer->username(), d->domain);
                info(QString("Authentication succeeded for '%1' from %2").arg(d->jid, d->origin()));
                updateCounter("incoming-client.auth.success");
                sendPacket(QXmppSaslSuccess());
                handleStart();
            } else {
                // FIXME: what condition?
                sendPacket(QXmppSaslFailure());
                disconnectFromHost();
            }
        }
    }
    else if (ns == ns_client)
    {
        if (nodeRecv.tagName() == QLatin1String("iq"))
        {
            const QString type = nodeRecv.attribute("type");
            if (QXmppBindIq::isBindIq(nodeRecv) && type == QLatin1String("set"))
            {
                QXmppBindIq bindSet;
                bindSet.parse(nodeRecv);
                d->resource = bindSet.resource().trimmed();
                if (d->resource.isEmpty())
                    d->resource = QXmppUtils::generateStanzaHash();
                d->jid = QString("%1/%2").arg(QXmppUtils::jidToBareJid(d->jid), d->resource);

                QXmppBindIq bindResult;
                bindResult.setType(QXmppIq::Result);
                bindResult.setId(bindSet.id());
                bindResult.setJid(d->jid);
                sendPacket(bindResult);

                // bound
                emit connected();
                return;
            }
            else if (QXmppSessionIq::isSessionIq(nodeRecv) && type == QLatin1String("set"))
            {
                QXmppSessionIq sessionSet;
                sessionSet.parse(nodeRecv);

                QXmppIq sessionResult;
                sessionResult.setType(QXmppIq::Result);
                sessionResult.setId(sessionSet.id());
                sessionResult.setTo(d->jid);
                sendPacket(sessionResult);
                return;
            }
        }

        // check the sender is legitimate
        const QString from = nodeRecv.attribute("from");
        if (!from.isEmpty() && from != d->jid && from != QXmppUtils::jidToBareJid(d->jid))
        {
            warning(QString("Received a stanza from unexpected JID %1").arg(from));
            return;
        }

        // process unhandled stanzas
        if (nodeRecv.tagName() == QLatin1String("iq") ||
            nodeRecv.tagName() == QLatin1String("message") ||
            nodeRecv.tagName() == QLatin1String("presence"))
        {
            QDomElement nodeFull(nodeRecv);

            // if the sender is empty, set it to the appropriate JID
            if (nodeFull.attribute("from").isEmpty())
            {
                if (nodeFull.tagName() == QLatin1String("presence") &&
                    (nodeFull.attribute("type") == QLatin1String("subscribe") ||
                    nodeFull.attribute("type") == QLatin1String("subscribed")))
                    nodeFull.setAttribute("from", QXmppUtils::jidToBareJid(d->jid));
                else
                    nodeFull.setAttribute("from", d->jid);
            }

            // if the recipient is empty, set it to the local domain
            if (nodeFull.attribute("to").isEmpty())
                nodeFull.setAttribute("to", d->domain);

            // emit stanza for processing by server
            emit elementReceived(nodeFull);
        }
    }
}
/// \endcond

void QXmppIncomingClient::onDigestReply()
{
    QXmppPasswordReply *reply = qobject_cast<QXmppPasswordReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    if (reply->error() == QXmppPasswordReply::TemporaryError) {
        warning(QString("Temporary authentication failure for '%1' from %2").arg(d->saslServer->username(), d->origin()));
        updateCounter("incoming-client.auth.temporary-auth-failure");
        sendPacket(QXmppSaslFailure("temporary-auth-failure"));
        disconnectFromHost();
        return;
    }

    QByteArray challenge;
    d->saslServer->setPasswordDigest(reply->digest());

    QXmppSaslServer::Response result = d->saslServer->respond(reply->property("__sasl_raw").toByteArray(), challenge);
    if (result != QXmppSaslServer::Challenge) {
        warning(QString("Authentication failed for '%1' from %2").arg(d->saslServer->username(), d->origin()));
        updateCounter("incoming-client.auth.not-authorized");
        sendPacket(QXmppSaslFailure("not-authorized"));
        disconnectFromHost();
        return;
    }

    // send new challenge
    sendPacket(QXmppSaslChallenge(challenge));
}

void QXmppIncomingClient::onPasswordReply()
{
    QXmppPasswordReply *reply = qobject_cast<QXmppPasswordReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    const QString jid = QString("%1@%2").arg(d->saslServer->username(), d->domain);
    switch (reply->error()) {
    case QXmppPasswordReply::NoError:
        d->jid = jid;
        info(QString("Authentication succeeded for '%1' from %2").arg(d->jid, d->origin()));
        updateCounter("incoming-client.auth.success");
        sendPacket(QXmppSaslSuccess());
        handleStart();
        break;
    case QXmppPasswordReply::AuthorizationError:
        warning(QString("Authentication failed for '%1' from %2").arg(jid, d->origin()));
        updateCounter("incoming-client.auth.not-authorized");
        sendPacket(QXmppSaslFailure("not-authorized"));
        disconnectFromHost();
        break;
    case QXmppPasswordReply::TemporaryError:
        warning(QString("Temporary authentication failure for '%1' from %2").arg(jid, d->origin()));
        updateCounter("incoming-client.auth.temporary-auth-failure");
        sendPacket(QXmppSaslFailure("temporary-auth-failure"));
        disconnectFromHost();
        break;
    }
}

void QXmppIncomingClient::onSocketDisconnected()
{
    info(QString("Socket disconnected for '%1' from %2").arg(d->jid, d->origin()));
    emit disconnected();
}

void QXmppIncomingClient::onTimeout()
{
    warning(QString("Idle timeout for '%1' from %2").arg(d->jid, d->origin()));
    disconnectFromHost();

    // make sure disconnected() gets emitted no matter what
    QTimer::singleShot(30, this, SIGNAL(disconnected()));
}


