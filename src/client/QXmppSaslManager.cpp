// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConfiguration.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppSasl2UserAgent.h"
#include "QXmppSaslManager_p.h"
#include "QXmppSasl_p.h"
#include "QXmppStream.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"
#include "XmppSocket.h"

#include <QDomElement>

namespace QXmpp::Private {

static std::tuple<QString, QStringList> chooseMechanism(const QXmppConfiguration &config, const QList<QString> &availableMechanisms)
{
    // supported and preferred SASL auth mechanisms
    const QString preferredMechanism = config.saslAuthMechanism();
    QStringList supportedMechanisms = QXmppSaslClient::availableMechanisms();
    if (supportedMechanisms.contains(preferredMechanism)) {
        supportedMechanisms.removeAll(preferredMechanism);
        supportedMechanisms.prepend(preferredMechanism);
    }
    if (config.facebookAppId().isEmpty() || config.facebookAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(u"X-FACEBOOK-PLATFORM"_s);
    }
    if (config.windowsLiveAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(u"X-MESSENGER-OAUTH2"_s);
    }
    if (config.googleAccessToken().isEmpty()) {
        supportedMechanisms.removeAll(u"X-OAUTH2"_s);
    }

    // determine SASL Authentication mechanism to use
    QStringList commonMechanisms;
    for (const auto &mechanism : std::as_const(supportedMechanisms)) {
        if (availableMechanisms.contains(mechanism)) {
            commonMechanisms << mechanism;
        }
    }

    // Remove disabled mechanisms and add to disabledAvailable
    const auto disabled = config.disabledSaslMechanisms();
    QStringList disabledAvailable;
    for (const auto &m : disabled) {
        if (commonMechanisms.removeAll(m)) {
            disabledAvailable.push_back(m);
        }
    }

    return { commonMechanisms.empty() ? QString() : commonMechanisms.first(), disabledAvailable };
}

struct InitSaslAuthResult {
    std::unique_ptr<QXmppSaslClient> saslClient;
    std::optional<SaslManager::AuthError> error;
    QByteArray initialResponse;
};

static InitSaslAuthResult error(QString text, AuthenticationError err)
{
    return { {}, SaslManager::AuthError { text, err }, {} };
}

static InitSaslAuthResult initSaslAuthentication(const QXmppConfiguration &config, const QList<QString> &availableMechanisms, QXmppLoggable *parent)
{
    auto info = [&](const auto &message) {
        Q_EMIT parent->logMessage(QXmppLogger::InformationMessage, message);
    };

    auto [mechanism, disabled] = chooseMechanism(config, availableMechanisms);
    if (mechanism.isEmpty()) {
        auto text = disabled.empty()
            ? u"No supported SASL mechanism available"_s
            : u"No supported SASL mechanism available (%1 is disabled)"_s.arg(disabled.join(u", "));

        return error(std::move(text), { AuthenticationError::MechanismMismatch, {}, {} });
    }

    auto saslClient = QXmppSaslClient::create(mechanism, parent);
    if (!saslClient) {
        return error(u"SASL mechanism negotiation failed"_s,
                     AuthenticationError { AuthenticationError::ProcessingError, {}, {} });
    }
    info(u"SASL mechanism '%1' selected"_s.arg(saslClient->mechanism().toString()));
    saslClient->setHost(config.domain());
    saslClient->setServiceType(u"xmpp"_s);
    saslClient->setUsername(config.user());
    saslClient->setCredentials(config.credentialData());

    // send SASL auth request
    if (auto response = saslClient->respond(QByteArray())) {
        return { std::move(saslClient), {}, *response };
    } else {
        return error(u"SASL initial response failed"_s,
                     AuthenticationError { AuthenticationError::ProcessingError, {}, {} });
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

QXmppTask<SaslManager::AuthResult> SaslManager::authenticate(const QXmppConfiguration &config, const QList<QString> &availableMechanisms, QXmppLoggable *parent)
{
    Q_ASSERT(!m_promise.has_value());

    auto result = initSaslAuthentication(config, availableMechanisms, parent);
    if (result.error) {
        return makeReadyTask<AuthResult>(std::move(*result.error));
    }

    m_socket->sendData(serializeXml(Sasl::Auth { result.saslClient->mechanism().toString(), result.initialResponse }));

    m_promise = QXmppPromise<AuthResult>();
    m_saslClient = std::move(result.saslClient);
    return m_promise->task();
}

HandleElementResult SaslManager::handleElement(const QDomElement &el)
{
    using namespace Sasl;

    auto finish = [this](auto &&value) {
        auto p = std::move(*m_promise);
        m_promise.reset();
        p.finish(value);
    };

    if (!m_promise.has_value()) {
        return Rejected;
    }

    if (Success::fromDom(el)) {
        finish(QXmpp::Success());
        return Finished;
    } else if (auto challenge = Challenge::fromDom(el)) {
        if (auto response = m_saslClient->respond(challenge->value)) {
            m_socket->sendData(serializeXml(Response { *response }));
            return Accepted;
        } else {
            finish(AuthError {
                u"Could not respond to SASL challenge"_s,
                AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
            });
            return Finished;
        }
    } else if (auto failure = Failure::fromDom(el)) {
        auto text = failure->text.isEmpty()
            ? errorConditionToString(failure->condition.value_or(ErrorCondition::NotAuthorized))
            : failure->text;

        finish(AuthError {
            u"Authentication failed: %1"_s.arg(text),
            AuthenticationError { mapSaslCondition(failure->condition), failure->text, std::move(*failure) },
        });
        return Finished;
    }
    return Rejected;
}

QXmppTask<Sasl2Manager::AuthResult> Sasl2Manager::authenticate(Sasl2::Authenticate &&auth, const QXmppConfiguration &config, const Sasl2::StreamFeature &feature, QXmppLoggable *loggable)
{
    Q_ASSERT(!m_state.has_value());

    auto result = initSaslAuthentication(config, feature.mechanisms, loggable);
    if (result.error) {
        return makeReadyTask<AuthResult>(std::move(*result.error));
    }

    // create request
    auth.mechanism = result.saslClient->mechanism().toString();
    auth.initialResponse = result.initialResponse;

    // set user-agent if enabled
    if (auto userAgent = config.sasl2UserAgent()) {
        // ID is mandatory
        if (userAgent->deviceId().isNull()) {
            return makeReadyTask<AuthResult>(AuthError {
                u"Invalid user-agent: device ID must be set."_s,
                AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
            });
        }
        auth.userAgent = { userAgent->deviceId(), userAgent->softwareName(), userAgent->deviceName() };
    }

    // send request
    m_socket->sendData(serializeXml(auth));

    m_state = State();
    m_state->sasl = std::move(result.saslClient);
    return m_state->p.task();
}

HandleElementResult Sasl2Manager::handleElement(const QDomElement &el)
{
    using namespace Sasl2;

    auto finish = [this](AuthResult &&value) {
        Q_ASSERT(m_state);
        auto state = std::move(*m_state);
        m_state.reset();
        state.p.finish(value);
    };

    if (!m_state) {
        return Rejected;
    }

    if (auto challenge = Challenge::fromDom(el)) {
        if (auto response = m_state->sasl->respond(challenge->data)) {
            m_socket->sendData(serializeXml(Response { *response }));
            return Accepted;
        } else {
            finish(AuthError {
                u"Could not respond to SASL challenge"_s,
                AuthenticationError { AuthenticationError::ProcessingError, {}, {} },
            });
            return Finished;
        }
    } else if (auto success = Success::fromDom(el)) {
        finish(std::move(*success));
        return Finished;
    } else if (auto failure = Failure::fromDom(el)) {
        auto text = failure->text.isEmpty()
            ? Sasl::errorConditionToString(failure->condition)
            : failure->text;

        if (failure->condition == Sasl::ErrorCondition::Aborted && m_state->unsupportedContinue) {
            finish(AuthError {
                u"Required authentication tasks not supported."_s,
                AuthenticationError {
                    AuthenticationError::RequiredTasks,
                    m_state->unsupportedContinue->text,
                    *m_state->unsupportedContinue,
                },
            });
        } else {
            finish(AuthError {
                u"Authentication failed: %1"_s.arg(text),
                AuthenticationError { mapSaslCondition(failure->condition), failure->text, std::move(*failure) },
            });
        }
        return Finished;
    } else if (auto continueElement = Continue::fromDom(el)) {
        // no SASL 2 tasks are currently implemented
        m_state->unsupportedContinue = continueElement;
        m_socket->sendData(serializeXml(Abort { u"SASL 2 tasks are not supported."_s }));
        return Accepted;
    }
    return Rejected;
}

}  // namespace QXmpp::Private
