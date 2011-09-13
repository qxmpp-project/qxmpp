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
#include <QTimer>

#include "QXmppBindIq.h"
#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppPasswordChecker.h"
#include "QXmppSaslAuth.h"
#include "QXmppSessionIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

#include "QXmppIncomingClient.h"

class QXmppIncomingClientPrivate
{
public:
    QTimer *idleTimer;

    QString domain;
    QString username;
    QString resource;
    QString jid;
    QXmppPasswordChecker *passwordChecker;
    QXmppSaslDigestMd5 saslDigest;
    int saslDigestStep;
    QString saslDigestUsername;
};

/// Constructs a new incoming client stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///

QXmppIncomingClient::QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent),
    d(new QXmppIncomingClientPrivate)
{
    d->passwordChecker = 0;
    d->domain = domain;
    d->saslDigestStep = 0;

    if (socket) {
        info(QString("Incoming client connection from %1 %2").arg(
            socket->peerAddress().toString(),
            QString::number(socket->peerPort())));
        setSocket(socket);
    }

    // create inactivity timer
    d->idleTimer = new QTimer(this);
    d->idleTimer->setSingleShot(true);
    bool check = connect(d->idleTimer, SIGNAL(timeout()),
                    this, SLOT(onTimeout()));
    Q_ASSERT(check);
    Q_UNUSED(check);
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
           !d->username.isEmpty() &&
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

void QXmppIncomingClient::handleStream(const QDomElement &streamElement)
{
    if (d->idleTimer->interval())
        d->idleTimer->start();
    d->saslDigestStep = 0;
    d->saslDigestUsername.clear();

    // start stream
    const QByteArray sessionId = generateStanzaHash().toAscii();
    QString response = QString("<?xml version='1.0'?><stream:stream"
        " xmlns=\"%1\" xmlns:stream=\"%2\""
        " id=\"%3\" from=\"%4\" version=\"1.0\" xml:lang=\"en\">").arg(
        ns_client,
        ns_stream,
        sessionId,
        d->domain.toAscii());
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
    if (!d->username.isEmpty())
    {
        features.setBindMode(QXmppStreamFeatures::Required);
        features.setSessionMode(QXmppStreamFeatures::Enabled);
    }
    else if (d->passwordChecker)
    {
        QList<QXmppConfiguration::SASLAuthMechanism> mechanisms;
        mechanisms << QXmppConfiguration::SASLPlain;
        if (d->passwordChecker->hasGetPassword())
            mechanisms << QXmppConfiguration::SASLDigestMD5;
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
        if (nodeRecv.tagName() == QLatin1String("auth"))
        {
            const QString mechanism = nodeRecv.attribute("mechanism");
            if (mechanism == QLatin1String("PLAIN"))
            {
                QList<QByteArray> auth = QByteArray::fromBase64(nodeRecv.text().toAscii()).split('\0');
                if (auth.size() != 3)
                {
                    sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><incorrect-encoding/></failure>");
                    disconnectFromHost();
                    return;
                }

                QXmppPasswordRequest request;
                request.setDomain(d->domain);
                request.setUsername(QString::fromUtf8(auth[1]));
                request.setPassword(QString::fromUtf8(auth[2]));
                if (!d->passwordChecker) {
                    // FIXME: what type of failure?
                    warning(QString("Cannot authenticate '%1', no password checker").arg(request.username()));
                    sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
                    disconnectFromHost();
                    return;
                }

                QXmppPasswordReply *reply = d->passwordChecker->checkPassword(request);
                reply->setParent(this);
                reply->setProperty("__sasl_username", request.username());
                connect(reply, SIGNAL(finished()), this, SLOT(onPasswordReply()));
            }
            else if (mechanism == QLatin1String("DIGEST-MD5"))
            {
                // generate nonce
                d->saslDigest.setNonce(QXmppSaslDigestMd5::generateNonce());
                d->saslDigest.setQop("auth");
                d->saslDigestStep = 1;

                QMap<QByteArray, QByteArray> challenge;
                challenge["nonce"] = d->saslDigest.nonce();
                challenge["realm"] = d->domain.toUtf8();
                challenge["qop"] = d->saslDigest.qop();
                challenge["charset"] = "utf-8";
                challenge["algorithm"] = "md5-sess";

                const QByteArray data = QXmppSaslDigestMd5::serializeMessage(challenge).toBase64();
                sendData("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>" + data +"</challenge>");
            }
            else
            {
                // unsupported method
                sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'></failure>");
                disconnectFromHost();
                return;
            }
        }
        else if (nodeRecv.tagName() == QLatin1String("response"))
        {
            if (d->saslDigestStep == 1)
            {
                const QByteArray raw = QByteArray::fromBase64(nodeRecv.text().toAscii());
                QMap<QByteArray, QByteArray> saslResponse = QXmppSaslDigestMd5::parseMessage(raw);

                // check credentials
                const QString username = QString::fromUtf8(saslResponse.value("username"));
                if (!d->passwordChecker) {
                    // FIXME: what type of failure?
                    warning(QString("Cannot authenticate '%1', no password checker").arg(username));
                    sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
                    disconnectFromHost();
                    return;
                }

                QXmppPasswordRequest request;
                request.setUsername(username);
                request.setDomain(d->domain);

                QXmppPasswordReply *reply = d->passwordChecker->getDigest(request);
                reply->setParent(this);
                reply->setProperty("__sasl_raw", raw);
                connect(reply, SIGNAL(finished()), this, SLOT(onDigestReply()));
            }
            else if (d->saslDigestStep == 2)
            {
                // authentication succeeded
                d->saslDigestStep = 3;
                d->username = d->saslDigestUsername;
                d->jid = QString("%1@%2").arg(d->username, d->domain);
                info(QString("Authentication succeeded for '%1'").arg(d->username));
                sendData("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
                handleStart();
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
                    d->resource = generateStanzaHash();
                d->jid = QString("%1@%2/%3").arg(d->username, d->domain, d->resource);

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
        if (!from.isEmpty() && from != d->jid && from != jidToBareJid(d->jid))
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
                    nodeFull.setAttribute("from", jidToBareJid(d->jid));
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

void QXmppIncomingClient::onDigestReply()
{
    QXmppPasswordReply *reply = qobject_cast<QXmppPasswordReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    const QMap<QByteArray, QByteArray> saslResponse = QXmppSaslDigestMd5::parseMessage(reply->property("__sasl_raw").toByteArray());
    const QString username = QString::fromUtf8(saslResponse.value("username"));
    if (reply->error() == QXmppPasswordReply::TemporaryError) {
        warning(QString("Temporary authentication failure for '%1'").arg(username));
        sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><temporary-auth-failure/></failure>");
        disconnectFromHost();
        return;
    }

    d->saslDigest.setSecret(reply->digest());
    d->saslDigest.setDigestUri(saslResponse.value("digest-uri"));
    d->saslDigest.setNc(saslResponse.value("nc"));
    d->saslDigest.setCnonce(saslResponse.value("cnonce"));
    if (saslResponse.value("response") != d->saslDigest.calculateDigest(
            QByteArray("AUTHENTICATE:") + d->saslDigest.digestUri()))
    {
        sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>");
        disconnectFromHost();
        return;
    }

    // send new challenge
    d->saslDigestUsername = username;
    d->saslDigestStep = 2;
    QMap<QByteArray, QByteArray> challenge;
    challenge["rspauth"] = d->saslDigest.calculateDigest(
        QByteArray(":") + d->saslDigest.digestUri());
    const QByteArray data = QXmppSaslDigestMd5::serializeMessage(challenge).toBase64();
    sendData("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>" + data +"</challenge>");
}

void QXmppIncomingClient::onPasswordReply()
{
    QXmppPasswordReply *reply = qobject_cast<QXmppPasswordReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    const QString username = reply->property("__sasl_username").toString();
    switch (reply->error()) {
    case QXmppPasswordReply::NoError:
        d->username = username;
        d->jid = QString("%1@%2").arg(d->username, d->domain);
        info(QString("Authentication succeeded for '%1'").arg(username));
        sendData("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
        handleStart();
        break;
    case QXmppPasswordReply::AuthorizationError:
        warning(QString("Authentication failed for '%1'").arg(username));
        sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>");
        disconnectFromHost();
        break;
    case QXmppPasswordReply::TemporaryError:
        warning(QString("Temporary authentication failure for '%1'").arg(username));
        sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><temporary-auth-failure/></failure>");
        disconnectFromHost();
        break;
    }
}

void QXmppIncomingClient::onTimeout()
{
    warning(QString("Idle timeout for '%1'").arg(d->jid));
    disconnectFromHost();

    // make sure disconnected() gets emitted no matter what
    QTimer::singleShot(30, this, SIGNAL(disconnected()));
}


