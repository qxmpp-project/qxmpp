// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>

class QXmppOutOfBandUrlPrivate;
class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppOutOfBandUrl
{
public:
    QXmppOutOfBandUrl();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppOutOfBandUrl)

    const QString &url() const;
    void setUrl(const QString &url);

    const std::optional<QString> &description() const;
    void setDescription(const std::optional<QString> &description);

    /// \cond
    bool parse(const QDomElement &el);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppOutOfBandUrlPrivate> d;
};
