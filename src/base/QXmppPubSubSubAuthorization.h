// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppDataFormBase.h"

class QXmppPubSubSubAuthorizationPrivate;

class QXMPP_EXPORT QXmppPubSubSubAuthorization : public QXmppExtensibleDataFormBase
{
public:
    static std::optional<QXmppPubSubSubAuthorization> fromDataForm(const QXmppDataForm &);

    QXmppPubSubSubAuthorization();
    QXmppPubSubSubAuthorization(const QXmppPubSubSubAuthorization &);
    QXmppPubSubSubAuthorization(QXmppPubSubSubAuthorization &&);
    ~QXmppPubSubSubAuthorization();

    QXmppPubSubSubAuthorization &operator=(const QXmppPubSubSubAuthorization &);
    QXmppPubSubSubAuthorization &operator=(QXmppPubSubSubAuthorization &&);

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
