// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBEVENT_H
#define QXMPPPUBSUBEVENT_H

#include "QXmppMessage.h"
#include "QXmppPubSubSubscription.h"

#include <functional>

#include <QDomElement>
#include <QSharedData>

class QXmppDataForm;
class QXmppPubSubEventPrivate;
class QXmppPubSubBaseItem;

class QXMPP_EXPORT QXmppPubSubEventBase : public QXmppMessage
{
public:
    ///
    /// Enumeration of different event types
    ///
    enum EventType : uint8_t {
        Configuration,
        Delete,
        Items,
        Retract,
        Purge,
        Subscription,
    };

    QXmppPubSubEventBase(EventType = Items, const QString &node = {});
    QXmppPubSubEventBase(const QXmppPubSubEventBase &other);
    QXmppPubSubEventBase(QXmppPubSubEventBase &&);
    virtual ~QXmppPubSubEventBase();

    QXmppPubSubEventBase &operator=(const QXmppPubSubEventBase &other);
    QXmppPubSubEventBase &operator=(QXmppPubSubEventBase &&);

    EventType eventType() const;
    void setEventType(EventType);

    QString node() const;
    void setNode(const QString &node);

    QStringList retractIds() const;
    void setRetractIds(const QStringList &);

    QString redirectUri() const;
    void setRedirectUri(const QString &);

    std::optional<QXmppPubSubSubscription> subscription() const;
    void setSubscription(const std::optional<QXmppPubSubSubscription> &subscription);

    std::optional<QXmppDataForm> configurationForm() const;
    void setConfigurationForm(const std::optional<QXmppDataForm> &configurationForm);

protected:
    /// \cond
    static bool isPubSubEvent(const QDomElement &element, std::function<bool(const QDomElement &)> isItemValid);

    bool parseExtension(const QDomElement &element, QXmpp::SceMode) override;
    void serializeExtensions(QXmlStreamWriter *writer, QXmpp::SceMode, const QString &baseNamespace) const override;

    virtual void parseItems(const QDomElement &) = 0;
    virtual void serializeItems(QXmlStreamWriter *writer) const = 0;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubEventPrivate> d;
};

template<typename T = QXmppPubSubBaseItem>
class QXmppPubSubEvent : public QXmppPubSubEventBase
{
public:
    QVector<T> items() const;
    void setItems(const QVector<T> &items);

    static bool isPubSubEvent(const QDomElement &element);

protected:
    /// \cond
    void parseItems(const QDomElement &) override;
    void serializeItems(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QVector<T> m_items;
};

///
/// Returns the PubSub items of the event.
///
template<typename T>
QVector<T> QXmppPubSubEvent<T>::items() const
{
    return m_items;
}

///
/// Sets the PubSub items of the event.
///
template<typename T>
void QXmppPubSubEvent<T>::setItems(const QVector<T> &items)
{
    m_items = items;
}

///
/// Returns whether the element is a valid QXmppPubSubEvent and contains only
/// valid items of type T.
///
template<typename T>
bool QXmppPubSubEvent<T>::isPubSubEvent(const QDomElement &element)
{
    return QXmppPubSubEventBase::isPubSubEvent(element, [](const QDomElement &element) {
        return T::isItem(element);
    });
}

/// \cond
template<typename T>
void QXmppPubSubEvent<T>::parseItems(const QDomElement &parent)
{
    QDomElement child = parent.firstChildElement(QStringLiteral("item"));
    while (!child.isNull()) {
        T item;
        item.parse(child);
        m_items << item;

        child = child.nextSiblingElement(QStringLiteral("item"));
    }
}

template<typename T>
void QXmppPubSubEvent<T>::serializeItems(QXmlStreamWriter *writer) const
{
    for (const auto &item : std::as_const(m_items)) {
        item.toXml(writer);
    }
}
/// \endcond

Q_DECLARE_METATYPE(QXmppPubSubEventBase::EventType)

#endif  // QXMPPPUBSUBEVENT_H
