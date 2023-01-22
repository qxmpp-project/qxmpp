// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QXmppPubSubEvent.h>
#include <QXmppPubSubManager.h>

namespace QXmpp::Private::Pep {

template<typename T>
using GetResult = std::variant<T, QXmppError>;
using PublishResult = std::variant<QString, QXmppError>;

template<typename ItemT>
inline QXmppTask<GetResult<ItemT>> request(QXmppPubSubManager *pubSub, const QString &jid, const QString &nodeName, QObject *parent)
{
    using PubSub = QXmppPubSubManager;

    auto process = [](PubSub::ItemsResult<ItemT> &&result) -> GetResult<ItemT> {
        if (const auto itemsResult = std::get_if<PubSub::Items<ItemT>>(&result)) {
            if (!itemsResult->items.isEmpty()) {
                return itemsResult->items.takeFirst();
            }
            return QXmppError { QStringLiteral("User has no published items."), {} };
        } else {
            return std::get<QXmppError>(std::move(result));
        }
    };
    return chain<GetResult<ItemT>>(pubSub->requestItems<ItemT>(jid, nodeName), parent, process);
}

// NodeName is a template parameter, so the right qstring comparison overload is used
// (if we used 'const QString &' as type, a 'const char *' string would be converted)
template<typename ItemT, typename NodeName, typename Manager, typename ReceivedSignal>
inline bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &eventNode, NodeName nodeName, Manager *manager, ReceivedSignal itemReceived)
{
    if (eventNode == nodeName && QXmppPubSubEvent<ItemT>::isPubSubEvent(element)) {
        QXmppPubSubEvent<ItemT> event;
        event.parse(element);

        if (event.eventType() == QXmppPubSubEventBase::Items) {
            if (!event.items().isEmpty()) {
                (manager->*itemReceived)(pubSubService, event.items().constFirst());
            } else {
                (manager->*itemReceived)(pubSubService, {});
            }
            return true;
        } else if (event.eventType() == QXmppPubSubEventBase::Retract) {
            (manager->*itemReceived)(pubSubService, {});
            return true;
        }
    }
    return false;
}

}  // namespace QXmpp::Private::Pep
