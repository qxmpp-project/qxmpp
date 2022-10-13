// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMESSAGEREACTION_H
#define QXMPPMESSAGEREACTION_H

#include "QXmppGlobal.h"

#include "QVector"

class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppMessageReaction : public QVector<QString>
{
public:
    QXmppMessageReaction();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppMessageReaction)

    QString id() const;
    void setId(const QString &id);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isMessageReaction(const QDomElement &element);

private:
    QString m_id;
};

#endif  // QXMPPMESSAGEREACTION_H
