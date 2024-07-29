// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOUTGOINGCLIENT_P_H
#define QXMPPOUTGOINGCLIENT_P_H

#include "QXmppOutgoingClient.h"
#include "QXmppPromise.h"
#include "QXmppSaslManager_p.h"
#include "QXmppSasl_p.h"
#include "QXmppStreamError_p.h"
#include "QXmppStreamManagement_p.h"

#include "XmppSocket.h"

#include <QDnsLookup>
#include <QDomElement>

class QTimer;
class QXmppPacket;

// this leaks into other files, maybe better put QXmppOutgoingClientPrivate into QXmpp::Private
using namespace QXmpp::Private;

namespace QXmpp::Private {

using LegacyError = std::variant<QAbstractSocket::SocketError, QXmpp::TimeoutError, QXmppStanza::Error::Condition>;

// STARTTLS
class StarttlsManager
{
public:
    QXmppTask<void> task() { return m_promise.task(); }
    HandleElementResult handleElement(const QDomElement &el);

private:
    QXmppPromise<void> m_promise;
};

struct ProtocolError {
    QString text;
};

struct BoundAddress {
    QString user;
    QString domain;
    QString resource;
};

// Resource Binding
class BindManager
{
public:
    using Result = std::variant<BoundAddress, QXmppStanza::Error, ProtocolError>;

    explicit BindManager(SendDataInterface *socket) : m_socket(socket) { }

    QXmppTask<Result> bindAddress(const QString &resource);
    HandleElementResult handleElement(const QDomElement &el);

private:
    SendDataInterface *m_socket;
    QString m_iqId;
    std::optional<QXmppPromise<Result>> m_promise;
};

struct NonSaslAuthOptions {
    bool plain;
    bool digest;
};

// Authentication using Non-SASL auth
class NonSaslAuthManager
{
public:
    using OptionsResult = std::variant<NonSaslAuthOptions, QXmppError>;
    using AuthResult = std::variant<Success, QXmppError>;

    explicit NonSaslAuthManager(SendDataInterface *socket) : m_socket(socket) { }

    QXmppTask<OptionsResult> queryOptions(const QString &streamFrom, const QString &username);
    QXmppTask<AuthResult> authenticate(bool plainText, const QString &username, const QString &password, const QString &resource, const QString &streamId);
    HandleElementResult handleElement(const QDomElement &el);

private:
    struct NoQuery {
    };
    struct OptionsQuery {
        QXmppPromise<OptionsResult> p;
    };
    struct AuthQuery {
        QXmppPromise<AuthResult> p;
        QString id;
    };

    SendDataInterface *m_socket;
    std::variant<NoQuery, OptionsQuery, AuthQuery> m_query;
};

// XEP-0199: XMPP Ping
class PingManager
{
public:
    explicit PingManager(QXmppOutgoingClient *q);

    void onDataReceived();

private:
    void sendPing();

    QXmppOutgoingClient *q;
    QTimer *pingTimer;
    QTimer *timeoutTimer;
};

using IqResult = QXmppOutgoingClient::IqResult;

struct IqState {
    QXmppPromise<IqResult> interface;
    QString jid;
};

// Manager for creating tasks for outgoing IQ requests
class OutgoingIqManager
{
public:
    OutgoingIqManager(QXmppLoggable *l, StreamAckManager &streamAckMananger);
    ~OutgoingIqManager();

    QXmppTask<IqResult> sendIq(QXmppIq &&, const QString &to);
    QXmppTask<IqResult> sendIq(QXmppPacket &&, const QString &id, const QString &to);

    bool hasId(const QString &id) const;
    bool isIdValid(const QString &id) const;

    QXmppTask<IqResult> start(const QString &id, const QString &to);
    void finish(const QString &id, IqResult &&result);
    void cancelAll();

    void onSessionOpened(const SessionBegin &);
    void onSessionClosed(const SessionEnd &);
    bool handleStanza(const QDomElement &stanza);

private:
    void warning(const QString &message);

    QXmppLoggable *l;
    StreamAckManager &m_streamAckManager;
    std::unordered_map<QString, IqState> m_requests;
};

}  // namespace QXmpp::Private

class QXmppOutgoingClientPrivate
{
public:
    struct Error {
        QString text;
        QXmppOutgoingClient::ConnectionError details;
        LegacyError legacyError;
    };

    explicit QXmppOutgoingClientPrivate(QXmppOutgoingClient *q);
    void connectToHost(const ServerAddress &);
    void connectToAddressList(std::vector<ServerAddress> &&);
    void connectToNextAddress();

    // This object provides the configuration
    // required for connecting to the XMPP server.
    QXmppConfiguration config;
    std::optional<Error> error;

    // Core stream
    XmppSocket socket;
    StreamAckManager streamAckManager;
    OutgoingIqManager iqManager;

    // DNS
    std::vector<ServerAddress> serverAddresses;
    std::size_t nextServerAddressIndex = 0;
    enum {
        Current,
        TryNext,
    } nextAddressState = Current;

    // Stream
    QString streamId;
    QString streamFrom;
    QString streamVersion;

    // Redirection
    std::optional<StreamErrorElement::SeeOtherHost> redirect;

    // Authentication & Session
    bool isAuthenticated = false;
    bool bindModeAvailable = false;
    bool sessionStarted = false;
    AuthenticationMethod authenticationMethod = AuthenticationMethod::Sasl;
    std::optional<Bind2Bound> bind2Bound;

    std::variant<QXmppOutgoingClient *, StarttlsManager, NonSaslAuthManager, SaslManager, Sasl2Manager, C2sStreamManager *, BindManager> listener;
    FastTokenManager fastTokenManager;
    C2sStreamManager c2sStreamManager;
    CarbonManager carbonManager;
    CsiManager csiManager;
    PingManager pingManager;

    template<typename T, typename... Args>
    T &setListener(Args... args)
    {
        listener = T { args... };
        return std::get<T>(listener);
    }

private:
    QXmppOutgoingClient *q;
};

#endif  // QXMPPOUTGOINGCLIENT_P_H
