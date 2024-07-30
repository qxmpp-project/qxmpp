// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppIncomingClient.h"

#include "QXmppBindIq.h"
#include "QXmppConstants_p.h"
#include "QXmppPasswordChecker.h"
#include "QXmppSasl_p.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Stream.h"
#include "StringLiterals.h"

#include <QDomElement>
#include <QHostAddress>
#include <QSslKey>
#include <QSslSocket>
#include <QTimer>

using namespace QXmpp::Private;

constexpr uint RESOURCE_RANDOM_SUFFIX_LENGTH = 8;

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
    enum {
        Sasl,
        Sasl2
    } saslVersion = Sasl;
    std::optional<Sasl2::Authenticate> sasl2AuthRequest;

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
        return socket->peerAddress().toString() + u' ' + QString::number(socket->peerPort());
    } else {
        return u"<unknown>"_s;
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

    info(u"Incoming client connection from %1"_s.arg(d->origin()));

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
    QString response = u"<?xml version='1.0'?><stream:stream xmlns=\"%1\" "
                       "xmlns:stream=\"%2\" id=\"%3\" from=\"%4\" "
                       "version=\"1.0\" xml:lang=\"en\">"_s
                           .arg(
                               ns_client,
                               ns_stream,
                               QString::fromUtf8(sessionId),
                               d->domain);
    sendData(response.toUtf8());

    // check requested domain
    if (streamElement.attribute(u"to"_s) != d->domain) {
        QString response = u"<stream:error>"
                           "<host-unknown xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\"/>"
                           "<text xmlns=\"urn:ietf:params:xml:ns:xmpp-streams\">"
                           "This server does not serve %1"
                           "</text>"
                           "</stream:error>"_s
                               .arg(streamElement.attribute(u"to"_s));
        sendData(response.toUtf8());
        disconnectFromHost();
        return;
    }
    sendStreamFeatures();
}

void QXmppIncomingClient::sendStreamFeatures()
{
    // send stream features
    QXmppStreamFeatures features;
    if (socket() && !socket()->isEncrypted() && !socket()->localCertificate().isNull() && !socket()->privateKey().isNull()) {
        features.setTlsMode(QXmppStreamFeatures::Enabled);
    }
    if (!d->jid.isEmpty()) {
        if (d->resource.isEmpty()) {
            features.setBindMode(QXmppStreamFeatures::Required);
        }
        features.setSessionMode(QXmppStreamFeatures::Enabled);
    } else if (d->passwordChecker) {
        QStringList mechanisms;
        mechanisms << u"PLAIN"_s;
        if (d->passwordChecker->hasGetPassword()) {
            mechanisms << u"DIGEST-MD5"_s;
        }
        features.setAuthMechanisms(mechanisms);
        features.setSasl2Feature(Sasl2::StreamFeature {
            mechanisms,
            d->resource.isEmpty() ? Bind2Feature {} : std::optional<Bind2Feature>(),
            {},
            false,
        });
    }
    sendPacket(features);
}

void QXmppIncomingClient::handleStanza(const QDomElement &nodeRecv)
{
    const QString ns = nodeRecv.namespaceURI();

    if (d->idleTimer->interval()) {
        d->idleTimer->start();
    }

    if (StarttlsRequest::fromDom(nodeRecv)) {
        sendData(serializeXml(StarttlsProceed()));
        socket()->flush();
        socket()->startServerEncryption();
        return;
    } else if (ns == ns_sasl_2) {
        if (!d->passwordChecker) {
            warning(u"Cannot perform authentication, no password checker"_s);
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, {} }));
            disconnectFromHost();
            return;
        }

        if (auto auth = Sasl2::Authenticate::fromDom(nodeRecv)) {
            d->saslVersion = QXmppIncomingClientPrivate::Sasl2;
            d->sasl2AuthRequest = std::move(auth);
            d->saslServer = QXmppSaslServer::create(d->sasl2AuthRequest->mechanism, this);
            if (!d->saslServer) {
                sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::InvalidMechanism, QString() }));
                disconnectFromHost();
                return;
            }

            d->saslServer->setRealm(d->domain);

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(d->sasl2AuthRequest->initialResponse, challenge);

            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(d->sasl2AuthRequest->initialResponse);
            } else if (result == QXmppSaslServer::Challenge) {
                sendData(serializeXml(Sasl2::Challenge { challenge }));
            } else {
                d->sasl2AuthRequest.reset();
                sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::NotAuthorized, {} }));
                disconnectFromHost();
                return;
            }
        } else if (auto response = Sasl2::Response::fromDom(nodeRecv)) {
            if (!d->saslServer) {
                warning(u"SASL response received, but no mechanism selected"_s);
                sendData(serializeXml(Sasl2::Failure()));
                disconnectFromHost();
                return;
            }

            QByteArray challenge;
            QXmppSaslServer::Response result = d->saslServer->respond(response->data, challenge);
            if (result == QXmppSaslServer::InputNeeded) {
                // check credentials
                d->checkCredentials(response->data);
            } else if (result == QXmppSaslServer::Succeeded) {
                // authentication succeeded
                d->jid = u"%1@%2"_s.arg(d->saslServer->username(), d->domain);
                info(u"Authentication succeeded for '%1' from %2"_s.arg(d->jid, d->origin()));
                Q_EMIT updateCounter(u"incoming-client.auth.success"_s);
                onSasl2Authenticated();
            } else {
                d->sasl2AuthRequest.reset();
                sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::NotAuthorized, {} }));
                disconnectFromHost();
            }
        } else if (auto abort = Sasl2::Abort::fromDom(nodeRecv)) {
            d->sasl2AuthRequest.reset();
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::Aborted, {} }));
        }
    } else if (ns == ns_sasl) {
        if (!d->passwordChecker) {
            warning(u"Cannot perform authentication, no password checker"_s);
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
            disconnectFromHost();
            return;
        }

        if (auto auth = Sasl::Auth::fromDom(nodeRecv)) {
            d->saslVersion = QXmppIncomingClientPrivate::Sasl;
            d->sasl2AuthRequest.reset();
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
                warning(u"SASL response received, but no mechanism selected"_s);
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
                d->jid = u"%1@%2"_s.arg(d->saslServer->username(), d->domain);
                info(u"Authentication succeeded for '%1' from %2"_s.arg(d->jid, d->origin()));
                Q_EMIT updateCounter(u"incoming-client.auth.success"_s);
                sendData(serializeXml(Sasl::Success()));
                handleStart();
            } else {
                // FIXME: what condition?
                sendData(serializeXml(Sasl::Failure()));
                disconnectFromHost();
            }
        }
    } else if (ns == ns_client) {
        if (nodeRecv.tagName() == u"iq") {
            const QString type = nodeRecv.attribute(u"type"_s);
            const auto id = nodeRecv.attribute(u"id"_s);

            if (QXmppBindIq::isBindIq(nodeRecv) && type == u"set") {
                QXmppBindIq bindSet;
                bindSet.parse(nodeRecv);
                d->resource = bindSet.resource().trimmed();
                if (d->resource.isEmpty()) {
                    d->resource = QXmppUtils::generateStanzaHash();
                }
                d->jid = u"%1/%2"_s.arg(QXmppUtils::jidToBareJid(d->jid), d->resource);

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
        const QString from = nodeRecv.attribute(u"from"_s);
        if (!from.isEmpty() && from != d->jid && from != QXmppUtils::jidToBareJid(d->jid)) {
            warning(u"Received a stanza from unexpected JID %1"_s.arg(from));
            return;
        }

        // process unhandled stanzas
        auto tagName = nodeRecv.tagName();
        if (tagName == u"iq" || tagName == u"message" || tagName == u"presence") {
            QDomElement nodeFull(nodeRecv);

            // if the sender is empty, set it to the appropriate JID
            if (nodeFull.attribute(u"from"_s).isEmpty()) {
                if (nodeFull.tagName() == u"presence" &&
                    (nodeFull.attribute(u"type"_s) == u"subscribe" ||
                     nodeFull.attribute(u"type"_s) == u"subscribed")) {
                    nodeFull.setAttribute(u"from"_s, QXmppUtils::jidToBareJid(d->jid));
                } else {
                    nodeFull.setAttribute(u"from"_s, d->jid);
                }
            }

            // if the recipient is empty, set it to the local domain
            if (nodeFull.attribute(u"to"_s).isEmpty()) {
                nodeFull.setAttribute(u"to"_s, d->domain);
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
        warning(u"Temporary authentication failure for '%1' from %2"_s.arg(d->saslServer->username(), d->origin()));
        Q_EMIT updateCounter(u"incoming-client.auth.temporary-auth-failure"_s);
        if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        } else {
            d->sasl2AuthRequest.reset();
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        }
        disconnectFromHost();
        return;
    }

    QByteArray challenge;
    d->saslServer->setPasswordDigest(reply->digest());

    QXmppSaslServer::Response result = d->saslServer->respond(reply->property("__sasl_raw").toByteArray(), challenge);
    if (result != QXmppSaslServer::Challenge) {
        warning(u"Authentication failed for '%1' from %2"_s.arg(d->saslServer->username(), d->origin()));
        Q_EMIT updateCounter(u"incoming-client.auth.not-authorized"_s);
        if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        } else {
            d->sasl2AuthRequest.reset();
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        }
        disconnectFromHost();
        return;
    }

    // send new challenge
    if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
        sendData(serializeXml(Sasl::Challenge { challenge }));
    } else {
        sendData(serializeXml(Sasl2::Challenge { challenge }));
    }
}

void QXmppIncomingClient::onPasswordReply()
{
    auto *reply = qobject_cast<QXmppPasswordReply *>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    const QString jid = u"%1@%2"_s.arg(d->saslServer->username(), d->domain);
    switch (reply->error()) {
    case QXmppPasswordReply::NoError:
        d->jid = jid;
        info(u"Authentication succeeded for '%1' from %2"_s.arg(d->jid, d->origin()));
        Q_EMIT updateCounter(u"incoming-client.auth.success"_s);
        if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
            sendData(serializeXml(Sasl::Success {}));
            handleStart();
        } else {
            onSasl2Authenticated();
        }
        break;
    case QXmppPasswordReply::AuthorizationError:
        warning(u"Authentication failed for '%1' from %2"_s.arg(jid, d->origin()));
        Q_EMIT updateCounter(u"incoming-client.auth.not-authorized"_s);
        if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        } else {
            d->sasl2AuthRequest.reset();
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::NotAuthorized, QString() }));
        }
        disconnectFromHost();
        break;
    case QXmppPasswordReply::TemporaryError:
        warning(u"Temporary authentication failure for '%1' from %2"_s.arg(jid, d->origin()));
        Q_EMIT updateCounter(u"incoming-client.auth.temporary-auth-failure"_s);
        if (d->saslVersion == QXmppIncomingClientPrivate::Sasl) {
            sendData(serializeXml(Sasl::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        } else {
            d->sasl2AuthRequest.reset();
            sendData(serializeXml(Sasl2::Failure { Sasl::ErrorCondition::TemporaryAuthFailure, QString() }));
        }
        disconnectFromHost();
        break;
    }
}

void QXmppIncomingClient::onSocketDisconnected()
{
    info(u"Socket disconnected for '%1' from %2"_s.arg(d->jid, d->origin()));
    Q_EMIT disconnected();
}

void QXmppIncomingClient::onTimeout()
{
    warning(u"Idle timeout for '%1' from %2"_s.arg(d->jid, d->origin()));
    disconnectFromHost();

    // make sure disconnected() gets emitted no matter what
    QTimer::singleShot(30, this, &QXmppStream::disconnected);
}

void QXmppIncomingClient::onSasl2Authenticated()
{
    Q_ASSERT(d->sasl2AuthRequest);

    if (d->sasl2AuthRequest->bindRequest) {
        // resource binding
        const auto &tag = d->sasl2AuthRequest->bindRequest->tag;
        if (tag.isEmpty()) {
            d->resource = QXmppUtils::generateStanzaHash(RESOURCE_RANDOM_SUFFIX_LENGTH);
        } else {
            d->resource = tag + u'.' + QXmppUtils::generateStanzaHash(RESOURCE_RANDOM_SUFFIX_LENGTH);
        }
        d->jid = u"%1/%2"_s.arg(QXmppUtils::jidToBareJid(d->jid), d->resource);

        sendData(serializeXml(Sasl2::Success { {}, d->jid, Bind2Bound {} }));

        // resource is bound now
        Q_EMIT connected();
    } else {
        sendData(serializeXml(Sasl2::Success { {}, d->jid, {} }));
    }
    // clean up
    d->sasl2AuthRequest.reset();

    sendStreamFeatures();
    handleStart();
}
