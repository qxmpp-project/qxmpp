// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppClientExtension.h"
#include "QXmppError.h"
#include "QXmppExternalService.h"
#include "QXmppTask.h"

#include <variant>

class QDateTime;
class QXmppExternalServicePrivate;
class QXMPP_EXPORT QXmppExternalServiceDiscoveryManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppExternalServiceDiscoveryManager();
    ~QXmppExternalServiceDiscoveryManager();

    using ServicesResult = std::variant<QVector<QXmppExternalService>, QXmppError>;

    QXmppTask<ServicesResult> requestServices(const QString &jid, const QString &node = {});

    /// \cond
    QStringList discoveryFeatures() const override;
    /// \endcond
};
