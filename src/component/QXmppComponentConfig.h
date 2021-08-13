// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef QXMPPCOMPONENTCONFIG_H
#define QXMPPCOMPONENTCONFIG_H

#include <QSharedDataPointer>

class QXmppComponentConfigPrivate;

class QXmppComponentConfig
{
public:
    QXmppComponentConfig();
    QXmppComponentConfig(const QXmppComponentConfig &other);
    ~QXmppComponentConfig();
    QXmppComponentConfig &operator=(const QXmppComponentConfig &other);

    QString componentName() const;
    void setComponentName(const QString &componentName);

    QString host() const;
    void setHost(const QString &host);

    quint16 port() const;
    void setPort(const quint16 &port);

    QString secret() const;
    void setSecret(const QString &secret);

    bool parseAllMessages() const;
    void setParseAllMessages(bool parseAllMessages);

    bool parseAllPresences() const;
    void setParseAllPresences(bool parseAllPresences);

private:
    QSharedDataPointer<QXmppComponentConfigPrivate> d;
};

#endif // QXMPPCOMPONENTCONFIG_H
