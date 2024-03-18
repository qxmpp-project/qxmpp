// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSASL2USERAGENT_H
#define QXMPPSASL2USERAGENT_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>
#include <QUuid>

struct QXmppSasl2UserAgentPrivate;

class QXMPP_EXPORT QXmppSasl2UserAgent
{
public:
    QXmppSasl2UserAgent();
    QXmppSasl2UserAgent(QUuid deviceId, const QString &softwareName, const QString &deviceName);

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppSasl2UserAgent)

    QUuid deviceId() const;
    void setDeviceId(QUuid);

    const QString &softwareName() const;
    void setSoftwareName(const QString &);

    const QString &deviceName() const;
    void setDeviceName(const QString &);

private:
    QSharedDataPointer<QXmppSasl2UserAgentPrivate> d;
};

#endif  // QXMPPSASL2USERAGENT_H
