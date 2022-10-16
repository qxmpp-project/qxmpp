// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMESSAGEREACTION_H
#define QXMPPMESSAGEREACTION_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;
class QXmppMessageReactionPrivate;

class QXMPP_EXPORT QXmppMessageReaction
{
public:
    QXmppMessageReaction();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppMessageReaction)

    QString messageId() const;
    void setMessageId(const QString &messageId);

    QVector<QString> emojis() const;
    void setEmojis(const QVector<QString> &emojis);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isMessageReaction(const QDomElement &element);

private:
    QSharedDataPointer<QXmppMessageReactionPrivate> d;
};

#endif  // QXMPPMESSAGEREACTION_H
