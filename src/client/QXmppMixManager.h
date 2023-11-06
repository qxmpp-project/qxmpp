// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXMANAGER_H
#define QXMPPMIXMANAGER_H

#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppMixConfigItem.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixParticipantItem.h"
#include "QXmppPubSubEventHandler.h"

class QXmppMixIq;
class QXmppMixInvitation;
class QXmppMixManagerPrivate;

class QXMPP_EXPORT QXmppMixManager : public QXmppClientExtension, public QXmppPubSubEventHandler
{
    Q_OBJECT
    Q_PROPERTY(bool supportedByServer READ supportedByServer NOTIFY supportedByServerChanged)
    Q_PROPERTY(bool archivingSupportedByServer READ archivingSupportedByServer NOTIFY archivingSupportedByServerChanged)
    Q_PROPERTY(QList<Service> services READ services NOTIFY servicesChanged)

public:
    struct QXMPP_EXPORT Service {
        QString jid;
        bool channelsSearchable = false;
        bool channelCreationAllowed = false;

        /// \cond
        bool operator==(const Service &other) const;
        /// \endcond
    };

    struct Subscription {
        QXmppMixConfigItem::Nodes additions;
        QXmppMixConfigItem::Nodes removals;
    };

    struct Participation {
        QString participantId;
        QString nickname;
        QXmppMixConfigItem::Nodes subscriptions;
    };

    using Jid = QString;
    using ChannelJid = QString;
    using Nickname = QString;

    using CreationResult = std::variant<ChannelJid, QXmppError>;
    using ChannelJidResult = std::variant<QVector<ChannelJid>, QXmppError>;
    using ChannelNodeResult = std::variant<QXmppMixConfigItem::Nodes, QXmppError>;
    using ConfigurationResult = std::variant<QXmppMixConfigItem, QXmppError>;
    using InformationResult = std::variant<QXmppMixInfoItem, QXmppError>;
    using JoiningResult = std::variant<Participation, QXmppError>;
    using NicknameResult = std::variant<Nickname, QXmppError>;
    using InvitationResult = std::variant<QXmppMixInvitation, QXmppError>;
    using SubscriptionResult = std::variant<Subscription, QXmppError>;
    using JidResult = std::variant<QVector<Jid>, QXmppError>;
    using ParticipantResult = std::variant<QVector<QXmppMixParticipantItem>, QXmppError>;

    QXmppMixManager();
    ~QXmppMixManager() override;

    QStringList discoveryFeatures() const override;

    bool supportedByServer() const;
    Q_SIGNAL void supportedByServerChanged();

    bool archivingSupportedByServer() const;
    Q_SIGNAL void archivingSupportedByServerChanged();

    QList<Service> services() const;
    Q_SIGNAL void servicesChanged();

    QXmppTask<CreationResult> createChannel(const QString &serviceJid, const QString &channelId = {});

    QXmppTask<ChannelJidResult> requestChannelJids(const QString &serviceJid);
    QXmppTask<ChannelNodeResult> requestChannelNodes(const QString &channelJid);

    QXmppTask<ConfigurationResult> requestChannelConfiguration(const QString &channelJid);
    QXmppTask<QXmppClient::EmptyResult> updateChannelConfiguration(const QString &channelJid, QXmppMixConfigItem configuration);
    Q_SIGNAL void channelConfigurationUpdated(const QString &channelJid, const QXmppMixConfigItem &configuration);

    QXmppTask<InformationResult> requestChannelInformation(const QString &channelJid);
    QXmppTask<QXmppClient::EmptyResult> updateChannelInformation(const QString &channelJid, QXmppMixInfoItem information);
    Q_SIGNAL void channelInformationUpdated(const QString &channelJid, const QXmppMixInfoItem &information);

    QXmppTask<JoiningResult> joinChannel(const QString &channelJid, const QString &nickname = {}, QXmppMixConfigItem::Nodes nodes = ~QXmppMixConfigItem::Nodes());
    QXmppTask<JoiningResult> joinChannel(const QXmppMixInvitation &invitation, const QString &nickname = {}, QXmppMixConfigItem::Nodes nodes = ~QXmppMixConfigItem::Nodes());

    QXmppTask<NicknameResult> updateNickname(const QString &channelJid, const QString &nickname);
    QXmppTask<SubscriptionResult> updateSubscriptions(const QString &channelJid, QXmppMixConfigItem::Nodes subscriptionAdditions = ~QXmppMixConfigItem::Nodes(), QXmppMixConfigItem::Nodes subscriptionRemovals = ~QXmppMixConfigItem::Nodes());

    QXmppTask<InvitationResult> requestInvitation(const QString &channelJid, const QString &inviteeJid);

    QXmppTask<JidResult> requestAllowedJids(const QString &channelJid);
    QXmppTask<QXmppClient::EmptyResult> allowJid(const QString &channelJid, const QString &jid);
    Q_SIGNAL void jidAllowed(const QString &channelJid, const QString &jid);
    Q_SIGNAL void allJidsAllowed(const QString &channelJid);

    QXmppTask<QXmppClient::EmptyResult> disallowJid(const QString &channelJid, const QString &jid);
    Q_SIGNAL void jidDisallowed(const QString &channelJid, const QString &jid);
    QXmppTask<QXmppClient::EmptyResult> disallowAllJids(const QString &channelJid);
    Q_SIGNAL void allJidsDisallowed(const QString &channelJid);

    QXmppTask<JidResult> requestBannedJids(const QString &channelJid);
    QXmppTask<QXmppClient::EmptyResult> banJid(const QString &channelJid, const QString &jid);
    Q_SIGNAL void jidBanned(const QString &channelJid, const QString &jid);

    QXmppTask<QXmppClient::EmptyResult> unbanJid(const QString &channelJid, const QString &jid);
    Q_SIGNAL void jidUnbanned(const QString &channelJid, const QString &jid);
    QXmppTask<QXmppClient::EmptyResult> unbanAllJids(const QString &channelJid);
    Q_SIGNAL void allJidsUnbanned(const QString &channelJid);

    QXmppTask<ParticipantResult> requestParticipants(const QString &channelJid);
    Q_SIGNAL void participantReceived(const QString &channelJid, const QXmppMixParticipantItem &participant);
    Q_SIGNAL void participantLeft(const QString &channelJid, const QString &participantId);

    QXmppTask<QXmppClient::EmptyResult> leaveChannel(const QString &channelJid);

    QXmppTask<QXmppClient::EmptyResult> deleteChannel(const QString &channelJid);
    Q_SIGNAL void channelDeleted(const QString &channelJid);

protected:
    /// \cond
    void onRegistered(QXmppClient *client) override;
    void onUnregistered(QXmppClient *client) override;
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond

private:
    QXmppMixIq prepareJoinIq(const QString &channelJid, const QString &nickname, QXmppMixConfigItem::Nodes nodes);
    QXmppTask<JoiningResult> joinChannel(QXmppMixIq &&iq);
    QXmppTask<JidResult> requestJids(const QString &channelJid, const QString &node);
    QXmppTask<QXmppClient::EmptyResult> addJidToNode(const QString &channelJid, const QString &node, const QString &jid);

    void handleDiscoInfo(const QXmppDiscoveryIq &iq);

    void setSupportedByServer(bool supportedByServer);
    void setArchivingSupportedByServer(bool archivingSupportedByServer);
    void addService(const Service &service);
    void removeService(const QString &jid);
    void removeServices();
    void resetCachedData();

    const std::unique_ptr<QXmppMixManagerPrivate> d;

    friend class tst_QXmppMixManager;
};

#endif  // QXMPPMIXMANAGER_H
