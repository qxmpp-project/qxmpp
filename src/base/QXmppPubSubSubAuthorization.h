/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPPUBSUBSUBAUTHORIZATION_H
#define QXMPPPUBSUBSUBAUTHORIZATION_H

#include "QXmppDataFormBase.h"

class QXmppPubSubSubAuthorizationPrivate;

class QXMPP_EXPORT QXmppPubSubSubAuthorization : public QXmppExtensibleDataFormBase
{
public:
    static std::optional<QXmppPubSubSubAuthorization> fromDataForm(const QXmppDataForm &);

    QXmppPubSubSubAuthorization();
    QXmppPubSubSubAuthorization(const QXmppPubSubSubAuthorization &);
    ~QXmppPubSubSubAuthorization();

    QXmppPubSubSubAuthorization &operator=(const QXmppPubSubSubAuthorization &);

    std::optional<bool> allowSubscription() const;
    void setAllowSubscription(std::optional<bool> allowSubscription);

    QString node() const;
    void setNode(const QString &node);

    QString subscriberJid() const;
    void setSubscriberJid(const QString &subscriberJid);

    QString subid() const;
    void setSubid(const QString &subid);

protected:
    QString formType() const override;
    bool parseField(const QXmppDataForm::Field &) override;
    void serializeForm(QXmppDataForm &) const override;

private:
    QSharedDataPointer<QXmppPubSubSubAuthorizationPrivate> d;
};

#endif // QXMPPPUBSUBSUBAUTHORIZATION_H
