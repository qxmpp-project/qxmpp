// SPDX-FileCopyrightText: 2024 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCREDENTIALS_H
#define QXMPPCREDENTIALS_H

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>

struct QXmppCredentialsPrivate;
class QXmlStreamReader;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppCredentials
{
public:
    QXmppCredentials();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppCredentials)

    static std::optional<QXmppCredentials> fromXml(QXmlStreamReader &);
    void toXml(QXmlStreamWriter &) const;

    /// Comparison operator
    bool operator==(const QXmppCredentials &other) const;
    /// Comparison operator
    bool operator!=(const QXmppCredentials &other) const = default;

private:
    friend class QXmppConfiguration;

    QSharedDataPointer<QXmppCredentialsPrivate> d;
};

#endif  // QXMPPCREDENTIALS_H
