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
#include <QTimer>

#include "QXmppBind.h"
#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppSession.h"
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
    QXmppPasswordChecker *passwordChecker;
    QByteArray saslNonce;
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

    setObjectName("C2S");
    setSocket(socket);

    // create inactivity timer
    d->idleTimer = new QTimer(this);
    d->idleTimer->setInterval(70000);
    d->idleTimer->setSingleShot(true);
    bool check = connect(d->idleTimer, SIGNAL(timeout()),
                    this, SLOT(slotTimeout()));
    Q_ASSERT(check);
}

/// Destroys the current stream.
///

QXmppIncomingClient::~QXmppIncomingClient()
{
    delete d;
}

/// Returns true if the client is authenticated and a resource is bound.
///

bool QXmppIncomingClient::isConnected() const
{
    return !d->username.isEmpty() && !d->resource.isEmpty();
}

/// Returns the client's JID.
///

QString QXmppIncomingClient::jid() const
{
    if (d->username.isEmpty())
        return QString();
    QString jid = d->username + "@" + d->domain;
    if (!d->resource.isEmpty())
        jid += "/" + d->resource;
    return jid;
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
    Q_UNUSED(streamElement);

    d->idleTimer->start();

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

    // send stream features
    QXmppStreamFeatures features;
    if (!socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull())
        features.setSecurityMode(QXmppConfiguration::TLSEnabled);
    if (!d->username.isEmpty())
    {
        features.setBindAvailable(true);
        features.setSessionAvailable(true);
    }
    else
    {
        QList<QXmppConfiguration::SASLAuthMechanism> mechanisms;
        mechanisms << QXmppConfiguration::SASLPlain;
        mechanisms << QXmppConfiguration::SASLDigestMD5;
        features.setAuthMechanisms(mechanisms);
    }
    sendPacket(features);
}

void QXmppIncomingClient::handleStanza(const QDomElement &nodeRecv)
{
    const QString ns = nodeRecv.namespaceURI();

    d->idleTimer->start();

    if (ns == ns_tls && nodeRecv.tagName() == "starttls")
    {
        sendData("<proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
        socket()->flush();
        socket()->startServerEncryption();
        return;
    }
    else if (ns == ns_sasl)
    {
        if (nodeRecv.tagName() == "auth")
        {
            const QString mechanism = nodeRecv.attribute("mechanism");
            if (mechanism == "PLAIN")
            {
                QList<QByteArray> auth = QByteArray::fromBase64(nodeRecv.text().toAscii()).split('\0');
                if (auth.size() != 3)
                {
                    sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
                    disconnectFromHost();
                    return;
                }

                const QString username = QString::fromUtf8(auth[1]);
                const QString password = QString::fromUtf8(auth[2]);
                if (d->passwordChecker && d->passwordChecker->check(username, password))
                {
                    d->username = username;
                    sendData("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
                } else {
                    sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>");
                    disconnectFromHost();
                    return;
                }
            }
            else if (mechanism == "DIGEST-MD5")
            {
                // generate nonce
                QByteArray nonce(32, 'm');
                for(int n = 0; n < nonce.size(); ++n)
                    nonce[n] = (char)(256.0*qrand()/(RAND_MAX+1.0));
                d->saslNonce = nonce.toBase64();

                QMap<QByteArray, QByteArray> challenge;
                challenge["nonce"] = d->saslNonce;
                challenge["realm"] = d->domain.toUtf8();
                challenge["qop"] = "auth";
                challenge["charset"] = "utf-8";
                challenge["algorithm"] = "md5-sess";

                const QByteArray data = serializeDigestMd5(challenge).toBase64();
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
        else if (nodeRecv.tagName() == "response")
        {
            const QByteArray raw = QByteArray::fromBase64(nodeRecv.text().toAscii());
            QMap<QByteArray, QByteArray> response = parseDigestMd5(raw);

            // check credentials
            const QString username = QString::fromUtf8(response.value("username"));
            QString password;
            if (!d->passwordChecker || !d->passwordChecker->get(username, password))
            {
                sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>");
                disconnectFromHost();
                return;
            }
            const QByteArray a1 = username.toUtf8() + ':' + d->domain.toUtf8() + ':' + password.toUtf8();
            const QByteArray remote = QByteArray::fromHex(response["response"]);
            if (remote != calculateDigestMd5(a1,
                d->saslNonce,
                response.value("nc"),
                response.value("cnonce"),
                response.value("digest-uri"),
                QByteArray()))
            {
                sendData("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>");
                disconnectFromHost();
                return;
            }

            // authentication succeeded
            d->username = username;
            sendData("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>");
        }
    }
    else if (ns == ns_client)
    {
        if (nodeRecv.tagName() == "iq")
        {
            const QString type = nodeRecv.attribute("type");
            if (QXmppBind::isBind(nodeRecv) && type == "set")
            {
                QXmppBind bindSet;
                bindSet.parse(nodeRecv);
                d->resource = bindSet.resource();

                QXmppBind bindResult(QXmppIq::Result);
                bindResult.setId(bindSet.id());
                bindResult.setJid(jid());
                sendPacket(bindResult);

                // bound
                emit connected();
            }
            else if (QXmppSession::isSession(nodeRecv) && type == "set")
            {
                QXmppSession sessionSet;
                sessionSet.parse(nodeRecv);

                QXmppSession sessionResult(QXmppIq::Result);
                sessionResult.setId(sessionSet.id());
                sendPacket(sessionResult);
            }
            else
            {
                QDomElement nodeFull(nodeRecv);
                nodeFull.setAttribute("from", jid());
                bool handled = false;
                emit elementReceived(nodeFull, handled);
            }
        }
        else if (nodeRecv.tagName() == "message" || nodeRecv.tagName() == "presence")
        {
            QDomElement nodeFull(nodeRecv);
            nodeFull.setAttribute("from", jid());
            bool handled = false;
            emit elementReceived(nodeFull, handled);
        }
    }
}

void QXmppIncomingClient::slotTimeout()
{
    warning(QString("Idle timeout for %1").arg(jid()));
    disconnectFromHost();
}

