// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConfiguration.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppSaslManager_p.h"
#include "QXmppStream.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils_p.h"

#include "XmppSocket.h"

#include <QDomElement>

#ifndef QXMPP_DOC

namespace QXmpp::Private {

static QString chooseMechanism(const QXmppConfiguration &config, const QList<QString> &availableMechanisms)
{
    // supported and preferred SASL auth mechanisms
    const QString preferredMechanism = config.saslAuthMechanism();
    QStringList supportedMechanisms = QXmppSaslClient::availableMechanisms();
    if (supportedMechanisms.contains(preferredMechanism)) {
        supportedMechanisms.removeAll(preferredMechanism);
        supportedMechanisms.prepend(preferredMechanism);
    }
    if (config.facebookAppId().isEmpty() || config.facebookAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(QStringLiteral("X-FACEBOOK-PLATFORM"));
    }
    if (config.windowsLiveAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(QStringLiteral("X-MESSENGER-OAUTH2"));
    }
    if (config.googleAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(QStringLiteral("X-OAUTH2"));
    }

    // determine SASL Authentication mechanism to use
    QStringList commonMechanisms;
    for (const auto &mechanism : std::as_const(supportedMechanisms)) {
        if (availableMechanisms.contains(mechanism)) {
            commonMechanisms << mechanism;
        }
    }
    return commonMechanisms.empty() ? QString() : commonMechanisms.first();
}

static void setCredentials(QXmppSaslClient *saslClient, const QXmppConfiguration &config)
{
    auto mechanism = saslClient->mechanism();
    if (mechanism == u"X-FACEBOOK-PLATFORM") {
        saslClient->setUsername(config.facebookAppId());
        saslClient->setPassword(config.facebookAccessToken());
    } else if (mechanism == u"X-MESSENGER-OAUTH2") {
        saslClient->setPassword(config.windowsLiveAccessToken());
    } else if (mechanism == u"X-OAUTH2") {
        saslClient->setUsername(config.user());
        saslClient->setPassword(config.googleAccessToken());
    } else {
        saslClient->setUsername(config.user());
        saslClient->setPassword(config.password());
    }
}

static AuthenticationError::Type mapSaslCondition(std::optional<Sasl::ErrorCondition> condition)
{
    using Auth = AuthenticationError;
    using Sasl = Sasl::ErrorCondition;

    switch (condition.value_or(Sasl::NotAuthorized)) {
    case Sasl::AccountDisabled:
        return Auth::AccountDisabled;
    case Sasl::CredentialsExpired:
        return Auth::CredentialsExpired;
    case Sasl::EncryptionRequired:
        return Auth::EncryptionRequired;
    case Sasl::IncorrectEncoding:
    case Sasl::InvalidAuthzid:
    case Sasl::InvalidMechanism:
    case Sasl::MalformedRequest:
    case Sasl::MechanismTooWeak:
        return Auth::ProcessingError;
    case Sasl::Aborted:
    case Sasl::NotAuthorized:
    case Sasl::TemporaryAuthFailure:
        return Auth::NotAuthorized;
    }
    return Auth::NotAuthorized;
}

QXmppTask<SaslManager::AuthResult> SaslManager::authenticate(const QXmppConfiguration &config, const QXmppStreamFeatures &features, QXmppLoggable *parent)
{
    Q_ASSERT(!m_promise.has_value());

    auto mechanism = chooseMechanism(config, features.authMechanisms());
    if (mechanism.isEmpty()) {
        return makeReadyTask<AuthResult>(AuthError {
            QStringLiteral("No supported SASL Authentication mechanism available"),
            AuthenticationError { AuthenticationError::MechanismMismatch, {}, {} },
        });
    }

    m_saslClient.reset(QXmppSaslClient::create(mechanism, parent));
    if (!m_saslClient) {
        return makeReadyTask<AuthResult>(AuthError {
            QStringLiteral("SASL mechanism negotiation failed"),
            AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
        });
    }
    m_saslClient->info(QStringLiteral("SASL mechanism '%1' selected").arg(m_saslClient->mechanism()));
    m_saslClient->setHost(config.domain());
    m_saslClient->setServiceType(QStringLiteral("xmpp"));
    setCredentials(m_saslClient.get(), config);

    // send SASL auth request
    QByteArray response;
    if (!m_saslClient->respond(QByteArray(), response)) {
        return makeReadyTask<AuthResult>(AuthError {
            QStringLiteral("SASL initial response failed"),
            AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
        });
    }
    m_socket->sendData(serializeXml(Sasl::Auth { m_saslClient->mechanism(), response }));

    m_promise = QXmppPromise<AuthResult>();
    return m_promise->task();
}

HandleElementResult SaslManager::handleElement(const QDomElement &el)
{
    auto finish = [this](auto &&value) {
        auto p = std::move(*m_promise);
        m_promise.reset();
        p.finish(value);
    };

    if (!m_promise.has_value() || el.namespaceURI() != ns_sasl) {
        return Rejected;
    }

    if (el.tagName() == u"success") {
        finish(Success());
        return Finished;
    } else if (auto challenge = Sasl::Challenge::fromDom(el)) {
        QByteArray response;
        if (m_saslClient->respond(challenge->value, response)) {
            m_socket->sendData(serializeXml(Sasl::Response { response }));
            return Accepted;
        } else {
            finish(AuthError {
                QStringLiteral("Could not respond to SASL challenge"),
                AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
            });
            return Finished;
        }
    } else if (auto failure = Sasl::Failure::fromDom(el)) {
        auto text = failure->text.isEmpty()
            ? Sasl::errorConditionToString(failure->condition.value_or(Sasl::ErrorCondition::NotAuthorized))
            : failure->text;

        finish(AuthError {
            QStringLiteral("Authentication failed: %1").arg(text),
            AuthenticationError { mapSaslCondition(failure->condition), failure->text, std::move(*failure) },
        });
        return Finished;
    }
    return Rejected;
}

}  // namespace QXmpp::Private

#endif  // QXMPP_DOC
