/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#include "QXmppIq.h"

#include <QDomElement>
#include <QSharedDataPointer>

#include <optional>

class QXmppDataForm;
class QXmppPubSubIqPrivate;
class QXmppPubSubItem;
class QXmppPubSubSubscription;
class QXmppPubSubAffiliation;
class QXmppResultSetReply;

class QXMPP_EXPORT QXmppPubSubIqBase : public QXmppIq
{
public:
    /// This enum is used to describe a publish-subscribe query type.
    enum QueryType {
        Affiliations,
        OwnerAffiliations,
        Configure,
        Create,
        Default,
        OwnerDefault,
        Delete,
        Items,
        Options,
        Publish,
        Purge,
        Retract,
        Subscribe,
        Subscription,
        Subscriptions,
        OwnerSubscriptions,
        Unsubscribe,
    };

    QXmppPubSubIqBase();
    QXmppPubSubIqBase(const QXmppPubSubIqBase &other);
    ~QXmppPubSubIqBase();

    QXmppPubSubIqBase &operator=(const QXmppPubSubIqBase &other);

    QXmppPubSubIqBase::QueryType queryType() const;
    void setQueryType(QXmppPubSubIqBase::QueryType queryType);

    QString queryJid() const;
    void setQueryJid(const QString &queryJid);

    QString queryNode() const;
    void setQueryNode(const QString &queryNode);

    QString subscriptionId() const;
    void setSubscriptionId(const QString &subscriptionId);

    QList<QXmppPubSubSubscription> subscriptions() const;
    void setSubscriptions(const QList<QXmppPubSubSubscription> &);

    std::optional<QXmppPubSubSubscription> subscription() const;
    void setSubscription(const std::optional<QXmppPubSubSubscription> &);

    QList<QXmppPubSubAffiliation> affiliations() const;
    void setAffiliations(const QList<QXmppPubSubAffiliation> &);

    quint32 maxItems() const;
    void setMaxItems(quint32);

    std::optional<QXmppDataForm> dataForm() const;
    void setDataForm(const std::optional<QXmppDataForm> &);

    std::optional<QXmppResultSetReply> itemsContinuation() const;
    void setItemsContinuation(const std::optional<QXmppResultSetReply> &itemsContinuation);

    /// \cond
    static bool isPubSubIq(const QDomElement &element);

protected:
    static bool isPubSubIq(const QDomElement &element,
                           bool (*isItemValid)(const QDomElement &));

    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

    virtual void parseItems(const QDomElement &queryElement) = 0;
    virtual void serializeItems(QXmlStreamWriter *writer) const = 0;
    /// \endcond

private:
    static std::optional<QueryType> queryTypeFromDomElement(const QDomElement &element);
    static bool queryTypeIsOwnerIq(QueryType type);

    QSharedDataPointer<QXmppPubSubIqPrivate> d;
};

template<class T = QXmppPubSubItem>
class QXmppPubSubIq : public QXmppPubSubIqBase
{
public:
    QList<T> items() const;
    void setItems(const QList<T> &items);

    static bool isPubSubIq(const QDomElement &element);

protected:
    /// \cond
    void parseItems(const QDomElement &queryElement) override;
    void serializeItems(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QList<T> m_items;
};

template<class T>
QList<T> QXmppPubSubIq<T>::items() const
{
    return m_items;
}

template<class T>
void QXmppPubSubIq<T>::setItems(const QList<T> &items)
{
    m_items = items;
}

template<class T>
bool QXmppPubSubIq<T>::isPubSubIq(const QDomElement &element)
{
    return QXmppPubSubIqBase::isPubSubIq(element, [](const QDomElement &item) -> bool {
        return T::isItem(item);
    });
}

/// \cond
template<class T>
void QXmppPubSubIq<T>::parseItems(const QDomElement &queryElement)
{
    QDomElement childElement = queryElement.firstChildElement(QStringLiteral("item"));
    while (!childElement.isNull()) {
        T item;
        item.parse(childElement);
        m_items << item;

        childElement = childElement.nextSiblingElement(QStringLiteral("item"));
    }
}

template<class T>
void QXmppPubSubIq<T>::serializeItems(QXmlStreamWriter *writer) const
{
    for (const auto &item : std::as_const(m_items)) {
        item.toXml(writer);
    }
}
/// \endcond

#endif  // QXMPPPUBSUBIQ_H
