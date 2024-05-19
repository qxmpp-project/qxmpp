// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Germán Márquez Mejía <mancho@olomono.de>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppPubSubAffiliation.h"
#include "QXmppPubSubBaseItem.h"
#include "QXmppPubSubEventHandler.h"
#include "QXmppPubSubSubscribeOptions.h"
#include "QXmppPubSubSubscription.h"
#include "QXmppStanza.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "Algorithms.h"
#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

///
/// \class QXmppPubSubEventHandler
///
/// Interface for handling \xep{0060, Publish-Subscribe} (PubSub) events.
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppPubSubEventHandler::handlePubSubEvent()
///
/// Handles the PubSub event.
///
/// \param element QDomElement of the &lt;message/&gt; stanza
/// \param pubSubService JID of the PubSub service
/// \param nodeName Name of the PubSub node on the service
/// \returns Whether the event has been handled and should not be handled by other event handlers.
///

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

/// \cond
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
QXmppTask<QXmppPubSubManager::FeaturesResult> QXmppPubSubManager::requestFeatures(const QString &serviceJid, ServiceType serviceType)
{
    QXmppDiscoveryIq request;
    request.setType(QXmppIq::Get);
    request.setQueryType(QXmppDiscoveryIq::InfoQuery);
    request.setTo(serviceJid);

    return chainIq(client()->sendIq(std::move(request)), this, [=](QXmppDiscoveryIq &&iq) -> FeaturesResult {
        const auto identities = iq.identities();

        const auto isPubSubServiceFound = std::any_of(identities.cbegin(), identities.cend(), [=](const QXmppDiscoveryIq::Identity &identity) {
            if (identity.category() == u"pubsub") {
                const auto identityType = identity.type();

                switch (serviceType) {
                case PubSubOrPep:
                    return identityType == u"service" || identityType == u"pep";
                case PubSub:
                    return identityType == u"service";
                case Pep:
                    return identityType == u"pep";
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
/// \endcond

///
/// Requests all listed nodes of a pubsub service via service discovery.
///
/// This uses a \xep{0030, Service Discovery} items request to get a list of
/// nodes.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \return
///
QXmppTask<QXmppPubSubManager::NodesResult> QXmppPubSubManager::requestNodes(const QString &jid)
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
auto QXmppPubSubManager::createNode(const QString &jid, const QString &nodeName) -> QXmppTask<Result>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Create);
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
auto QXmppPubSubManager::createNode(const QString &jid, const QString &nodeName, const QXmppPubSubNodeConfig &config) -> QXmppTask<Result>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Create);
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
QXmppTask<QXmppPubSubManager::InstantNodeResult> QXmppPubSubManager::createInstantNode(const QString &jid)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Create);
    request.setTo(jid);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> InstantNodeResult {
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
auto QXmppPubSubManager::createInstantNode(const QString &jid, const QXmppPubSubNodeConfig &config) -> QXmppTask<InstantNodeResult>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Create);
    request.setTo(jid);
    request.setDataForm(config);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> InstantNodeResult {
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
auto QXmppPubSubManager::deleteNode(const QString &jid, const QString &nodeName) -> QXmppTask<Result>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Delete);
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
QXmppTask<QXmppPubSubManager::ItemIdsResult> QXmppPubSubManager::requestItemIds(const QString &serviceJid, const QString &nodeName)
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
auto QXmppPubSubManager::retractItem(const QString &jid, const QString &nodeName, const QString &itemId) -> QXmppTask<Result>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Retract);
    request.setQueryNode(nodeName);
    request.setItems({ QXmppPubSubBaseItem(itemId) });
    request.setTo(jid);

    return client()->sendGenericIq(std::move(request));
}

///
/// Deletes an item from a pubsub node.
///
/// \param jid Jabber ID of the entity hosting the pubsub service
/// \param nodeName the name of the node to delete the item from
/// \param itemId the ID of the item to delete
///
auto QXmppPubSubManager::retractItem(const QString &jid, const QString &nodeName, StandardItemId itemId) -> QXmppTask<Result>
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
auto QXmppPubSubManager::purgeItems(const QString &jid, const QString &nodeName) -> QXmppTask<Result>
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIq<>::Purge);
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
QXmppTask<QXmppPubSubManager::SubscriptionsResult> QXmppPubSubManager::requestSubscriptions(const QString &jid)
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
QXmppTask<QXmppPubSubManager::SubscriptionsResult> QXmppPubSubManager::requestSubscriptions(const QString &jid, const QString &nodeName)
{
    PubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(PubSubIq<>::Subscriptions);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> SubscriptionsResult {
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
QXmppTask<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestNodeAffiliations(const QString &jid, const QString &nodeName)
{
    PubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(PubSubIq<>::OwnerAffiliations);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> AffiliationsResult {
                       return iq.affiliations();
                   });
}

///
/// Requests the user's affiliations with all PubSub nodes on a PubSub service.
///
/// \param jid JID of the pubsub service
/// \return
///
QXmppTask<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestAffiliations(const QString &jid)
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
QXmppTask<QXmppPubSubManager::AffiliationsResult> QXmppPubSubManager::requestAffiliations(const QString &jid, const QString &nodeName)
{
    PubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    request.setQueryType(PubSubIq<>::Affiliations);
    request.setQueryNode(nodeName);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> AffiliationsResult {
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
QXmppTask<QXmppPubSubManager::OptionsResult> QXmppPubSubManager::requestSubscribeOptions(const QString &service, const QString &nodeName)
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
QXmppTask<QXmppPubSubManager::OptionsResult> QXmppPubSubManager::requestSubscribeOptions(const QString &service, const QString &nodeName, const QString &subscriberJid)
{
    PubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(service);
    request.setQueryType(PubSubIq<>::Options);
    request.setQueryNode(nodeName);
    request.setQueryJid(subscriberJid);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> OptionsResult {
                       if (const auto form = iq.dataForm()) {
                           if (const auto options = QXmppPubSubSubscribeOptions::fromDataForm(*form)) {
                               return *options;
                           }
                       }
                       return QXmppError { u"Server returned invalid data form."_s, {} };
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options)
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::setSubscribeOptions(const QString &service, const QString &nodeName, const QXmppPubSubSubscribeOptions &options, const QString &subscriberJid)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryType(PubSubIq<>::Options);
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
QXmppTask<QXmppPubSubManager::NodeConfigResult> QXmppPubSubManager::requestNodeConfiguration(const QString &service, const QString &nodeName)
{
    PubSubIq request;
    request.setType(QXmppIq::Get);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(PubSubIq<>::Configure);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](PubSubIq<> &&iq) -> NodeConfigResult {
                       if (const auto dataForm = iq.dataForm()) {
                           if (const auto config = QXmppPubSubNodeConfig::fromDataForm(*dataForm)) {
                               return *config;
                           }
                           return QXmppError { u"Server returned invalid data form."_s, {} };
                       }
                       return QXmppError { u"Server returned no data form."_s, {} };
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::configureNode(const QString &service, const QString &nodeName, const QXmppPubSubNodeConfig &config)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(PubSubIq<>::Configure);
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::cancelNodeConfiguration(const QString &service, const QString &nodeName)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(service);
    request.setQueryNode(nodeName);
    request.setQueryType(PubSubIq<>::Configure);
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::subscribeToNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(serviceJid);
    request.setQueryNode(nodeName);
    request.setQueryType(PubSubIq<>::Subscribe);
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
QXmppTask<QXmppPubSubManager::Result> QXmppPubSubManager::unsubscribeFromNode(const QString &serviceJid, const QString &nodeName, const QString &subscriberJid)
{
    PubSubIq request;
    request.setType(QXmppIq::Set);
    request.setTo(serviceJid);
    request.setQueryNode(nodeName);
    request.setQueryType(PubSubIq<>::Unsubscribe);
    request.setQueryJid(subscriberJid);
    return client()->sendGenericIq(std::move(request));
}

/// \cond
///
/// \fn QXmppPubSubManager::requestOwnPepFeatures()
///
/// Requests all features of the own PEP service via service discovery.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestFeatures on the current account's bare JID.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \endcond

///
///
/// \fn QXmppPubSubManager::requestOwnPepNodes()
///
/// Requests all listed nodes of the own PEP service via service discovery.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::fetchNodes on the current account's bare JID.
///

///
/// \fn QXmppTask<Result> QXmppPubSubManager::createOwnPepNode(const QString &nodeName)
///
/// Creates an empty PEP node with the default configuration.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::createNode on the current account's bare JID.
///
/// Calling this before QXmppPubSubManager::publishOwnPepItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param nodeName the name of the PEP node to be created
/// \return
///

///
/// \fn QXmppTask<Result> QXmppPubSubManager::createOwnPepNode(const QString &nodeName, const QXmppPubSubNodeConfig &config)
///
/// Creates an empty PEP node with a custom configuration.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::createNode on the current account's bare JID.
///
/// Calling this before QXmppPubSubManager::publishOwnPepItems is usually not
/// necessary when publishing to a node for the first time if the service
/// suppports the auto-create feature (Section 7.1.4 of \xep{0060}).
///
/// \param nodeName the name of the PEP node to be created
/// \param config The configuration for the node
/// \return
///

///
/// \fn QXmppPubSubManager::deleteOwnPepNode
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
/// \fn QXmppPubSubManager::requestOwnPepItem(const QString &nodeName, const QString &itemId)
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
/// \fn QXmppPubSubManager::requestOwnPepItem(const QString &nodeName, StandardItemId itemId)
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
/// \fn QXmppPubSubManager::requestOwnPepItems(const QString &nodeName)
///
/// Requests all items of a PEP node.
///
/// This is a convenience method equivalent to calling
/// QXmppPubSubManager::requestItems on the current account's bare JID.
///
/// \param nodeName name of the PEP node whose items are requested
///

///
/// \fn QXmppPubSubManager::requestOwnPepItemIds(const QString &nodeName)
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
/// \fn QXmppPubSubManager::retractOwnPepItem(const QString &nodeName, const QString &itemId)
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
/// \fn QXmppPubSubManager::retractOwnPepItem(const QString &nodeName, StandardItemId itemId)
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
/// \fn QXmppPubSubManager::purgeOwnPepItems
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
/// \fn QXmppPubSubManager::requestOwnPepNodeConfiguration
///
/// Requests the node configuration and starts the configuration process.
///
/// This is a convenience method equivalent to calling
/// requestNodeConfiguration() the current account's bare JID.
///
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa configureOwnPepNode()
/// \sa cancelOwnPepNodeConfiguration()
///

///
/// \fn QXmppPubSubManager::configureOwnPepNode
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
/// \sa requestOwnPepNodeConfiguration()
///

///
/// \fn QXmppPubSubManager::cancelOwnPepNodeConfiguration
///
/// This is a convenience method equivalent to calling cancelNodeConfiguration()
/// the current account's bare JID.
///
/// \param nodeName Name of the pubsub node on the service
/// \return
///
/// \sa requestOwnPepNodeConfiguration()
///

///
/// Returns a standard item ID string.
///
/// \param itemId standard item ID to be translated
/// \return the item ID string or a default-constructed string if there is no
///         corresponding one
///
QString QXmppPubSubManager::standardItemIdToString(StandardItemId itemId)
{
    switch (itemId) {
    case Current:
        return u"current"_s;
    }
    return {};
}

/// \cond
QStringList QXmppPubSubManager::discoveryFeatures() const
{
    return {
        ns_pubsub_rsm.toString()
    };
}

bool QXmppPubSubManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != u"message") {
        return false;
    }

    auto event = firstChildElement(element, u"event", ns_pubsub_event);
    if (!event.isNull()) {
        const auto service = element.attribute(u"from"_s);
        const auto node = event.firstChildElement().attribute(u"node"_s);

        const auto extensions = client()->extensions();
        for (auto *extension : extensions) {
            if (auto *eventHandler = dynamic_cast<QXmppPubSubEventHandler *>(extension)) {
                if (eventHandler->handlePubSubEvent(element, service, node)) {
                    return true;
                }
            }
        }
    }
    return false;
}

PubSubIq<> QXmppPubSubManager::requestItemsIq(const QString &jid, const QString &nodeName, const QStringList &itemIds)
{
    PubSubIq request;
    request.setTo(jid);
    request.setType(QXmppIq::Get);
    request.setQueryType(PubSubIqBase::Items);
    request.setQueryNode(nodeName);

    if (!itemIds.isEmpty()) {
        QVector<QXmppPubSubBaseItem> items;
        items.reserve(itemIds.size());
        for (const auto &id : itemIds) {
            items << QXmppPubSubBaseItem(id);
        }
        request.setItems(items);
    }
    return request;
}

auto QXmppPubSubManager::publishItem(PubSubIqBase &&request) -> QXmppTask<PublishItemResult>
{
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIqBase::Publish);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> PublishItemResult {
                       if (!iq.items().isEmpty()) {
                           return iq.items().constFirst().id();
                       } else {
                           return QString();
                       }
                   });
}

auto QXmppPubSubManager::publishItems(PubSubIqBase &&request) -> QXmppTask<PublishItemsResult>
{
    request.setType(QXmppIq::Set);
    request.setQueryType(PubSubIqBase::Publish);

    return chainIq(client()->sendIq(std::move(request)), this,
                   [](const PubSubIq<> &iq) -> PublishItemsResult {
                       const auto items = iq.items();
                       return transform<QVector<QString>>(items, [](const auto &item) {
                           return item.id();
                       });
                   });
}
/// \endcond
