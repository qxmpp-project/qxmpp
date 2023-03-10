// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBIQ_H
#define QXMPPPUBSUBIQ_H

#include "QXmppIq.h"

#include <QSharedDataPointer>

#if QXMPP_DEPRECATED_SINCE(1, 2)
#include "QXmppPubSubItem.h"
#endif

class QXmppPubSubIqPrivate;

#if QXMPP_DEPRECATED_SINCE(1, 5)
class QXMPP_EXPORT QXmppPubSubIq : public QXmppIq
{
public:
    enum [[deprecated]] QueryType {
        AffiliationsQuery,
        DefaultQuery,
        ItemsQuery,
        PublishQuery,
        RetractQuery,
        SubscribeQuery,
        SubscriptionQuery,
        SubscriptionsQuery,
        UnsubscribeQuery
    };

    [[deprecated]] QXmppPubSubIq();
    QXmppPubSubIq(const QXmppPubSubIq &iq);
    ~QXmppPubSubIq();

    QXmppPubSubIq &operator=(const QXmppPubSubIq &iq);

    [[deprecated]] QXmppPubSubIq::QueryType queryType() const;
    [[deprecated]] void setQueryType(QXmppPubSubIq::QueryType queryType);

    [[deprecated]] QString queryJid() const;
    [[deprecated]] void setQueryJid(const QString &jid);

    [[deprecated]] QString queryNode() const;
    [[deprecated]] void setQueryNode(const QString &node);

    [[deprecated]] QList<QXmppPubSubItem> items() const;
    [[deprecated]] void setItems(const QList<QXmppPubSubItem> &items);

    [[deprecated]] QString subscriptionId() const;
    [[deprecated]] void setSubscriptionId(const QString &id);

    [[deprecated]] static bool isPubSubIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    QSharedDataPointer<QXmppPubSubIqPrivate> d;
};
#endif

#endif  // QXMPPPUBSUBIQ_H
