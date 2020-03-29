/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Germán Márquez Mejía
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

#ifndef QXMPPPUBSUBIQ_H
#define QXMPPPUBSUBIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"
#include "QXmppPubSubItem.h"

#include <QSharedDataPointer>

#if QXMPP_DEPRECATED_SINCE(1, 2)
#include "QXmppPubSubItem.h"
#endif

class QXmppPubSubIqPrivate;

///
/// \brief The QXmppPubSubIq class represents an IQ used for the
/// publish-subscribe mechanisms defined by \xep{0060}: Publish-Subscribe.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppPubSubIq : public QXmppIq
{
public:
    /// This enum is used to describe a publish-subscribe query type.
    enum QueryType {
        AffiliationsQuery,
        DefaultQuery,
        ItemsQuery,
        PublishQuery,
        RetractQuery,
        SubscribeQuery,
        SubscriptionQuery,
        SubscriptionsQuery,
        UnsubscribeQuery,
        CreateQuery,
        DeleteQuery
    };

    QXmppPubSubIq();
    QXmppPubSubIq(const QXmppPubSubIq &other);
    ~QXmppPubSubIq();

    QXmppPubSubIq &operator=(const QXmppPubSubIq &other);

    QXmppPubSubIq::QueryType queryType() const;
    void setQueryType(QXmppPubSubIq::QueryType queryType);

    QString queryJid() const;
    void setQueryJid(const QString &queryJid);

    QString queryNodeName() const;
    void setQueryNodeName(const QString &queryNodeName);

    QList<QXmppPubSubItem> items() const;
    void setItems(const QList<QXmppPubSubItem> &items);

    QString subscriptionId() const;
    void setSubscriptionId(const QString &subscriptionId);

    QXmppDataForm publishOptions() const;
    void setPublishOptions(const QXmppDataForm &publishOptions);

    /// \cond
    static bool isPubSubIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubIqPrivate> d;
};

#endif  // QXMPPPUBSUBIQ_H
