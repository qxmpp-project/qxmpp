// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSASLMANAGER_P_H
#define QXMPPSASLMANAGER_P_H

#include "QXmppAuthenticationError.h"
#include "QXmppOutgoingClient.h"
#include "QXmppPromise.h"
#include "QXmppSasl_p.h"
#include "QXmppTask.h"

#include <optional>

class QXmppConfiguration;
class QXmppStreamFeatures;

namespace QXmpp::Private {

class SendDataInterface;

// Authentication using SASL
class SaslManager
{
public:
    using AuthError = std::pair<QString, AuthenticationError>;
    using AuthResult = std::variant<Success, AuthError>;

    explicit SaslManager(SendDataInterface *socket) : m_socket(socket) { }

    QXmppTask<AuthResult> authenticate(const QXmppConfiguration &config, const QList<QString> &availableMechanisms, QXmppLoggable *parent);
    HandleElementResult handleElement(const QDomElement &el);

private:
    SendDataInterface *m_socket;
    std::unique_ptr<QXmppSaslClient> m_saslClient;
    std::optional<QXmppPromise<AuthResult>> m_promise;
};

// Authentication using SASL 2
class Sasl2Manager
{
public:
    using AuthError = std::pair<QString, AuthenticationError>;
    using AuthResult = std::variant<Sasl2::Success, AuthError>;

    explicit Sasl2Manager(SendDataInterface *socket) : m_socket(socket) { }

    QXmppTask<AuthResult> authenticate(Sasl2::Authenticate &&authenticate, const QXmppConfiguration &config, const Sasl2::StreamFeature &feature, QXmppLoggable *loggable);
    HandleElementResult handleElement(const QDomElement &);

private:
    struct State {
        std::unique_ptr<QXmppSaslClient> sasl;
        QXmppPromise<AuthResult> p;
        std::optional<Sasl2::Continue> unsupportedContinue;
    };

    SendDataInterface *m_socket;
    std::optional<State> m_state;
};

}  // namespace QXmpp::Private

#endif  // QXMPPSASLMANAGER_P_H
