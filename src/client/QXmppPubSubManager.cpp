// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppPubSubAffiliation.h"
#include "QXmppPubSubEventManager.h"
#include "QXmppPubSubItem.h"
#include "QXmppPubSubSubscribeOptions.h"
#include "QXmppPubSubSubscription.h"
#include "QXmppStanza.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QFutureInterface>

using namespace QXmpp::Private;

///
/// \class QXmppPubSubManager
///
/// \brief The QXmppPubSubManager aims to provide publish-subscribe
/// functionality as specified in \xep{0060, Publish-Subscribe} (PubSub).
///
/// However, it currently only supports a few PubSub use cases but all of the
/// \xep{0060, Personal Eventing Protocol} (PEP) ones. PEP allows
/// a standard XMPP user account to function as a virtual PubSub service.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppPubSubManager *manager = new QXmppPubSubManager;
/// client->addExtension(manager);
/// \endcode
///
/// \note To subscribe to PEP event notifications use the
/// QXmppClientExtension::discoveryFeatures method of your client extension
/// according to section 9.2 of \xep{0060}. For example:
/// \code
/// QStringList YourExtension::discoveryFeatures() const
/// {
///    return { "http://jabber.org/protocol/tune+notify" };
/// }
/// \endcode
///
/// \todo
///  - Item pagination:
///    Requesting a continuation
///  - Requesting most recent items (max_items=x):
///    https://xmpp.org/extensions/xep-0060.html#subscriber-retrieve-requestrecent
///  - subscribe()/unsubscribe():
///    - return subscription on success
///    - correctly handle configuration required (and other) cases
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

///
/// \struct QXmppPubSubManager::Items<T>
///
/// Struct containing a list of items and a continuation if the results were
/// incomplete.
///

///
/// \typedef QXmppPubSubManager::Result
///
/// Result of a generic request without a return value. Contains Success in case
/// everything went well. If the returned IQ contained an error a
/// QXmppStanza::Error is reported.
///

///
/// \typedef QXmppPubSubManager::FeaturesResult
///
/// Type containing service discovery features, InvalidServiceType if the service is not of the
/// desired type or the returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::NodesResult
///
/// Type containing a list of node names or the returned IQ error
/// (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::InstantNodeResult
///
/// Contains the name of the new node (QString) or the returned IQ error
/// (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::ItemResult
///
/// Contains the item if it has been found (std::optional<T>) or the returned IQ
/// error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::ItemsResult
///
/// Contains all items that have been found (QVector<T>) or the returned IQ
/// error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::ItemIdsResult
///
/// Contains all item IDs that have been found (QVector<QString>) or the
/// returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::PublishItemResult
///
/// Contains the ID of the item, if no ID was set in the request (QString) or
/// the returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::PublishItemsResult
///
/// Contains the IDs of the items, if no IDs were set in the request
/// (QVector<QString>) or the returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::SubscriptionsResult
///
/// Contains a list of active subscriptions (QVector<QXmppPubSubSubscription>)
/// or the returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::AffiliationsResult
///
/// Contains the list of affiliations with the node(s)
/// (QVector<QXmppPubSubAffiliation>) or the returned IQ error
/// (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::OptionsResult
///
/// Contains the current subscribe options (QXmppPubSubSubscribeOptions) or the
/// returned IQ error (QXmppStanza::Error).
///

///
/// \typedef QXmppPubSubManager::NodeConfigResult
///
/// Contains a node configuration (QXmppPubSubNodeConfig) or the returned IQ
/// error (QXmppStanza::Error).
///

///
/// Default constructor.
///
QXmppPubSubManager::QXmppPubSubManager()
{
}

///
/// Default destructor.
///
QXmppPubSubManager::~QXmppPubSubManager()
{
}

///
/// Requests all features of a pubsub service and checks the identities via service discovery.
///
/// This uses a \xep{0030, Service Discovery} info request to get the service
/// identities and features.
///
/// The features are only returned if the service is of type serviceType,
/// otherwise InvalidServiceType is returned.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \param serviceJid JID of the entity hosting the pubsub service
/// \param serviceType type of service to retrieve features for
///
QFuture<QXmppPubSubManager::FeaturesResult> QXmppPubSubManager::requestFeatures(const QString &serviceJid, ServiceType serviceType)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(serviceJid);

    return chainIq(client()->sendIq(std::move(request)), this, [=](QXmppDiscoveryIq &&iq) -> FeaturesResult {
        const auto identities = iq.identities();

        const auto isPubSubServiceFound = std::any_of(identities.cbegin(), identities.cend(), [=](const QXmppDiscoveryIq::Identity &identity) {
            if (identity.category() == QStringLiteral("pubsub")) {
                const auto identityType = identity.type();

                switch (serviceType) {
                case PubSubOrPep:
                    return identityType == QStringLiteral("service") || identityType == QStringLiteral("pep");
                case PubSub:
                    return identityType == QStringLiteral("service");
                case Pep:
                    return identityType == QStringLiteral("pep");
                }
            }
            return false;
        });

        if (isPubSubServiceFound) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            return iq.features();
#else
            return iq.features().toVector();
#endif
        }

        return InvalidServiceType();
    });
}

///
/// Requests all listed nodes of a pubsub service via service discovery.
///
/// This uses a \xep{0030, Service Discovery} items request to get a list of
/// nodes.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \return
///
QFuture<QXmppPubSubManager::NodesResult> QXmppPubSubManager::fetchNodes(const QString &jid)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setTo(jid);

    return chainIq(client()->sendIq(std::move(request)), this, [](QXmppDiscoveryIq &&iq) -> NodesResult {
        const auto items = iq.items();
        QVector<QString> nodes;
        for (const auto &item : items) {
            // only accept non-empty nodes
            if (const auto node = item.node(); !node.isEmpty()) {
                nodes << node;
            }
        }
        // make unique
        std::sort(nodes.begin(), nodes.end());
        nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
        return nodes;
    });
}

///
/// Creates an empty pubsub node with the default configuration.
///
/// Calling this before QXmppPubSubManager::publishItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to be created
/// \return
///
auto QXmppPubSubManager::createNode(const QString &jid, const QString &nodeName) -> QFuture<Result>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Create);
    request.setQueryNode(nodeName);
    request.setTo(jid);

    return client()->sendGenericIq(std::move(request));
}

///
/// Creates an empty pubsub node with a custom configuration.
///
/// Calling this before QXmppPubSubManager::publishItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to be created
/// \param config The configuration for the node
/// \return
///
auto QXmppPubSubManager::createNode(const QString &jid, const QString &nodeName, const QXmppPubSubNodeConfig &config) -> QFuture<Result>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Create);
    request.setQueryNode(nodeName);
    request.setTo(jid);
    request.setDataForm(config);

    return client()->sendGenericIq(std::move(request));
}

///
/// Creates an instant pubsub node with the default configuration.
///
/// The pubsub service automatically generates a random node name. On success
/// it is returned via the QFuture.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \return
///
QFuture<QXmppPubSubManager::InstantNodeResult> QXmppPubSubManager::createInstantNode(const QString &jid)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Create);
    request.setTo(jid);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> InstantNodeResult {
                       return iq.queryNode();
                   });
}

///
/// Creates an instant pubsub node with a custom configuration.
///
/// The pubsub service automatically generates a random node name. On success
/// it is returned via the QFuture.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param config The configuration for the node
/// \return
///
auto QXmppPubSubManager::createInstantNode(const QString &jid, const QXmppPubSubNodeConfig &config) -> QFuture<InstantNodeResult>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Create);
    request.setTo(jid);
    request.setDataForm(config);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> InstantNodeResult {
                       return iq.queryNode();
                   });
}

///
/// Deletes a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete along with all of its items
/// \return
///
auto QXmppPubSubManager::deleteNode(const QString &jid, const QString &nodeName) -> QFuture<Result>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Delete);
    request.setQueryNode(nodeName);
    request.setTo(jid);

    return client()->sendGenericIq(std::move(request));
}

///
/// Requests the IDs of all items of a pubsub service node via service
/// discovery.
///
/// This uses a \xep{0030, Service Discovery} items request to get a list of
/// items.
///
/// \param serviceJid JID of the entity hosting the pubsub service
/// \param nodeName the name of the node whose items are requested
/// \return
///
QFuture<QXmppPubSubManager::ItemIdsResult> QXmppPubSubManager::requestItemIds(const QString &serviceJid, const QString &nodeName)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::ItemsQuery);
    request.setQueryNode(nodeName);
    request.setTo(serviceJid);

    return chainIq(client()->sendIq(std::move(request)), this, [](QXmppDiscoveryIq &&iq) -> ItemIdsResult {
        const auto queryItems = iq.items();
        QVector<QString> itemIds;
        itemIds.reserve(queryItems.size());
        for (const auto &queryItem : queryItems) {
            itemIds << queryItem.name();
        }
        return itemIds;
    });
}

///
/// Deletes an item from a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete the item from
/// \param itemId the ID of the item to delete
/// \return
///
auto QXmppPubSubManager::retractItem(const QString &jid, const QString &nodeName, const QString &itemId) -> QFuture<Result>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Retract);
    request.setQueryNode(nodeName);
    request.setItems({ QXmppPubSubItem(itemId) });
    request.setTo(jid);

    return client()->sendGenericIq(std::move(request));
}

///
/// Deletes an item from a pubsub node.
///
/// The default value of itemId is used for singleton nodes (i.e., the node's
/// single item is deleted).
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete the item from
/// \param itemId the ID of the item to delete
///
auto QXmppPubSubManager::retractItem(const QString &jid, const QString &nodeName, StandardItemId itemId) -> QFuture<Result>
{
    return retractItem(jid, nodeName, standardItemIdToString(itemId));
}

///
/// Purges all items from a node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the PEP node to delete along with all of its
/// items
/// \return
///
auto QXmppPubSubManager::purgeItems(const QString &jid, const QString &nodeName) -> QFuture<Result>
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIq<>::Purge);
    request.setQueryNode(nodeName);
    request.setTo(jid);

    return client()->sendGenericIq(std::move(request));
}

///
/// Requests all subscriptions with a PubSub service.
///
/// \param jid JID of the pubsub service
/// \return
///
QFuture<QXmppPubSubManager::SubscriptionsResult> QXmppPubSubManager::requestSubscriptions(const QString &jid)
{
    return requestSubscriptions(jid, {});
}

///
/// Requests the subscription(s) with a specific PubSub node.
///
/// \param jid JID of the pubsub service
/// \param nodeName Name of the node on the pubsub service
/// \return
///
QFuture<QXmppPubSubManager::SubscriptionsResult> QXmppPubSubManager::requestSubscriptions(const QString &jid, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(QXmppPubSubIq<>::Subscriptions);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> SubscriptionsResult {
                       return iq.subscriptions();
                   });
}

///
/// Requests the affiliations of all users on a PubSub node.
///
/// This can be used to view and manage affiliations of other users with a node.
/// Owner privileges are required.
///
/// \param jid JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service.
/// \return
///
QFuture<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestNodeAffiliations(const QString &jid, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(QXmppPubSubIq<>::OwnerAffiliations);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> AffiliationsResult {
                       return iq.affiliations();
                   });
}

///
/// Requests the user's affiliations with all PubSub nodes on a PubSub service.
///
/// \param jid JID of the pubsub service
/// \return
///
QFuture<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestAffiliations(const QString &jid)
{
    return requestAffiliations(jid, {});
}

///
/// Requests the user's affiliations with a PubSub node.
///
/// \param jid JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service.
/// \return
///
QFuture<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestAffiliations(const QString &jid, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(QXmppPubSubIq<>::Affiliations);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> AffiliationsResult {
                       return iq.affiliations();
                   });
}

///
/// Requests the subscribe options form of the own subscription to a node.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service.
/// \return
///
QFuture<QXmppPubSubManager::OptionsResult> QXmppPubSubManager::requestSubscribeOptions(const QString &service, const QString &nodeName)
{
    return requestSubscribeOptions(service, nodeName, client()->configuration().jidBare());
}

///
/// Requests the subscribe options form of a user's subscription to a node.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \param subscriberJid JID of the user to request the options for
/// \return
///
QFuture<QXmppPubSubManager::OptionsResult> QXmppPubSubManager::requestSubscribeOptions(const QString &service, const QString &nodeName, const QString &subscriberJid)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(service);
    request.setQueryType(QXmppPubSubIq<>::Options);
    request.setQueryNode(nodeName);
    request.setQueryJid(subscriberJid);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> OptionsResult {
                       if (const auto form = iq.dataForm()) {
                           if (const auto options = QXmppPubSubSubscribeOptions::fromDataForm(*form)) {
                               return *options;
                           }
                       }

                       // "real" stanza errors are already handled
                       using Error = QXmppStanza::Error;
                       return Error(Error::Cancel,
                                    Error::Condition::InternalServerError,
                                    QStringLiteral("Server returned invalid data form."));
                   });
}

///
/// Sets the subscription options for our own account.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \param options The new options to be set
/// \return
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options)
{
    return setSubscribeOptions(service, nodeName, options, client()->configuration().jidBare());
}

///
/// Sets the subscription options for another users's account.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \param options The new options to be set
/// \param subscriberJid The JID of the user
/// \return
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options, const QString &subscriberJid)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryType(QXmppPubSubIq<>::Options);
    request.setDataForm(options);
    request.setQueryNode(nodeName);
    request.setQueryJid(subscriberJid);
    return client()->sendGenericIq(std::move(request));
}

///
/// Requests the node configuration and starts the configuration process.
///
/// Requires owner privileges. If the result is successful (a node config form
/// has been returned) this starts the configuration process. The next step is
/// to call configureNode() or cancelNodeConfiguration().
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa configureNode()
/// \sa cancelNodeConfiguration()
///
QFuture<QXmppPubSubManager::NodeConfigResult> QXmppPubSubManager::requestNodeConfiguration(const QString &service, const QString &nodeName)
{
    using Error = QXmppStanza::Error;

    QXmppPubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(QXmppPubSubIq<>::Configure);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](QXmppPubSubIq<> &&iq) -> NodeConfigResult {
                       if (const auto dataForm = iq.dataForm()) {
                           if (const auto config = QXmppPubSubNodeConfig::fromDataForm(*dataForm)) {
                               return *config;
                           }
                           return Error(Error::Cancel, Error::UndefinedCondition, QStringLiteral("Server returned invalid data form."));
                       }
                       return Error(Error::Cancel, Error::UndefinedCondition, QStringLiteral("Server returned no data form."));
                   });
}

///
/// Sets a node configuration.
///
/// Requires owner privileges. You can use requestNodeConfiguration() to receive
/// a data form with all valid options and default values.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \param config
/// \return
///
/// \sa requestNodeConfiguration()
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::configureNode(const QString &service, const QString &nodeName, const QXmppPubSubNodeConfig &config)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(QXmppPubSubIq<>::Configure);
    request.setDataForm(config);
    return client()->sendGenericIq(std::move(request));
}

///
/// Cancels the configuration process and uses the default or exisiting
/// configuration.
///
/// \param service JID of the pubsub service
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa requestNodeConfiguration()
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::cancelNodeConfiguration(const QString &service, const QString &nodeName)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(QXmppPubSubIq<>::Configure);
    request.setDataForm(QXmppDataForm(QXmppDataForm::Cancel));
    return client()->sendGenericIq(std::move(request));
}

///
/// Subscribes to a node.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \param serviceJid JID of the pubsub service
/// \param nodeName name of the pubsub node being subscribed
/// \param subscriberJid bare or full JID of the subscriber
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::subscribeToNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(serviceJid);
    request.setQueryNode(nodeName);
    request.setQueryType(QXmppPubSubIq<>::Subscribe);
    request.setQueryJid(subscriberJid);
    return client()->sendGenericIq(std::move(request));
}

///
/// Unsubscribes from a node.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \param serviceJid JID of the pubsub service
/// \param nodeName name of the pubsub node being subscribed
/// \param subscriberJid bare or full JID of the subscriber
///
QFuture<QXmppPubSubManager::Result> QXmppPubSubManager::unsubscribeFromNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid)
{
    QXmppPubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(serviceJid);
    request.setQueryNode(nodeName);
    request.setQueryType(QXmppPubSubIq<>::Unsubscribe);
    request.setQueryJid(subscriberJid);
    return client()->sendGenericIq(std::move(request));
}

///
///
/// \fn QXmppPubSubManager::fetchPepNodes()
///
/// Requests all listed nodes of the own PEP service via service discovery.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::fetchNodes on the current account's bare JID.
///

///
/// \fn QFuture<Result> QXmppPubSubManager::createPepNode(const QString &nodeName)
///
/// Creates an empty PEP node with the default configuration.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::createNode on the current account's bare JID.
///
/// Calling this before QXmppPubSubManager::publishPepItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param nodeName the name of the PEP node to be created
/// \return
///

///
/// \fn QFuture<Result> QXmppPubSubManager::createPepNode(const QString &nodeName, const QXmppPubSubNodeConfig &config)
///
/// Creates an empty PEP node with a custom configuration.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::createNode on the current account's bare JID.
///
/// Calling this before QXmppPubSubManager::publishPepItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param nodeName the name of the PEP node to be created
/// \param config The configuration for the node
/// \return
///

///
/// \fn QXmppPubSubManager::deletePepNode
///
/// Deletes a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::deleteNode on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete along with all of its
/// items
/// \return
///

///
/// \fn QXmppPubSubManager::requestPepItem(const QString &nodeName, const QString &itemId)
///
/// Requests a specific item of a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestItem on the current account's bare JID.
///
/// \param nodeName name of the PEP node whose item is requested
/// \param itemId ID of the requested item
///

///
/// \fn QXmppPubSubManager::requestPepItem(const QString &nodeName, StandardItemId itemId)
///
/// Requests a specific item of a PEP node.
///
/// The default value of itemId is used for singleton nodes (i.e., the node's
/// single item is requested).
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestItem on the current account's bare JID.
///
/// \param nodeName name of the PEP node whose item is requested
/// \param itemId ID of the requested item
///

///
/// \fn QXmppPubSubManager::requestPepItems(const QString &nodeName)
///
/// Requests all items of a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestItems on the current account's bare JID.
///
/// \param nodeName name of the PEP node whose items are requested
///

///
/// \fn QXmppPubSubManager::requestPepItemIds(const QString &nodeName)
///
/// Requests the IDs of all items of a pubsub service node via service
/// discovery.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestItemIds on the current account's bare JID.
///
/// \param nodeName name of the PEP node whose item IDs are requested
///

///
/// \fn QXmppPubSubManager::retractPepItem(const QString &nodeName, const QString &itemId)
///
/// Deletes an item from a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::retractItem on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete the item from
/// \param itemId the ID of the item to delete
///

///
/// \fn QXmppPubSubManager::retractPepItem(const QString &nodeName, StandardItemId itemId)
///
/// Deletes an item from a PEP node.
///
/// The default value of itemId is used for singleton nodes (i.e., the node's
/// single item is deleted).
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::retractItem on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete the item from
/// \param itemId the ID of the item to delete
///

///
/// \fn QXmppPubSubManager::purgePepItems
///
/// Purges all items from a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::purgeItems on the current account's bare JID.
///
/// \param nodeName the name of the PEP node to delete along with all of its
/// items
/// \return
///

///
/// \fn QXmppPubSubManager::requestPepNodeConfiguration
///
/// Requests the node configuration and starts the configuration process.
///
/// This is a convenience method equivalent to calling
/// requestNodeConfiguration() the current account's bare JID.
///
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa configurePepNode()
/// \sa cancelPepNodeConfiguration()
///

///
/// \fn QXmppPubSubManager::configurePepNode
///
/// Sets a node configuration.
///
/// This is a convenience method equivalent to calling configureNode() the
/// current account's bare JID.
///
/// \param nodeName Name of the pubsub node on the service
/// \param config
/// \return
///
/// \sa requestPepNodeConfiguration()
///

///
/// \fn QXmppPubSubManager::cancelPepNodeConfiguration
///
/// This is a convenience method equivalent to calling cancelNodeConfiguration()
/// the current account's bare JID.
///
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa requestPepNodeConfiguration()
///

/// \cond
QStringList QXmppPubSubManager::discoveryFeatures() const
{
    return {
        ns_pubsub_rsm
    };
}

bool QXmppPubSubManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "message") {
        return false;
    }
    for (auto event = element.firstChildElement("event");
         !event.isNull();
         event = event.nextSiblingElement("event")) {
        if (event.namespaceURI() == ns_pubsub_event) {
            const auto service = element.attribute("from");
            const auto node = event.firstChildElement().attribute("node");

            const auto extensions = client()->extensions();
            for (auto *extension : extensions) {
                if (auto *eventManager = qobject_cast<QXmppPubSubEventManager *>(extension)) {
                    if (eventManager->handlePubSubEvent(element, service, node)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

QXmppPubSubIq<> QXmppPubSubManager::requestItemsIq(const QString &jid, const QString &nodeName, const QStringList &itemIds)
{
    QXmppPubSubIq request;
    request.setTo(jid);
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppPubSubIqBase::Items);
    request.setQueryNode(nodeName);

    if (!itemIds.isEmpty()) {
        QVector<QXmppPubSubItem> items;
        items.reserve(itemIds.size());
        for (const auto &id : itemIds) {
            items << QXmppPubSubItem(id);
        }
        request.setItems(items);
    }
    return request;
}

QString QXmppPubSubManager::standardItemIdToString(StandardItemId itemId)
{
    switch (itemId) {
    case Current:
        return QStringLiteral("current");
    }
    return {};
}

auto QXmppPubSubManager::publishItem(QXmppPubSubIqBase &&request) -> QFuture<PublishItemResult>
{
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIqBase::Publish);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> PublishItemResult {
                       if (!iq.items().isEmpty()) {
                           return iq.items().constFirst().id();
                       } else {
                           return QString();
                       }
                   });
}

auto QXmppPubSubManager::publishItems(QXmppPubSubIqBase &&request) -> QFuture<PublishItemsResult>
{
    request.setType(QXmppIq::Set);
    request.setQueryType(QXmppPubSubIqBase::Publish);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const QXmppPubSubIq<> &iq) -> PublishItemsResult {
                       const auto itemToId = [](const QXmppPubSubItem &item) {
                           return item.id();
                       };

                       const auto items = iq.items();
                       QVector<QString> ids(items.size());
                       std::transform(items.begin(), items.end(), ids.begin(), itemToId);
                       return ids;
                   });
}
/// \endcond
