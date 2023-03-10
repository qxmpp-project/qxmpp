// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBMANAGER_H
#define QXMPPPUBSUBMANAGER_H

#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMessage.h"
#include "QXmppPubSubIq_p.h"
#include "QXmppPubSubPublishOptions.h"
#include "QXmppResultSet.h"

class QXmppPubSubPublishOptions;
class QXmppPubSubSubscribeOptions;

class QXMPP_EXPORT QXmppPubSubManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    ///
    /// Type of PubSub service
    ///
    enum ServiceType {
        PubSubOrPep,  ///< PubSub service or PEP service
        PubSub,       ///< PubSub service only
        Pep           ///< PEP service only
    };

    ///
    /// Pre-defined ID of a PubSub item
    ///
    enum StandardItemId {
        Current  ///< Item of a singleton node (i.e., the node's single item)
    };

    ///
    /// Used to indicate a service type mismatch.
    ///
    struct InvalidServiceType
    {
    };

    template<typename T>
    struct Items
    {
        QVector<T> items;
        std::optional<QXmppResultSetReply> continuation;
    };

    using Result = std::variant<QXmpp::Success, QXmppError>;
    using FeaturesResult = std::variant<QVector<QString>, InvalidServiceType, QXmppError>;
    using NodesResult = std::variant<QVector<QString>, QXmppError>;
    using InstantNodeResult = std::variant<QString, QXmppError>;
    template<typename T>
    using ItemResult = std::variant<T, QXmppError>;
    template<typename T>
    using ItemsResult = std::variant<Items<T>, QXmppError>;
    using ItemIdsResult = std::variant<QVector<QString>, QXmppError>;
    using PublishItemResult = std::variant<QString, QXmppError>;
    using PublishItemsResult = std::variant<QVector<QString>, QXmppError>;
    using SubscriptionsResult = std::variant<QVector<QXmppPubSubSubscription>, QXmppError>;
    using AffiliationsResult = std::variant<QVector<QXmppPubSubAffiliation>, QXmppError>;
    using OptionsResult = std::variant<QXmppPubSubSubscribeOptions, QXmppError>;
    using NodeConfigResult = std::variant<QXmppPubSubNodeConfig, QXmppError>;

    QXmppPubSubManager();
    ~QXmppPubSubManager();

    // Generic PubSub (the PubSub service is the given entity)
    QXmppTask<NodesResult> requestNodes(const QString &jid);
    QXmppTask<Result> createNode(const QString &jid, const QString &nodeName);
    QXmppTask<Result> createNode(const QString &jid, const QString &nodeName, const QXmppPubSubNodeConfig &config);
    QXmppTask<InstantNodeResult> createInstantNode(const QString &jid);
    QXmppTask<InstantNodeResult> createInstantNode(const QString &jid, const QXmppPubSubNodeConfig &config);
    QXmppTask<Result> deleteNode(const QString &jid, const QString &nodeName);
    QXmppTask<ItemIdsResult> requestItemIds(const QString &serviceJid, const QString &nodeName);
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemResult<T>> requestItem(const QString &jid, const QString &nodeName, const QString &itemId);
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemResult<T>> requestItem(const QString &jid, const QString &nodeName, StandardItemId itemId);
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemsResult<T>> requestItems(const QString &jid, const QString &nodeName);
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemsResult<T>> requestItems(const QString &jid, const QString &nodeName, const QStringList &itemIds);
    template<typename T>
    QXmppTask<PublishItemResult> publishItem(const QString &jid, const QString &nodeName, const T &item);
    template<typename T>
    QXmppTask<PublishItemResult> publishItem(const QString &jid, const QString &nodeName, const T &item, const QXmppPubSubPublishOptions &publishOptions);
    template<typename T>
    QXmppTask<PublishItemsResult> publishItems(const QString &jid, const QString &nodeName, const QVector<T> &items);
    template<typename T>
    QXmppTask<PublishItemsResult> publishItems(const QString &jid, const QString &nodeName, const QVector<T> &items, const QXmppPubSubPublishOptions &publishOptions);
    QXmppTask<Result> retractItem(const QString &jid, const QString &nodeName, const QString &itemId);
    QXmppTask<Result> retractItem(const QString &jid, const QString &nodeName, StandardItemId itemId);
    QXmppTask<Result> purgeItems(const QString &jid, const QString &nodeName);
    QXmppTask<SubscriptionsResult> requestSubscriptions(const QString &jid);
    QXmppTask<SubscriptionsResult> requestSubscriptions(const QString &jid, const QString &nodeName);
    QXmppTask<AffiliationsResult> requestNodeAffiliations(const QString &jid, const QString &nodeName);
    QXmppTask<AffiliationsResult> requestAffiliations(const QString &jid);
    QXmppTask<AffiliationsResult> requestAffiliations(const QString &jid, const QString &nodeName);
    QXmppTask<OptionsResult> requestSubscribeOptions(const QString &service, const QString &nodeName);
    QXmppTask<OptionsResult> requestSubscribeOptions(const QString &service, const QString &nodeName, const QString &subscriberJid);
    QXmppTask<Result> setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options);
    QXmppTask<Result> setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options, const QString &subscriberJid);
    QXmppTask<NodeConfigResult> requestNodeConfiguration(const QString &service, const QString &nodeName);
    QXmppTask<Result> configureNode(const QString &service, const QString &nodeName, const QXmppPubSubNodeConfig &config);
    QXmppTask<Result> cancelNodeConfiguration(const QString &service, const QString &nodeName);
    QXmppTask<Result> subscribeToNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid);
    QXmppTask<Result> unsubscribeFromNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid);

    // PEP-specific (the PubSub service is the current account)
    QXmppTask<NodesResult> requestOwnPepNodes() { return requestNodes(client()->configuration().jidBare()); };
    QXmppTask<Result> createOwnPepNode(const QString &nodeName) { return createNode(client()->configuration().jidBare(), nodeName); }
    QXmppTask<Result> createOwnPepNode(const QString &nodeName, const QXmppPubSubNodeConfig &config) { return createNode(client()->configuration().jidBare(), nodeName, config); }
    QXmppTask<Result> deleteOwnPepNode(const QString &nodeName) { return deleteNode(client()->configuration().jidBare(), nodeName); }
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemResult<T>> requestOwnPepItem(const QString &nodeName, const QString &itemId) { return requestItem<T>(client()->configuration().jidBare(), nodeName, itemId); }
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemResult<T>> requestOwnPepItem(const QString &nodeName, StandardItemId itemId) { return requestItem<T>(client()->configuration().jidBare(), nodeName, itemId); }
    template<typename T = QXmppPubSubBaseItem>
    QXmppTask<ItemsResult<T>> requestOwnPepItems(const QString &nodeName) { return requestItems(client()->configuration().jidBare(), nodeName); }
    QXmppTask<ItemIdsResult> requestOwnPepItemIds(const QString &nodeName) { return requestItemIds(client()->configuration().jidBare(), nodeName); }
    template<typename T>
    QXmppTask<PublishItemResult> publishOwnPepItem(const QString &nodeName, const T &item, const QXmppPubSubPublishOptions &publishOptions);
    template<typename T>
    QXmppTask<PublishItemResult> publishOwnPepItem(const QString &nodeName, const T &item);
    template<typename T>
    QXmppTask<PublishItemsResult> publishOwnPepItems(const QString &nodeName, const QVector<T> &items, const QXmppPubSubPublishOptions &publishOptions);
    template<typename T>
    QXmppTask<PublishItemsResult> publishOwnPepItems(const QString &nodeName, const QVector<T> &items);
    QXmppTask<Result> retractOwnPepItem(const QString &nodeName, const QString &itemId) { return retractItem(client()->configuration().jidBare(), nodeName, itemId); }
    QXmppTask<Result> retractOwnPepItem(const QString &nodeName, StandardItemId itemId) { return retractItem(client()->configuration().jidBare(), nodeName, itemId); }
    QXmppTask<Result> purgeOwnPepItems(const QString &nodeName) { return purgeItems(client()->configuration().jidBare(), nodeName); }
    QXmppTask<NodeConfigResult> requestOwnPepNodeConfiguration(const QString &nodeName) { return requestNodeConfiguration(client()->configuration().jidBare(), nodeName); }
    QXmppTask<Result> configureOwnPepNode(const QString &nodeName, const QXmppPubSubNodeConfig &config) { return configureNode(client()->configuration().jidBare(), nodeName, config); }
    QXmppTask<Result> cancelOwnPepNodeConfiguration(const QString &nodeName) { return cancelNodeConfiguration(client()->configuration().jidBare(), nodeName); }

    static QString standardItemIdToString(StandardItemId itemId);

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

private:
    // for private requestFeatures() API
    friend class tst_QXmppPubSubManager;
    friend class QXmppOmemoManagerPrivate;

    QXmppTask<FeaturesResult> requestFeatures(const QString &serviceJid, ServiceType serviceType = PubSubOrPep);
    QXmppTask<FeaturesResult> requestOwnPepFeatures() { return requestFeatures(client()->configuration().jidBare(), Pep); };

    QXmppTask<PublishItemResult> publishItem(QXmpp::Private::PubSubIqBase &&iq);
    QXmppTask<PublishItemsResult> publishItems(QXmpp::Private::PubSubIqBase &&iq);
    static QXmpp::Private::PubSubIq<> requestItemsIq(const QString &jid, const QString &nodeName, const QStringList &itemIds);
};

///
/// Requests a specific item of an entity's node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this
/// should be an account's bare JID
/// \param nodeName the name of the node to query
/// \param itemId the ID of the item to retrieve
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::ItemResult<T>> QXmppPubSubManager::requestItem(const QString &jid,
                                                                             const QString &nodeName,
                                                                             const QString &itemId)
{
    using namespace QXmpp::Private;
    return chainIq(client()->sendIq(requestItemsIq(jid, nodeName, { itemId })), this,
                   [](PubSubIq<T> &&iq) -> ItemResult<T> {
                       if (!iq.items().isEmpty()) {
                           return iq.items().constFirst();
                       }
                       return QXmppError { QStringLiteral("No such item has been found."), {} };
                   });
}

///
/// Requests a specific item of an entity's node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this
/// should be an account's bare JID
/// \param nodeName the name of the node to query
/// \param itemId the ID of the item to retrieve
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::ItemResult<T>> QXmppPubSubManager::requestItem(const QString &jid,
                                                                             const QString &nodeName,
                                                                             StandardItemId itemId)
{
    return requestItem<T>(jid, nodeName, standardItemIdToString(itemId));
}

///
/// Requests all items of an entity's node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this
/// should be an account's bare JID
/// \param nodeName the name of the node to query
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::ItemsResult<T>> QXmppPubSubManager::requestItems(const QString &jid,
                                                                               const QString &nodeName)
{
    return requestItems<T>(jid, nodeName, {});
}

///
/// Requests items of an entity's node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service. For PEP this
/// should be an account's bare JID
/// \param nodeName the name of the node to query
/// \param itemIds the IDs of the items to retrieve. If empty, retrieves all
/// items
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::ItemsResult<T>> QXmppPubSubManager::requestItems(const QString &jid,
                                                                               const QString &nodeName,
                                                                               const QStringList &itemIds)
{
    using namespace QXmpp::Private;
    return chainIq(client()->sendIq(requestItemsIq(jid, nodeName, itemIds)), this,
                   [](PubSubIq<T> &&iq) -> ItemsResult<T> {
                       return Items<T> {
                           iq.items(),
                           iq.itemsContinuation(),
                       };
                   });
}

///
/// Publishs one item to a pubsub node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::publishItem with no publish options.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the item to
/// \param item the item to publish
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemResult> QXmppPubSubManager::publishItem(const QString &jid,
                                                                                 const QString &nodeName,
                                                                                 const T &item)
{
    QXmpp::Private::PubSubIq<T> request;
    request.setTo(jid);
    request.setItems({ item });
    request.setQueryNode(nodeName);
    return publishItem(std::move(request));
}

///
/// Publishs one item to a pubsub node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::publishItem with no publish options.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the item to
/// \param item the item to publish
/// \param publishOptions publish-options for the items
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemResult> QXmppPubSubManager::publishItem(const QString &jid,
                                                                                 const QString &nodeName,
                                                                                 const T &item,
                                                                                 const QXmppPubSubPublishOptions &publishOptions)
{
    QXmpp::Private::PubSubIq<T> request;
    request.setTo(jid);
    request.setItems({ item });
    request.setQueryNode(nodeName);
    request.setDataForm(publishOptions.toDataForm());
    return publishItem(std::move(request));
}

///
/// Publishs items to a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the items to
/// \param items the items to publish
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemsResult> QXmppPubSubManager::publishItems(const QString &jid,
                                                                                   const QString &nodeName,
                                                                                   const QVector<T> &items)
{
    QXmpp::Private::PubSubIq<T> request;
    request.setTo(jid);
    request.setItems(items);
    request.setQueryNode(nodeName);
    return publishItems(std::move(request));
}

///
/// Publishs items to a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to publish the items to
/// \param items the items to publish
/// \param publishOptions publish-options for the items
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemsResult> QXmppPubSubManager::publishItems(const QString &jid,
                                                                                   const QString &nodeName,
                                                                                   const QVector<T> &items,
                                                                                   const QXmppPubSubPublishOptions &publishOptions)
{
    QXmpp::Private::PubSubIq<T> request;
    request.setTo(jid);
    request.setItems(items);
    request.setQueryNode(nodeName);
    request.setDataForm(publishOptions.toDataForm());
    return publishItems(std::move(request));
}

///
/// Publishs one item to a PEP node.
///
/// \param nodeName the name of the PEP node to publish the item to
/// \param item the item to publish
/// \param publishOptions publish-options for fine tuning
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemResult> QXmppPubSubManager::publishOwnPepItem(const QString &nodeName, const T &item, const QXmppPubSubPublishOptions &publishOptions)
{
    return publishItem(client()->configuration().jidBare(), nodeName, item, publishOptions);
}

///
/// Publishs one item to a PEP node.
///
/// \param nodeName the name of the PEP node to publish the item to
/// \param item the item to publish
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemResult> QXmppPubSubManager::publishOwnPepItem(const QString &nodeName, const T &item)
{
    return publishItem(client()->configuration().jidBare(), nodeName, item);
}

///
/// Publishs items to a PEP node.
///
/// \param nodeName the name  of the PEP node to publish the items to
/// \param items the items to publish
/// \param publishOptions publish-options for fine tuning (optional). Pass
/// an empty form to honor the default options of the PEP node
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemsResult> QXmppPubSubManager::publishOwnPepItems(const QString &nodeName, const QVector<T> &items, const QXmppPubSubPublishOptions &publishOptions)
{
    return publishItems(client()->configuration().jidBare(), nodeName, items, publishOptions);
}

///
/// Publishs items to a PEP node.
///
/// \param nodeName the name of the PEP node to publish the items to
/// \param items the items to publish
/// \return
///
template<typename T>
QXmppTask<QXmppPubSubManager::PublishItemsResult> QXmppPubSubManager::publishOwnPepItems(const QString &nodeName, const QVector<T> &items)
{
    return publishItems(client()->configuration().jidBare(), nodeName, items);
}

#endif  // QXMPPPUBSUBMANAGER_H
