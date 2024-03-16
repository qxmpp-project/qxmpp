// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIncomingClient.h"

#include "QXmppBindIq.h"
#include "QXmppConstants_p.h"
#include "QXmppPasswordChecker.h"
#include "QXmppSasl_p.h"
#include "QXmppStartTlsPacket.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QHostAddress>
#include <QSslKey>
#include <QSslSocket>
#include <QStringBuilder>
#include <QTimer>

using namespace QXmpp::Private;

class QXmppIncomingClientPrivate
{
public:
    QXmppIncomingClientPrivate(QXmppIncomingClient *qq);
    QTimer *idleTimer = nullptr;

    QString domain;
    QString jid;
    QString resource;
    QXmppPasswordChecker *passwordChecker = nullptr;
    std::unique_ptr<QXmppSaslServer> saslServer;

    void checkCredentials(const QByteArray &response);
    QString origin() const;

private:
    QXmppIncomingClient *q;
};

QXmppIncomingClientPrivate::QXmppIncomingClientPrivate(QXmppIncomingClient *qq)
    : q(qq)
{
}

void QXmppIncomingClientPrivate::checkCredentials(const QByteArray &response)
{
    QXmppPasswordRequest request;
    request.setDomain(domain);
    request.setUsername(saslServer->username());

    if (saslServer->mechanism() == u"PLAIN") {
        request.setPassword(saslServer->password());

        QXmppPasswordReply *reply = passwordChecker->checkPassword(request);
        reply->setParent(q);
        reply->setProperty("__sasl_raw", response);
        QObject::connect(reply, &QXmppPasswordReply::finished,
                         q, &QXmppIncomingClient::onPasswordReply);
    } else if (saslServer->mechanism() == u"DIGEST-MD5") {
        QXmppPasswordReply *reply = passwordChecker->getDigest(request);
        reply->setParent(q);
        reply->setProperty("__sasl_raw", response);
        QObject::connect(reply, &QXmppPasswordReply::finished,
                         q, &QXmppIncomingClient::onDigestReply);
    }
}

QString QXmppIncomingClientPrivate::origin() const
{
    QSslSocket *socket = q->socket();
    if (socket) {
        return socket->peerAddress().toString() % u' ' % QString::number(socket->peerPort());
    } else {
        return QStringLiteral("<unknown>");
    }
}

///
/// Constructs a new incoming client stream.
///
/// \param socket The socket for the XMPP stream.
/// \param domain The local domain.
/// \param parent The parent QObject for the stream (optional).
///
QXmppIncomingClient::QXmppIncomingClient(QSslSocket *socket, const QString &domain, QObject *parent)
    : QXmppStream(parent),
      d(std::make_unique<QXmppIncomingClientPrivate>(this))
{
    d->domain = domain;

    if (socket) {
        connect(socket, &QAbstractSocket::disconnected,
                this, &QXmppIncomingClient::onSocketDisconnected);

        setSocket(socket);
    }

    info(QStringLiteral("Incoming client connection from %1").arg(d->origin()));

    // create inactivity timer
    d->idleTimer = new QTimer(this);
    d->idleTimer->setSingleShot(true);
    connect(d->idleTimer, &QTimer::timeout,
            this, &QXmppIncomingClient::onTimeout);
}

QXmppIncomingClient::~QXmppIncomingClient() = default;

///
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
    if (d->idleTimer->interval()) {
        d->idleTimer->start();
    }
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
    if (d->idleTimer->interval()) {
        d->idleTimer->start();
    }
    d->saslServer.reset();

    // start stream
    const QByteArray sessionId = QXmppUtils::generateStanzaHash().toLatin1();
    QString response = QStringLiteral("<?xml version='1.0'?><stream:stream xmlns=\"%1\" "
                                      "xmlns:stream=\"%2\" id=\"%3\" from=\"%4\" "
                                      "version=\"1.0\" xml:lang=\"en\">")
                           .arg(
                               ns_client,
                               ns_stream,
                               QString::fromUtf8(sessionId),
                               d->domain);
    sendData(response.toUtf8());

    // check requested domain
    if (streamElement.attribute(QStringLiteral("to")) != d->domain) {
        QString response = QStringLiteral("<stream:error>"
                                          "<host-unknown xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
                                          "<text xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\">"
                                          "This server does not serve %1"
                                          "</text>"
                                          "</stream:error>")
                               .arg(streamElement.attribute(QStringLiteral("to")));
        sendData(response.toUtf8());
        disconnectFromHost();
        return;
    }

    // send stream features
    QXmppStreamFeatures features;
    if (socket() && !socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull()) {
        features.setTlsMode(QXmppStreamFeatures::Enabled);
    }
    if (!d->jid.isEmpty()) {
        features.setBindMode(QXmppStreamFeatures::Required);
        features.setSessionMode(QXmppStreamFeatures::Enabled);
    } else if (d->passwordChecker) {
        QStringList mechanisms;
        mechanisms << QStringLiteral("PLAIN");
        if (d->passwordChecker->hasGetPassword()) {
            mechanisms << QStringLiteral("DIGEST-MD5");
        }
        features.setAuthMechanisms(mechanisms);
    }
    sendPacket(features);
}

void QXmppIncomingClient::handleStanza(const QDomElement &nodeRecv)
{
    const QString ns = nodeRecv.namespaceURI();

    if (d->idleTimer->interval()) {
        d->idleTimer->start();
    }

    if (QXmppStartTlsPacket::isStartTlsPacket(nodeRecv, QXmppStartTlsPacket::StartTls)) {
        sendPacket(QXmppStartTlsPacket(QXmppStartTlsPacket::Proceed));
        socket()->flush();
        socket()->startServerEncryption();
        return;
    } else if (ns == ns_sasl) {
        if (!d->passwordChecker) {
            warning(QStringLiteral("Cannot perform authentication, no password checker"));
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
            disconnectFromHost();
            return;
        }

        if (auto auth = Sasl::Auth::fromDom(nodeRecv)) {
            d->saslServer = QXmppSaslServer::create(auth->mechanism, this);
            if (!d->saslServer) {
                sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::InvalidMechanism, QString() }));
                disconnectFromHost();
                return;
            }

            d->saslServer->setRealm(d->domain);

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(auth->value, challenge);

            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(auth->value);
            } else if (result == QXmppSaslServer::Challenge) {
                sendData(serializeXml(Sasl::Challenge { challenge }));
            } else {
                // FIXME: what condition?
                sendData(serializeXml(Sasl::Failure()));
                disconnectFromHost();
                return;
            }
        } else if (auto response = Sasl::Response::fromDom(nodeRecv)) {
            if (!d->saslServer) {
                warning(QStringLiteral("SASL response received, but no mechanism selected"));
                sendData(serializeXml(Sasl::Failure()));
                disconnectFromHost();
                return;
            }

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(response->value, challenge);
            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(response->value);
            } else if (result == QXmppSaslServer::Succeeded) {
                // authentication succeeded
                d->jid = QStringLiteral("%1@%2").arg(d->saslServer->username(), d->domain);
                info(QStringLiteral("Authentication succeeded for '%1' from %2").arg(d->jid, d->origin()));
                Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.success"));
                sendData(serializeXml(Sasl::Success()));
                handleStart();
            } else {
                // FIXME: what condition?
                sendData(serializeXml(Sasl::Failure()));
                disconnectFromHost();
            }
        }
    } else if (ns == ns_client) {
        if (nodeRecv.tagName() == QLatin1String("iq")) {
            const QString type = nodeRecv.attribute(QStringLiteral("type"));
            const auto id = nodeRecv.attribute(QStringLiteral("id"));

            if (QXmppBindIq::isBindIq(nodeRecv) && type == QLatin1String("set")) {
                QXmppBindIq bindSet;
                bindSet.parse(nodeRecv);
                d->resource = bindSet.resource().trimmed();
                if (d->resource.isEmpty()) {
                    d->resource = QXmppUtils::generateStanzaHash();
                }
                d->jid = QStringLiteral("%1/%2").arg(QXmppUtils::jidToBareJid(d->jid), d->resource);

                QXmppBindIq bindResult;
                bindResult.setType(QXmppIq::Result);
                bindResult.setId(bindSet.id());
                bindResult.setJid(d->jid);
                sendPacket(bindResult);

                // bound
                Q_EMIT connected();
                return;
            } else if (isIqType(nodeRecv, u"session", ns_session) && type == u"set") {
                QXmppIq sessionResult;
                sessionResult.setType(QXmppIq::Result);
                sessionResult.setId(id);
                sessionResult.setTo(d->jid);
                sendPacket(sessionResult);
                return;
            }
        }

        // check the sender is legitimate
        const QString from = nodeRecv.attribute(QStringLiteral("from"));
        if (!from.isEmpty() && from != d->jid && from != QXmppUtils::jidToBareJid(d->jid)) {
            warning(QStringLiteral("Received a stanza from unexpected JID %1").arg(from));
            return;
        }

        // process unhandled stanzas
        if (nodeRecv.tagName() == QLatin1String("iq") ||
            nodeRecv.tagName() == QLatin1String("message") ||
            nodeRecv.tagName() == QLatin1String("presence")) {
            QDomElement nodeFull(nodeRecv);

            // if the sender is empty, set it to the appropriate JID
            if (nodeFull.attribute(QStringLiteral("from")).isEmpty()) {
                if (nodeFull.tagName() == QLatin1String("presence") &&
                    (nodeFull.attribute(QStringLiteral("type")) == u"subscribe" ||
                     nodeFull.attribute(QStringLiteral("type")) == u"subscribed")) {
                    nodeFull.setAttribute(QStringLiteral("from"), QXmppUtils::jidToBareJid(d->jid));
                } else {
                    nodeFull.setAttribute(QStringLiteral("from"), d->jid);
                }
            }

            // if the recipient is empty, set it to the local domain
            if (nodeFull.attribute(QStringLiteral("to")).isEmpty()) {
                nodeFull.setAttribute(QStringLiteral("to"), d->domain);
            }

            // emit stanza for processing by server
            Q_EMIT elementReceived(nodeFull);
        }
    }
}
/// \endcond

void QXmppIncomingClient::onDigestReply()
{
    auto *reply = qobject_cast<QXmppPasswordReply *>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    if (reply->error() == QXmppPasswordReply::TemporaryError) {
        warning(QStringLiteral("Temporary authentication failure for '%1' from %2").arg(d->saslServer->username(), d->origin()));
        Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.temporary-auth-failure"));
        sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        disconnectFromHost();
        return;
    }

    QByteArray challenge;
    d->saslServer->setPasswordDigest(reply->digest());

    QXmppSaslServer::Response result = d->saslServer->respond(reply->property("__sasl_raw").toByteArray(), challenge);
    if (result != QXmppSaslServer::Challenge) {
        warning(QStringLiteral("Authentication failed for '%1' from %2").arg(d->saslServer->username(), d->origin()));
        Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.not-authorized"));
        sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        disconnectFromHost();
        return;
    }

    // send new challenge
    sendData(serializeXml(Sasl::Challenge { challenge }));
}

void QXmppIncomingClient::onPasswordReply()
{
    auto *reply = qobject_cast<QXmppPasswordReply *>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    const QString jid = QStringLiteral("%1@%2").arg(d->saslServer->username(), d->domain);
    switch (reply->error()) {
    case QXmppPasswordReply::NoError:
        d->jid = jid;
        info(QStringLiteral("Authentication succeeded for '%1' from %2").arg(d->jid, d->origin()));
        Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.success"));
        sendData(serializeXml(Sasl::Success()));
        handleStart();
        break;
    case QXmppPasswordReply::AuthorizationError:
        warning(QStringLiteral("Authentication failed for '%1' from %2").arg(jid, d->origin()));
        Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.not-authorized"));
        sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        disconnectFromHost();
        break;
    case QXmppPasswordReply::TemporaryError:
        warning(QStringLiteral("Temporary authentication failure for '%1' from %2").arg(jid, d->origin()));
        Q_EMIT updateCounter(QStringLiteral("incoming-client.auth.temporary-auth-failure"));
        sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        disconnectFromHost();
        break;
    }
}

void QXmppIncomingClient::onSocketDisconnected()
{
    info(QStringLiteral("Socket disconnected for '%1' from %2").arg(d->jid, d->origin()));
    Q_EMIT disconnected();
}

void QXmppIncomingClient::onTimeout()
{
    warning(QStringLiteral("Idle timeout for '%1' from %2").arg(d->jid, d->origin()));
    disconnectFromHost();

    // make sure disconnected() gets emitted no matter what
    QTimer::singleShot(30, this, &QXmppStream::disconnected);
}
