/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#ifndef INTEGRATIONTESTING_H
#define INTEGRATIONTESTING_H

#include <QtGlobal>
#include <QDebug>

#include "QXmppConfiguration.h"

#define ENV_ENABLED "QXMPP_TESTS_INTEGRATION_ENABLED"
#define ENV_JID "QXMPP_TESTS_JID"
#define ENV_PASSWORD "QXMPP_TESTS_PASSWORD"

#define SKIP_IF_INTEGRATION_TESTS_DISABLED() \
    if (!IntegrationTests::enabled()) { \
        QSKIP("Export 'QXMPP_TESTS_INTEGRATION_ENABLED=1' to enable."); \
    } else if (!IntegrationTests::credentialsAvailable()) { \
        QFAIL("No credentials for integration tests provided! " \
              "Export 'QXMPP_TESTS_JID' and 'QXMPP_TESTS_PASSWORD'."); \
    }

class IntegrationTests
{
public:
    static QString environmentVariable(const char *varName, const QString &defaultValue = {})
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        return qEnvironmentVariable(varName, defaultValue);
#else
        return qEnvironmentVariableIsSet(varName) ? QString::fromLocal8Bit(qgetenv(varName)) : QString();
#endif
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

#endif // INTEGRATIONTESTING_H
