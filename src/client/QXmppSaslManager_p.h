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

    QXmppTask<AuthResult> authenticate(const QXmppConfiguration &config, const QXmppStreamFeatures &features, QXmppLoggable *parent);
    HandleElementResult handleElement(const QDomElement &el);

private:
    static AuthenticationError::Type mapSaslCondition(const std::optional<Sasl::ErrorCondition> &condition);

    SendDataInterface *m_socket;
    std::unique_ptr<QXmppSaslClient> m_saslClient;
    std::optional<QXmppPromise<AuthResult>> m_promise;
};

}  // namespace QXmpp::Private

#endif  // QXMPPSASLMANAGER_P_H
