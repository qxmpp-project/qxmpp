// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBIQ_H
#define QXMPPPUBSUBIQ_H

#include "QXmppIq.h"

#include <optional>

#include <QDomElement>
#include <QSharedDataPointer>

class QXmppDataForm;
class QXmppPubSubBaseItem;
class QXmppPubSubSubscription;
class QXmppPubSubAffiliation;
class QXmppResultSetReply;

namespace QXmpp::Private {

class PubSubIqPrivate;

class QXMPP_EXPORT PubSubIqBase : public QXmppIq
{
public:
    /// This enum is used to describe a publish-subscribe query type.
    enum QueryType : uint8_t {
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

    PubSubIqBase();
    PubSubIqBase(const PubSubIqBase &other);
    ~PubSubIqBase();

    PubSubIqBase &operator=(const PubSubIqBase &other);

    QueryType queryType() const;
    void setQueryType(QueryType queryType);

    QString queryJid() const;
    void setQueryJid(const QString &queryJid);

    QString queryNode() const;
    void setQueryNode(const QString &queryNode);

    QString subscriptionId() const;
    void setSubscriptionId(const QString &subscriptionId);

    QVector<QXmppPubSubSubscription> subscriptions() const;
    void setSubscriptions(const QVector<QXmppPubSubSubscription> &);

    std::optional<QXmppPubSubSubscription> subscription() const;
    void setSubscription(const std::optional<QXmppPubSubSubscription> &);

    QVector<QXmppPubSubAffiliation> affiliations() const;
    void setAffiliations(const QVector<QXmppPubSubAffiliation> &);

    std::optional<uint32_t> maxItems() const;
    void setMaxItems(std::optional<uint32_t>);

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

    QSharedDataPointer<PubSubIqPrivate> d;
};

template<typename T = QXmppPubSubBaseItem>
class PubSubIq : public PubSubIqBase
{
public:
    QVector<T> items() const;
    void setItems(const QVector<T> &items);

    static bool isPubSubIq(const QDomElement &element);

protected:
    /// \cond
    void parseItems(const QDomElement &queryElement) override;
    void serializeItems(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QVector<T> m_items;
};

template<typename T>
QVector<T> PubSubIq<T>::items() const
{
    return m_items;
}

template<typename T>
void PubSubIq<T>::setItems(const QVector<T> &items)
{
    m_items = items;
}

template<typename T>
bool PubSubIq<T>::isPubSubIq(const QDomElement &element)
{
    return PubSubIqBase::isPubSubIq(element, [](const QDomElement &item) -> bool {
        return T::isItem(item);
    });
}

/// \cond
template<typename T>
void PubSubIq<T>::parseItems(const QDomElement &queryElement)
{
    for (auto childElement = queryElement.firstChildElement(QStringLiteral("item"));
         !childElement.isNull();
         childElement = childElement.nextSiblingElement(QStringLiteral("item"))) {
        T item;
        item.parse(childElement);
        m_items.push_back(std::move(item));
    }
}

template<typename T>
void PubSubIq<T>::serializeItems(QXmlStreamWriter *writer) const
{
    for (const auto &item : std::as_const(m_items)) {
        item.toXml(writer);
    }
}
/// \endcond

}  // namespace QXmpp::Private

#endif  // QXMPPPUBSUBIQ_H
