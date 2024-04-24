// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMessage.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixInvitation.h"
#include "QXmppMixIq.h"
#include "QXmppMixIq_p.h"
#include "QXmppPubSubEvent.h"
#include "QXmppPubSubManager.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"

#include "Algorithms.h"

#include <QDomElement>

using namespace QXmpp::Private;

class QXmppMixManagerPrivate
{
public:
    QXmppPubSubManager *pubSubManager = nullptr;
    QXmppDiscoveryManager *discoveryManager = nullptr;
    bool supportedByServer = false;
    bool archivingSupportedByServer = false;
    QList<QXmppMixManager::Service> services;
};

///
/// \class QXmppMixManager
///
/// This class manages group chat communication as specified in the following XEPs:
///     * \xep{0369, Mediated Information eXchange (MIX)}
///     * \xep{0405, Mediated Information eXchange (MIX): Participant Server Requirements}
///     * \xep{0406, Mediated Information eXchange (MIX): MIX Administration}
///     * \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}
///
/// In order to use this manager, make sure to add all managers needed by this manager:
/// \code
/// client->addNewExtension<QXmppDiscoveryManager>();
/// client->addNewExtension<QXmppPubSubManager>();
/// \endcode
///
/// Afterwards, you need to add this manager to the client:
/// \code
/// auto *manager = client->addNewExtension<QXmppMixManager>();
/// \endcode
///
/// If you want to be informed about updates of the channel (e.g., its configuration or allowed
/// JIDs), make sure to subscribe to the corresponding nodes.
///
/// In order to send a message to a MIX channel, you have to set the type QXmppMessage::GroupChat.
///
/// Example for an unencrypted message:
/// \code
/// message->setType(QXmppMessage::GroupChat);
/// message->setTo("group@mix.example.org")
/// client->send(std::move(message));
/// \endcode
///
/// Example for an encrypted message decryptable by Alice and Bob:
/// \code
/// message->setType(QXmppMessage::GroupChat);
/// message->setTo("group@mix.example.org")
///
/// QXmppSendStanzaParams params;
/// params.setEncryptionJids({ "alice@example.org", "bob@example.com" })
///
/// client->sendSensitive(std::move(message), params);
/// \endcode
///
/// If you receive a message, you can check whether it contains an invitation to a MIX channel and
/// use that invitation to join the channel:
/// \code
/// if (auto invitation = message.mixInvitation()) {
///     manager->joinChannel(*invitation, nickname, nodes);
/// }
/// \endcode
///
/// In order to invite someone to a MIX channel that allows the invitee to participate, you can
/// create an invitation on your own.
///
/// That can be done in the following cases:
///     * Everybody is allowed to participate in the channel.
///     * The invitee is explicitly allowed to participate in the channel.
///       That is particularly relevant if the channel does not support invitations received via
///       requestInvitation() but the inviter is permitted to allow JIDs via allowJid().
///       In that case, the invitee's JID has to be allowed before the invitee tries to join the
///       channel.
///
/// Invitations are sent as regular messages.
/// They are not meant to be read by a human.
/// Instead, the receiving client needs to support it.
/// But you can add an appropriate text to the body of the invitation message to enable human users
/// of clients that do not support that feature to join the channel manually.
/// For example, you could add the JID of the channel or an XMPP URI to the body.
///
/// If the body (i.e., the invitation text) is empty, the message would neither be delivered to all
/// clients via \xep{0280, Message Carbons} nor delivered to clients which are currently offline via
/// \xep{0313, Message Archive Management}.
/// To enforce that behavior, set a corresponding message type and message processing hint:
/// \code
/// QXmppMixInvitation invitation;
///
/// invitation.setInviterJid(client->configuration().jidBare());
/// invitation.setInviteeJid(inviteeJid);
/// invitation.setChannelJid(channelJid);
///
/// QXmppMessage message;
///
/// message.setTo(inviteeJid);
/// message.setMixInvitation(invitation);
///
/// if (messageBody.isEmpty()) {
///     message.setType(QXmppMessage::Chat);
///     message.addHint(QXmppMessage::Store);
/// } else {
///     message.setBody(messageBody);
/// }
///
/// client()->sendSensitive(std::move(message));
/// \endcode
///
/// In order to invite someone to a MIX channel that does not yet allow the invitee to participate,
/// you can request an invitation from the channel (if permitted to do so):
/// \code
/// manager->requestInvitation().then(this, [](QXmppMixManager::InvitationResult &&result) mutable {
///     if (auto *error = std::get_if<QXmppError>(&result)) {
///         // Handle the error.
///     } else {
///         auto invitation = std::get<QXmppMixInvitation>(std::move(result));
///         // Create and send the invitation.
///     }
/// });
/// \endcode
///
/// \ingroup Managers
///
/// \since QXmpp 1.7
///

///
/// \property QXmppMixManager::supportedByServer
///
/// \see QXmppMixManager::supportedByServer()
///

///
/// \property QXmppMixManager::archivingSupportedByServer
///
/// \see QXmppMixManager::archivingSupportedByServer()
///

///
/// \property QXmppMixManager::services
///
/// \see QXmppMixManager::services()
///

///
/// \struct QXmppMixManager::Service
///
/// Service providing MIX channels and corresponding nodes.
///
/// \var QXmppMixManager::Service::jid
///
/// JID of the service.
///
/// \var QXmppMixManager::Service::channelsSearchable
///
/// Whether the service can be searched for channels.
///
/// \var QXmppMixManager::Service::channelCreationAllowed
///
/// Whether channels can be created on the service.
///

/// \cond
bool QXmppMixManager::Service::operator==(const Service &other) const
{
    return jid == other.jid &&
        channelsSearchable == other.channelsSearchable &&
        channelCreationAllowed == other.channelCreationAllowed;
}
/// \endcond

///
/// \struct QXmppMixManager::Subscription
///
/// Subscription to nodes of a MIX channel.
///
/// \var QXmppMixManager::Subscription::additions
///
/// Nodes belonging to the channel that are subscribed to.
///
/// \var QXmppMixManager::Subscription::removals
///
/// Nodes belonging to the channel that are unsubscribed from.
///

///
/// \struct QXmppMixManager::Participation
///
/// Participation in a channel.
///
/// \var QXmppMixManager::Participation::participantId
///
/// ID of the user within the channel.
///
/// \var QXmppMixManager::Participation::nickname
///
/// Nickname of the user within the channel.
///
/// If the server modified the desired nickname, this is the modified one.
///
/// \var QXmppMixManager::Participation::subscriptions
///
/// Nodes belonging to the joined channel that are subscribed to.
///
/// If not all desired nodes could be subscribed, this contains only the subscribed nodes.
///

///
/// \typedef QXmppMixManager::Jid
///
/// JID of a user or domain.
///

///
/// \typedef QXmppMixManager::ChannelJid
///
/// JID of a MIX channel.
///

///
/// \typedef QXmppMixManager::Nickname
///
/// Nickname of the user within a MIX channel.
///
/// If the server modified the desired nickname, this is the modified one.
///

///
/// \typedef QXmppMixManager::CreationResult
///
/// Contains the JID of the created MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::ChannelJidResult
///
/// Contains the JIDs of all discoverable MIX channels of a MIX service or a QXmppError if it
/// failed.
///

///
/// \typedef QXmppMixManager::ChannelNodeResult
///
/// Contains all nodes of the requested MIX channel that can be subscribed by the user or a
/// QXmppError on failure.
///

///
/// \typedef QXmppMixManager::ConfigurationResult
///
/// Contains the configuration of the MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::InformationResult
///
/// Contains the information of the MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::JoiningResult
///
/// Contains the result of the joined MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::NicknameResult
///
/// Contains the new nickname within a joined MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::InvitationResult
///
/// Contains the requested invitation to a MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::SubscriptionResult
///
/// Contains the result of the subscribed/unsubscribed nodes belonging to a MIX channel or a
/// QXmppError on failure.
///

///
/// \typedef QXmppMixManager::JidResult
///
/// Contains the JIDs of users or domains that are allowed to participate resp. banned from
/// participating in a MIX channel or a QXmppError on failure.
///

///
/// \typedef QXmppMixManager::ParticipantResult
///
/// Contains the participants of a MIX channel or a QXmppError on failure.
///

constexpr QStringView MIX_SERVICE_DISCOVERY_NODE = u"mix";

///
/// Constructs a MIX manager.
///
QXmppMixManager::QXmppMixManager()
    : d(new QXmppMixManagerPrivate())
{
}

QXmppMixManager::~QXmppMixManager() = default;

QStringList QXmppMixManager::discoveryFeatures() const
{
    return { ns_mix.toString() };
}

///
/// Returns whether the own server supports MIX clients.
///
/// In that case, the server interacts between a client and a MIX service.
/// E.g., the server adds a MIX service to the client's roster after joining it and archives the
/// messages sent through the channel while the client is offline.
///
/// \return whether MIX clients are supported
///
bool QXmppMixManager::supportedByServer() const
{
    return d->supportedByServer;
}

///
/// \fn QXmppMixManager::supportedByServerChanged()
///
/// Emitted when the server enabled or disabled supporting MIX clients.
///

///
/// Returns whether the own server supports archiving messages via
/// \xep{0313, Message Archive Management} of MIX channels the user participates in.
///
/// \return whether MIX messages are archived
///
bool QXmppMixManager::archivingSupportedByServer() const
{
    return d->archivingSupportedByServer;
}

///
/// \fn QXmppMixManager::archivingSupportedByServerChanged()
///
/// Emitted when the server enabled or disabled supporting archiving for MIX.
///

///
/// Returns the services providing MIX on the own server.
///
/// Such services provide MIX channels and their nodes.
/// It interacts directly with clients or with their servers.
///
/// \return the provided MIX services
///
QList<QXmppMixManager::Service> QXmppMixManager::services() const
{
    return d->services;
}

///
/// \fn QXmppMixManager::servicesChanged()
///
/// Emitted when the services providing MIX on the own server changed.
///

///
/// Creates a MIX channel.
///
/// If no channel ID is passed, the channel is created with an ID provided by the MIX service.
/// Furthermore, the channel cannot be discovered by anyone.
/// A channel with the mentioned properties is called an "ad-hoc channel".
///
/// The channel ID is the local part of the channel JID.
/// The MIX service JID is the domain part of the channel JID.
/// Example: "channel" is the channel ID and "mix.example.org" the service JID of the channel JID
/// "channel@mix.example.org".
///
/// \param serviceJid JID of the service
/// \param channelId ID of the channel (default: provided by the server)
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::CreationResult> QXmppMixManager::createChannel(const QString &serviceJid, const QString &channelId)
{
    QXmppMixIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(serviceJid);
    iq.setActionType(QXmppMixIq::Create);
    iq.setChannelId(channelId);

    return chainIq(client()->sendIq(std::move(iq)), this, [](QXmppMixIq &&iq) -> CreationResult {
        return iq.channelJid().isEmpty() ? iq.channelId() + u'@' + iq.from() : iq.channelJid();
    });
}

///
/// Requests the JIDs of all discoverable MIX channels of a MIX service.
///
/// \param serviceJid JID of the service that provides the channels
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::ChannelJidResult> QXmppMixManager::requestChannelJids(const QString &serviceJid)
{
    return chainMapSuccess(d->discoveryManager->requestDiscoItems(serviceJid), this, [](QList<QXmppDiscoveryIq::Item> &&items) {
        return transform<QVector<ChannelJid>>(items, [](const QXmppDiscoveryIq::Item &item) {
            return item.jid();
        });
    });
}

///
/// Requests all nodes of a MIX channel that can be subscribed by the user.
///
/// \param channelJid JID of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::ChannelNodeResult> QXmppMixManager::requestChannelNodes(const QString &channelJid)
{
    return chainMapSuccess(d->discoveryManager->requestDiscoItems(channelJid, MIX_SERVICE_DISCOVERY_NODE.toString()), this, [](QList<QXmppDiscoveryIq::Item> &&items) {
        return listToMixNodes(transform<QVector<QString>>(items, [](const QXmppDiscoveryIq::Item &item) {
            return item.node();
        }));
    });
}

///
/// Requests the configuration of a MIX channel.
///
/// \param channelJid JID of the channel whose configuration is requested
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::ConfigurationResult> QXmppMixManager::requestChannelConfiguration(const QString &channelJid)
{
    return chainMapSuccess(d->pubSubManager->requestItems<QXmppMixConfigItem>(channelJid, ns_mix_node_config.toString()), this, [](QXmppPubSubManager::Items<QXmppMixConfigItem> &&items) {
        return items.items.takeFirst();
    });
}

///
/// Updates the configuration of a MIX channel.
///
/// In order to use this method, retrieve the current configuration via
/// requestChannelConfiguration() first, change the desired attributes and pass the configuration to
/// this method.
///
/// \param channelJid JID of the channel whose configuration is to be updated
/// \param configuration new configuration of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::updateChannelConfiguration(const QString &channelJid, QXmppMixConfigItem configuration)
{
    configuration.setFormType(QXmppDataForm::Submit);
    return chainSuccess(d->pubSubManager->publishItem(channelJid, ns_mix_node_config.toString(), configuration), this);
}

///
/// \fn QXmppMixManager::channelConfigurationUpdated(const QString &channelJid, const QXmppMixConfigItem &configuration)
///
/// Emitted when the configuration of a MIX channel is updated.
///
/// \param channelJid JID of the channel whose configuration is updated
/// \param configuration new channel configuration
///

///
/// Requests the information of a MIX channel.
///
/// \param channelJid JID of the channel whose information is requested
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::InformationResult> QXmppMixManager::requestChannelInformation(const QString &channelJid)
{
    return chainMapSuccess(d->pubSubManager->requestItems<QXmppMixInfoItem>(channelJid, ns_mix_node_info.toString()), this, [](QXmppPubSubManager::Items<QXmppMixInfoItem> &&items) {
        return items.items.takeFirst();
    });
}

///
/// Updates the information of a MIX channel.
///
/// In order to use this method, retrieve the current information via requestChannelInformation()
/// first, change the desired attributes and pass the information to this method.
///
/// \param channelJid JID of the channel whose information is to be updated
/// \param information new information of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::updateChannelInformation(const QString &channelJid, QXmppMixInfoItem information)
{
    information.setFormType(QXmppDataForm::Submit);
    return chainSuccess(d->pubSubManager->publishItem(channelJid, ns_mix_node_info.toString(), information), this);
}

///
/// \fn QXmppMixManager::channelInformationUpdated(const QString &channelJid, const QXmppMixInfoItem &information)
///
/// Emitted when the information of a MIX channel is updated.
///
/// \param channelJid JID of the channel whose information is updated
/// \param information new channel information
///

///
/// Joins a MIX channel to become a participant of it.
///
/// \param channelJid JID of the channel being joined
/// \param nickname nickname of the user which is usually required by the server (default: no
///        nickname is set)
/// \param nodes nodes of the channel that are subscribed to for receiving their updates (default:
///        all nodes are subcribed to)
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JoiningResult> QXmppMixManager::joinChannel(const QString &channelJid, const QString &nickname, QXmppMixConfigItem::Nodes nodes)
{
    return joinChannel(prepareJoinIq(channelJid, nickname, nodes));
}

///
/// Joins a MIX channel via an invitation to become a participant of it.
///
/// \param invitation invitation to the channel
/// \param nickname nickname of the user which is usually required by the server (default: no
///        nickname is set)
/// \param nodes nodes of the channel that are subscribed to for receiving their updates (default:
///        all nodes are subcribed to)
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JoiningResult> QXmppMixManager::joinChannel(const QXmppMixInvitation &invitation, const QString &nickname, QXmppMixConfigItem::Nodes nodes)
{
    auto iq = prepareJoinIq(invitation.channelJid(), nickname, nodes);

    // Submit the invitation only if it was generated by the channel and thus needed to join.
    if (!invitation.token().isEmpty()) {
        iq.setInvitation(invitation);
    }

    return joinChannel(std::move(iq));
}

///
/// Updates the nickname within a channel.
///
/// If the update succeeded, the new nickname is returned which may differ from the requested one.
///
/// \param channelJid JID of the channel
/// \param nickname nickname to be set
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::NicknameResult> QXmppMixManager::updateNickname(const QString &channelJid, const QString &nickname)
{
    QXmppMixIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(channelJid);
    iq.setActionType(QXmppMixIq::SetNick);
    iq.setNick(nickname);

    return chainIq(client()->sendIq(std::move(iq)), this, [](QXmppMixIq &&iq) -> NicknameResult {
        return iq.nick();
    });
}

///
/// Updates the subscriptions to nodes of a MIX channel.
///
/// \param channelJid JID of the channel
/// \param subscriptionAdditions nodes to subscribe to
/// \param subscriptionRemovals nodes to unsubscribe from
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::SubscriptionResult> QXmppMixManager::updateSubscriptions(const QString &channelJid, QXmppMixConfigItem::Nodes subscriptionAdditions, QXmppMixConfigItem::Nodes subscriptionRemovals)
{
    QXmppMixSubscriptionUpdateIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(channelJid);
    iq.setAdditions(subscriptionAdditions);
    iq.setRemovals(subscriptionRemovals);

    return chainIq(client()->sendIq(std::move(iq)), this, [](QXmppMixSubscriptionUpdateIq &&iq) -> SubscriptionResult {
        return Subscription { iq.additions(), iq.removals() };
    });
}

///
/// Requests an invitation to a MIX channel that the invitee is not yet allowed to participate in.
///
/// The invitee can use the invitation to join the channel.
///
/// That invitation mechanism avoids storing allowed JIDs for an indefinite time if the
/// corresponding user never joins the channel.
/// By using this method, there is no need to allow the invitee to participate in the channel via
/// allowJid().
///
/// This method can be used in the following cases:
///     * The inviter is an administrator of the channel.
///     * The inviter is a participant of the channel and the channel allows all participants to
///       invite new users.
///
/// \param channelJid JID of the channel that the invitee is invited to
/// \param inviteeJid JID of the invitee
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::InvitationResult> QXmppMixManager::requestInvitation(const QString &channelJid, const QString &inviteeJid)
{
    QXmppMixInvitationRequestIq iq;
    iq.setType(QXmppIq::Get);
    iq.setTo(channelJid);
    iq.setInviteeJid(inviteeJid);

    return chainIq(client()->sendIq(std::move(iq)), this, [](QXmppMixInvitationResponseIq &&iq) -> InvitationResult {
        return iq.invitation();
    });
}

///
/// Requests all JIDs which are allowed to participate in a MIX channel.
///
/// The JIDs can specify users (e.g., "alice@example.org") or groups of users (e.g., "example.org")
/// to let all users join which have a JID containing the specified domain.
/// This is only relevant/used for private channels having a user-specified JID.
///
/// \param channelJid JID of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JidResult> QXmppMixManager::requestAllowedJids(const QString &channelJid)
{
    return requestJids(channelJid, ns_mix_node_allowed.toString());
}

///
/// Allows a JID to participate in a MIX channel.
///
/// The JID can specify a user (e.g., "alice@example.org") or groups of users (e.g., "example.org")
/// to let all users join which have a JID containing the specified domain.
///
/// Allowing a JID is only needed if the channel does not allow anyone to participate.
/// That is the case when QXmppMixConfigItem::Node::AllowedJids exists for the channel.
/// Use requestChannelConfiguration() and QXmppMixConfigItem::nodes() to determine that.
/// Call updateChannelConfiguration() and QXmppMixConfigItem::setNodes() to update it accordingly.
/// In order to allow all JIDs to participate in a channel, you need to remove
/// QXmppMixConfigItem::Node::AllowedJids from the channel's nodes.
///
/// \param channelJid JID of the channel
/// \param jid bare JID to be allowed
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::allowJid(const QString &channelJid, const QString &jid)
{
    return addJidToNode(channelJid, ns_mix_node_allowed.toString(), jid);
}

///
/// \fn QXmppMixManager::jidAllowed(const QString &channelJid, const QString &jid)
///
/// Emitted when a JID is allowed to participate in a MIX channel.
///
/// That happens if allowJid() was successful or if another resource or user did that.
///
/// \param channelJid JID of the channel
/// \param jid allowed bare JID
///

///
/// \fn QXmppMixManager::allJidsAllowed(const QString &channelJid)
///
/// Emitted when all JIDs are allowed to participate in a MIX channel.
///
/// That happens if QXmppMixConfigItem::Node::AllowedJids is removed from a channel.
///
/// \param channelJid JID of the channel
///

///
/// Disallows a formerly allowed JID to participate in a MIX channel.
///
/// Only allowed JIDs can be disallowed via this method.
/// In order to disallow other JIDs, use banJid().
///
/// \param channelJid JID of the channel
/// \param jid bare JID to be disallowed
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::disallowJid(const QString &channelJid, const QString &jid)
{
    return d->pubSubManager->retractItem(channelJid, ns_mix_node_allowed.toString(), jid);
}

///
/// \fn QXmppMixManager::jidDisallowed(const QString &channelJid, const QString &jid)
///
/// Emitted when a fomerly allowed JID is disallowed to participate in a MIX channel anymore.
///
/// That happens if disallowJid() was successful or if another resource or user did that.
///
/// \param channelJid JID of the channel
/// \param jid disallowed bare JID
///

///
/// Disallows all formerly allowed JIDs to participate in a MIX channel.
///
/// Only allowed JIDs can be disallowed via this method.
/// In order to disallow other JIDs, use banJid().
///
/// \param channelJid JID of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::disallowAllJids(const QString &channelJid)
{
    return d->pubSubManager->purgeItems(channelJid, ns_mix_node_allowed.toString());
}

///
/// \fn QXmppMixManager::allJidsDisallowed(const QString &channelJid)
///
/// Emitted when no JID is allowed to participate in a MIX channel anymore.
///
/// That happens if disallowAllJids() was successful or if another resource or user did that.
///
/// \param channelJid JID of the channel
///

///
/// Requests all JIDs which are not allowed to participate in a MIX channel.
///
/// \param channelJid JID of the corresponding channel
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JidResult> QXmppMixManager::requestBannedJids(const QString &channelJid)
{
    return requestJids(channelJid, ns_mix_node_banned.toString());
}

///
/// Bans a JID from participating in a MIX channel.
///
/// The JID can specify a user (e.g., "alice@example.org") or groups of users (e.g., "example.org")
/// to ban all users which have a JID containing the specified domain.
///
/// Before calling this, make sure that QXmppMixConfigItem::Node::BannedJids exists for the channel.
/// Use requestChannelConfiguration() and QXmppMixConfigItem::nodes() to determine that.
/// Call updateChannelConfiguration() and QXmppMixConfigItem::setNodes() to update it accordingly.
///
/// \param channelJid JID of the channel
/// \param jid bare JID to be banned
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::banJid(const QString &channelJid, const QString &jid)
{
    return addJidToNode(channelJid, ns_mix_node_banned.toString(), jid);
}

///
/// \fn QXmppMixManager::jidBanned(const QString &channelJid, const QString &jid)
///
/// Emitted when a JID is banned from participating in a MIX channel.
///
/// That happens if banJid() was successful or if another resource or user did that.
///
/// \param channelJid JID of the channel
/// \param jid banned bare JID
///

///
/// Unbans a formerly banned JID from participating in a MIX channel.
///
/// \param channelJid JID of the channel
/// \param jid bare JID to be unbanned
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::unbanJid(const QString &channelJid, const QString &jid)
{
    return d->pubSubManager->retractItem(channelJid, ns_mix_node_banned.toString(), jid);
}

///
/// \fn QXmppMixManager::jidUnbanned(const QString &channelJid, const QString &jid)
///
/// Emitted when a formerly banned JID is unbanned from participating in a MIX channel.
///
/// That happens if unbanJid() was successful or if another resource or user did that.
///
/// \param channelJid JID of the channel
/// \param jid unbanned bare JID
///

///
/// Unbans all formerly banned JIDs from participating in a MIX channel.
///
/// \param channelJid JID of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::unbanAllJids(const QString &channelJid)
{
    return d->pubSubManager->purgeItems(channelJid, ns_mix_node_banned.toString());
}

///
/// \fn QXmppMixManager::allJidsUnbanned(const QString &channelJid)
///
/// Emitted when all JIDs are unbanned from participating in a MIX channel.
///
/// That happens if unbanAllJids() was successful or if another resource or user did that.
/// Furthermore, that happens if QXmppMixConfigItem::Node::BannedJids is removed from a channel.
///
/// \param channelJid JID of the channel
///

///
/// Requests all participants of a MIX channel.
///
/// In the case of a channel that not everybody is allowed to participate in, the participants are a
/// subset of the allowed JIDs.
///
/// \param channelJid JID of the channel
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::ParticipantResult> QXmppMixManager::requestParticipants(const QString &channelJid)
{
    return chainMapSuccess(d->pubSubManager->requestItems<QXmppMixParticipantItem>(channelJid, ns_mix_node_participants.toString()), this, [](QXmppPubSubManager::Items<QXmppMixParticipantItem> &&items) {
        return items.items;
    });
}

///
/// \fn QXmppMixManager::participantReceived(const QString &channelJid, const QXmppMixParticipantItem &participant)
///
/// Emitted when a user joined a MIX channel or a participant is updated.
///
/// \param channelJid JID of the channel that the user joined or whose participant is updated
/// \param participant new or updated participant
///

///
/// \fn QXmppMixManager::participantLeft(const QString &channelJid, const QString &participantId)
///
/// Emitted when a participant left the MIX channel.
///
/// \param channelJid JID of the channel that is left by the participant
/// \param participantId ID of the left participant
///

///
/// Leaves a MIX channel.
///
/// \param channelJid JID of the channel to be left
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::leaveChannel(const QString &channelJid)
{
    QXmppMixIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(client()->configuration().jidBare());
    iq.setActionType(QXmppMixIq::ClientLeave);
    iq.setChannelJid(channelJid);

    return client()->sendGenericIq(std::move(iq));
}

///
/// Deletes a MIX channel.
///
/// \param channelJid JID of the channel to be deleted
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::deleteChannel(const QString &channelJid)
{
    QXmppMixIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(QXmppUtils::jidToDomain(channelJid));
    iq.setActionType(QXmppMixIq::Destroy);
    iq.setChannelId(QXmppUtils::jidToUser(channelJid));

    return client()->sendGenericIq(std::move(iq));
}

///
/// \fn QXmppMixManager::channelDeleted(const QString &channelJid)
///
/// Emitted when a MIX channel is deleted.
///
/// \param channelJid JID of the deleted channel
///

/// \cond
void QXmppMixManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::connected, this, [this, client]() {
        if (client->streamManagementState() == QXmppClient::NewStream) {
            resetCachedData();
        }
    });

    d->discoveryManager = client->findExtension<QXmppDiscoveryManager>();
    Q_ASSERT_X(d->discoveryManager, "QXmppMixManager", "QXmppDiscoveryManager is missing");

    connect(d->discoveryManager, &QXmppDiscoveryManager::infoReceived, this, &QXmppMixManager::handleDiscoInfo);

    d->pubSubManager = client->findExtension<QXmppPubSubManager>();
    Q_ASSERT_X(d->pubSubManager, "QXmppMixManager", "QXmppPubSubManager is missing");
}

void QXmppMixManager::onUnregistered(QXmppClient *client)
{
    disconnect(d->discoveryManager, &QXmppDiscoveryManager::infoReceived, this, &QXmppMixManager::handleDiscoInfo);
    resetCachedData();
    disconnect(client, &QXmppClient::connected, this, nullptr);
}

bool QXmppMixManager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    if (nodeName == ns_mix_node_allowed && QXmppPubSubEvent<QXmppPubSubBaseItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppPubSubBaseItem> event;
        event.parse(element);

        switch (event.eventType()) {
        // Items have been published.
        case QXmppPubSubEventBase::Items: {
            const auto items = event.items();
            for (const auto &item : items) {
                Q_EMIT jidAllowed(pubSubService, item.id());
            }
            break;
        }
        // Specific items are deleted.
        case QXmppPubSubEventBase::Retract: {
            const auto ids = event.retractIds();
            for (const auto &id : ids) {
                Q_EMIT jidDisallowed(pubSubService, id);
            }
            break;
        }
        // All items are deleted.
        case QXmppPubSubEventBase::Purge:
            Q_EMIT allJidsDisallowed(pubSubService);
            break;
        // The whole node is deleted.
        case QXmppPubSubEventBase::Delete:
            Q_EMIT allJidsAllowed(pubSubService);
            break;
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    if (nodeName == ns_mix_node_banned && QXmppPubSubEvent<QXmppPubSubBaseItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppPubSubBaseItem> event;
        event.parse(element);

        switch (event.eventType()) {
        // Items have been published.
        case QXmppPubSubEventBase::Items: {
            const auto items = event.items();
            for (const auto &item : items) {
                Q_EMIT jidBanned(pubSubService, item.id());
            }
            break;
        }
        // Specific items are deleted.
        case QXmppPubSubEventBase::Retract: {
            const auto ids = event.retractIds();
            for (const auto &id : ids) {
                Q_EMIT jidUnbanned(pubSubService, id);
            }
            break;
        }
        // All items are deleted.
        case QXmppPubSubEventBase::Purge:
        // The whole node is deleted.
        case QXmppPubSubEventBase::Delete:
            Q_EMIT allJidsUnbanned(pubSubService);
            break;
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    if (nodeName == ns_mix_node_config && QXmppPubSubEvent<QXmppMixConfigItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppMixConfigItem> event;
        event.parse(element);

        switch (event.eventType()) {
        case QXmppPubSubEventBase::Items: {
            const auto item = event.items().constFirst();
            Q_EMIT channelConfigurationUpdated(pubSubService, item);
            break;
        }
        case QXmppPubSubEventBase::Retract:
        case QXmppPubSubEventBase::Purge:
        case QXmppPubSubEventBase::Delete:
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    if (nodeName == ns_mix_node_info && QXmppPubSubEvent<QXmppMixInfoItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppMixInfoItem> event;
        event.parse(element);

        switch (event.eventType()) {
        case QXmppPubSubEventBase::Items: {
            const auto item = event.items().constFirst();
            Q_EMIT channelInformationUpdated(pubSubService, item);
            break;
        }
        case QXmppPubSubEventBase::Retract:
        case QXmppPubSubEventBase::Purge:
        case QXmppPubSubEventBase::Delete:
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    if (nodeName == ns_mix_node_participants && QXmppPubSubEvent<QXmppMixParticipantItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppMixParticipantItem> event;
        event.parse(element);

        switch (event.eventType()) {
        // Items have been published.
        case QXmppPubSubEventBase::Items: {
            const auto items = event.items();
            for (const auto &item : items) {
                Q_EMIT participantReceived(pubSubService, item);
            }
            break;
        }
        // Specific items are deleted.
        case QXmppPubSubEventBase::Retract: {
            const auto ids = event.retractIds();
            for (const auto &id : ids) {
                Q_EMIT participantLeft(pubSubService, id);
            }
            break;
        }
        // All items are deleted.
        case QXmppPubSubEventBase::Purge:
        // The whole node is deleted.
        case QXmppPubSubEventBase::Delete:
            Q_EMIT channelDeleted(pubSubService);
            break;
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    return false;
}
/// \endcond

///
/// Pepares an IQ stanza for joining a MIX channel.
///
/// \param channelJid JID of the channel being joined
/// \param nickname nickname of the user which is usually required by the server (default: no
///        nickname is set)
/// \param nodes nodes of the channel that are subscribed to for receiving their updates (default:
///        all nodes are subcribed to)
///
/// \return the prepared MIX join IQ stanza
///
QXmppMixIq QXmppMixManager::prepareJoinIq(const QString &channelJid, const QString &nickname, QXmppMixConfigItem::Nodes nodes)
{
    QXmppMixIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(client()->configuration().jidBare());
    iq.setActionType(QXmppMixIq::ClientJoin);
    iq.setChannelJid(channelJid);
    iq.setNick(nickname);
    iq.setSubscriptions(nodes);

    return iq;
}

///
/// Joins a MIX channel.
///
/// \param iq IQ stanza for joining a channel
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JoiningResult> QXmppMixManager::joinChannel(QXmppMixIq &&iq)
{
    return chainIq(client()->sendIq(std::move(iq)), this, [](QXmppMixIq &&iq) -> JoiningResult {
        return Participation { iq.participantId(), iq.nick(), iq.subscriptions() };
    });
}

///
/// Requests all JIDs of a node belonging to a MIX.
///
/// This is only used for nodes storing items with IDs representing JIDs.
///
/// \param channelJid JID of the channel
/// \param node node to be queried
///
/// \return the result of the action
///
QXmppTask<QXmppMixManager::JidResult> QXmppMixManager::requestJids(const QString &channelJid, const QString &node)
{
    return chainMapSuccess(d->pubSubManager->requestItems(channelJid, node), this, [](QXmppPubSubManager::Items<QXmppPubSubBaseItem> &&items) {
        return transform<QVector<Jid>>(items.items, [](const QXmppPubSubBaseItem &item) {
            return item.id();
        });
    });
}

///
/// Adds a JID to a node of a MIX channel.
///
/// This is only used for nodes storing items with IDs representing JIDs.
///
/// \param channelJid JID of the channel
/// \param node node to which the JID is added
/// \param jid JID to be added
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMixManager::addJidToNode(const QString &channelJid, const QString &node, const QString &jid)
{
    return chainSuccess(d->pubSubManager->publishItem(channelJid, node, QXmppPubSubBaseItem { jid }), this);
}

///
/// Handles incoming service infos specified by \xep{0030, Service Discovery}
///
/// \param iq received Service Discovery IQ stanza
///
void QXmppMixManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    // Check the server's functionality to support MIX clients.
    if (iq.from().isEmpty() || iq.from() == client()->configuration().domain()) {
        // Check whether MIX is supported.
        if (iq.features().contains(ns_mix_pam)) {
            setSupportedByServer(true);

            // Check whether MIX archiving is supported.
            if (iq.features().contains(ns_mix_pam_archiving)) {
                setArchivingSupportedByServer(true);
            }
        } else {
            setSupportedByServer(false);
            setArchivingSupportedByServer(false);
        }
    }

    const auto jid = iq.from().isEmpty() ? client()->configuration().domain() : iq.from();

    // If no MIX service is provided by the JID, remove it from the cache.
    if (!iq.features().contains(ns_mix)) {
        removeService(jid);
        return;
    }

    const auto identities = iq.identities();

    // Search for MIX features provided by the determined MIX service.
    for (const QXmppDiscoveryIq::Identity &identity : identities) {
        // ' || identity.type() == u"text"' is a workaround for older ejabberd versions.
        if (identity.category() == u"conference" && (identity.type() == MIX_SERVICE_DISCOVERY_NODE || identity.type() == u"text")) {
            Service service;
            service.jid = iq.from().isEmpty() ? client()->configuration().domain() : iq.from();
            service.channelsSearchable = iq.features().contains(ns_mix_searchable);
            service.channelCreationAllowed = iq.features().contains(ns_mix_create_channel);

            addService(service);
            return;
        }
    }

    removeService(jid);
}

///
/// Sets whether the own server supports MIX.
///
/// \param supportedByServer whether MIX is supported by the own server
///
void QXmppMixManager::setSupportedByServer(bool supportedByServer)
{
    if (d->supportedByServer != supportedByServer) {
        d->supportedByServer = supportedByServer;
        Q_EMIT supportedByServerChanged();
    }
}

///
/// Sets whether the own server supports archiving messages via
/// \xep{0313, Message Archive Management} of MIX channels the user participates in.
///
/// \param archivingSupportedByServer whether MIX messages are archived by the own server
///
void QXmppMixManager::setArchivingSupportedByServer(bool archivingSupportedByServer)
{
    if (d->archivingSupportedByServer != archivingSupportedByServer) {
        d->archivingSupportedByServer = archivingSupportedByServer;
        Q_EMIT archivingSupportedByServerChanged();
    }
}

///
/// Adds a MIX service.
///
/// \param service MIX service
///
void QXmppMixManager::addService(const Service &service)
{
    const auto jidsEqual = [&jid = service.jid](const Service &service) {
        return service.jid == jid;
    };

    auto itr = std::find_if(d->services.begin(), d->services.end(), jidsEqual);

    if (itr == d->services.end()) {
        d->services.append(service);
    } else if (*itr == service) {
        // Do not emit "servicesChanged()" if the service is already cached with the same
        // properties.
        return;
    } else {
        *itr = service;
    }

    Q_EMIT servicesChanged();
}

///
/// Removes a MIX service.
///
/// \param jid JID of the MIX service
///
void QXmppMixManager::removeService(const QString &jid)
{
    const auto jidsEqual = [&jid](const Service &service) {
        return service.jid == jid;
    };

    auto itr = std::find_if(d->services.begin(), d->services.end(), jidsEqual);

    if (itr == d->services.end()) {
        return;
    }

    d->services.erase(itr);

    Q_EMIT servicesChanged();
}

///
/// Removes all MIX services.
///
void QXmppMixManager::removeServices()
{
    if (!d->services.isEmpty()) {
        d->services.clear();
        Q_EMIT servicesChanged();
    }
}

///
/// Resets the cached data.
///
void QXmppMixManager::resetCachedData()
{
    setSupportedByServer(false);
    setArchivingSupportedByServer(false);
    removeServices();
}
