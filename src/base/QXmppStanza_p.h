// SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTANZA_P_H
#define QXMPPSTANZA_P_H

#include "QXmppStanza.h"

#include <optional>

//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API. It exists for the convenience
// of the QXmppStanza class.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

static QString conditionToString(QXmppStanza::Error::Condition condition)
{
    switch (condition) {
    case QXmppStanza::Error::BadRequest:
        return "bad-request";
    case QXmppStanza::Error::Conflict:
        return "conflict";
    case QXmppStanza::Error::FeatureNotImplemented:
        return "feature-not-implemented";
    case QXmppStanza::Error::Forbidden:
        return "forbidden";
    case QXmppStanza::Error::Gone:
        return "gone";
    case QXmppStanza::Error::InternalServerError:
        return "internal-server-error";
    case QXmppStanza::Error::ItemNotFound:
        return "item-not-found";
    case QXmppStanza::Error::JidMalformed:
        return "jid-malformed";
    case QXmppStanza::Error::NotAcceptable:
        return "not-acceptable";
    case QXmppStanza::Error::NotAllowed:
        return "not-allowed";
    case QXmppStanza::Error::NotAuthorized:
        return "not-authorized";
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
    case QXmppStanza::Error::PaymentRequired:
        QT_WARNING_POP
        return "payment-required";
    case QXmppStanza::Error::PolicyViolation:
        return "policy-violation";
    case QXmppStanza::Error::RecipientUnavailable:
        return "recipient-unavailable";
    case QXmppStanza::Error::Redirect:
        return "redirect";
    case QXmppStanza::Error::RegistrationRequired:
        return "registration-required";
    case QXmppStanza::Error::RemoteServerNotFound:
        return "remote-server-not-found";
    case QXmppStanza::Error::RemoteServerTimeout:
        return "remote-server-timeout";
    case QXmppStanza::Error::ResourceConstraint:
        return "resource-constraint";
    case QXmppStanza::Error::ServiceUnavailable:
        return "service-unavailable";
    case QXmppStanza::Error::SubscriptionRequired:
        return "subscription-required";
    case QXmppStanza::Error::UndefinedCondition:
        return "undefined-condition";
    case QXmppStanza::Error::UnexpectedRequest:
        return "unexpected-request";
    }
    return {};
}

static std::optional<QXmppStanza::Error::Condition> conditionFromString(const QString &string)
{
    if (string == "bad-request")
        return QXmppStanza::Error::BadRequest;
    else if (string == "conflict")
        return QXmppStanza::Error::Conflict;
    else if (string == "feature-not-implemented")
        return QXmppStanza::Error::FeatureNotImplemented;
    else if (string == "forbidden")
        return QXmppStanza::Error::Forbidden;
    else if (string == "gone")
        return QXmppStanza::Error::Gone;
    else if (string == "internal-server-error")
        return QXmppStanza::Error::InternalServerError;
    else if (string == "item-not-found")
        return QXmppStanza::Error::ItemNotFound;
    else if (string == "jid-malformed")
        return QXmppStanza::Error::JidMalformed;
    else if (string == "not-acceptable")
        return QXmppStanza::Error::NotAcceptable;
    else if (string == "not-allowed")
        return QXmppStanza::Error::NotAllowed;
    else if (string == "not-authorized")
        return QXmppStanza::Error::NotAuthorized;
    else if (string == "payment-required") {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        return QXmppStanza::Error::PaymentRequired;
        QT_WARNING_POP
    } else if (string == "policy-violation")
        return QXmppStanza::Error::PolicyViolation;
    else if (string == "recipient-unavailable")
        return QXmppStanza::Error::RecipientUnavailable;
    else if (string == "redirect")
        return QXmppStanza::Error::Redirect;
    else if (string == "registration-required")
        return QXmppStanza::Error::RegistrationRequired;
    else if (string == "remote-server-not-found")
        return QXmppStanza::Error::RemoteServerNotFound;
    else if (string == "remote-server-timeout")
        return QXmppStanza::Error::RemoteServerTimeout;
    else if (string == "resource-constraint")
        return QXmppStanza::Error::ResourceConstraint;
    else if (string == "service-unavailable")
        return QXmppStanza::Error::ServiceUnavailable;
    else if (string == "subscription-required")
        return QXmppStanza::Error::SubscriptionRequired;
    else if (string == "undefined-condition")
        return QXmppStanza::Error::UndefinedCondition;
    else if (string == "unexpected-request")
        return QXmppStanza::Error::UnexpectedRequest;
    return std::nullopt;
}

static QString typeToString(QXmppStanza::Error::Type type)
{
    switch (type) {
    case QXmppStanza::Error::Cancel:
        return QStringLiteral("cancel");
    case QXmppStanza::Error::Continue:
        return QStringLiteral("continue");
    case QXmppStanza::Error::Modify:
        return QStringLiteral("modify");
    case QXmppStanza::Error::Auth:
        return QStringLiteral("auth");
    case QXmppStanza::Error::Wait:
        return QStringLiteral("wait");
    }
    return {};
}

static std::optional<QXmppStanza::Error::Type> typeFromString(const QString &string)
{
    if (string == QStringLiteral("cancel"))
        return QXmppStanza::Error::Cancel;
    else if (string == QStringLiteral("continue"))
        return QXmppStanza::Error::Continue;
    else if (string == QStringLiteral("modify"))
        return QXmppStanza::Error::Modify;
    else if (string == QStringLiteral("auth"))
        return QXmppStanza::Error::Auth;
    else if (string == QStringLiteral("wait"))
        return QXmppStanza::Error::Wait;
    return std::nullopt;
}

#endif
