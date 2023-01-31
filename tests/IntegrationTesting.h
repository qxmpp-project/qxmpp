// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef INTEGRATIONTESTING_H
#define INTEGRATIONTESTING_H

#include "QXmppConfiguration.h"

#include <QDebug>
#include <QtGlobal>

#define ENV_ENABLED "QXMPP_TESTS_INTEGRATION_ENABLED"
#define ENV_JID "QXMPP_TESTS_JID"
#define ENV_PASSWORD "QXMPP_TESTS_PASSWORD"

#define SKIP_IF_INTEGRATION_TESTS_DISABLED()                            \
    if (!IntegrationTests::enabled()) {                                 \
        QSKIP("Export 'QXMPP_TESTS_INTEGRATION_ENABLED=1' to enable."); \
    } else if (!IntegrationTests::credentialsAvailable()) {             \
        QFAIL("No credentials for integration tests provided! "         \
              "Export 'QXMPP_TESTS_JID' and 'QXMPP_TESTS_PASSWORD'.");  \
    }

class IntegrationTests
{
public:
    static QString environmentVariable(const char *varName, const QString &defaultValue = {})
    {
        return qEnvironmentVariable(varName, defaultValue);
    }

    static bool enabled()
    {
        return environmentVariable(ENV_ENABLED, "0") == "1";
    }

    static bool credentialsAvailable()
    {
        return !qEnvironmentVariableIsEmpty(ENV_JID) && !qEnvironmentVariableIsEmpty(ENV_PASSWORD);
    }

    static QXmppConfiguration clientConfiguration()
    {
        QXmppConfiguration config;
        config.setJid(environmentVariable(ENV_JID));
        config.setPassword(environmentVariable(ENV_PASSWORD));
        return config;
    }
};

#undef ENV_ENABLED
#undef ENV_JID
#undef ENV_PASSWORD

#endif  // INTEGRATIONTESTING_H
