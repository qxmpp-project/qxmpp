// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "QXmppComponentConfig.h"

class QXmppComponentConfigPrivate : public QSharedData
{
public:
    QString componentName;
    QString host;
    quint16 port;
    QString secret;
    bool parseAllMessages = true;
    bool parseAllPresences = true;
};

QXmppComponentConfig::QXmppComponentConfig()
    : d(new QXmppComponentConfigPrivate)
{
}

QXmppComponentConfig::QXmppComponentConfig(const QXmppComponentConfig &other) = default;
QXmppComponentConfig::~QXmppComponentConfig() = default;
QXmppComponentConfig &QXmppComponentConfig::operator=(const QXmppComponentConfig &other) = default;

QString QXmppComponentConfig::componentName() const
{
    return d->componentName;
}

void QXmppComponentConfig::setComponentName(const QString &componentName)
{
    d->componentName = componentName;
}

QString QXmppComponentConfig::host() const
{
    return d->host;
}

void QXmppComponentConfig::setHost(const QString &host)
{
    d->host = host;
}

quint16 QXmppComponentConfig::port() const
{
    return d->port;
}

void QXmppComponentConfig::setPort(const quint16 &port)
{
    d->port = port;
}

QString QXmppComponentConfig::secret() const
{
    return d->secret;
}

void QXmppComponentConfig::setSecret(const QString &secret)
{
    d->secret = secret;
}

bool QXmppComponentConfig::parseAllMessages() const
{
    return d->parseAllMessages;
}

void QXmppComponentConfig::setParseAllMessages(bool parseAllMessages)
{
    d->parseAllMessages = parseAllMessages;
}

bool QXmppComponentConfig::parseAllPresences() const
{
    return d->parseAllPresences;
}

void QXmppComponentConfig::setParseAllPresences(bool parseAllPresences)
{
    d->parseAllPresences = parseAllPresences;
}
