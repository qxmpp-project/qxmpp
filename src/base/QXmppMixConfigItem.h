// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXCONFIGITEM_H
#define QXMPPMIXCONFIGITEM_H

#include "QXmppDataForm.h"
#include "QXmppPubSubBaseItem.h"

class QXmppMixConfigItemPrivate;

class QXMPP_EXPORT QXmppMixConfigItem : public QXmppPubSubBaseItem
{
public:
    enum class Role {
        Owner,
        Administrator,
        Participant,
        Allowed,
        Anyone,
        Nobody,
    };

    enum class Node {
        AllowedJids = 1 << 0,
        AvatarData = 1 << 1,
        AvatarMetadata = 1 << 2,
        BannedJids = 1 << 3,
        Configuration = 1 << 4,
        Information = 1 << 5,
        JidMap = 1 << 6,
        Messages = 1 << 7,
        Participants = 1 << 8,
        Presence = 1 << 9,
    };
    Q_DECLARE_FLAGS(Nodes, Node)

    QXmppMixConfigItem();
    QXmppMixConfigItem(const QXmppMixConfigItem &);
    QXmppMixConfigItem(QXmppMixConfigItem &&);
    ~QXmppMixConfigItem() override;

    QXmppMixConfigItem &operator=(const QXmppMixConfigItem &);
    QXmppMixConfigItem &operator=(QXmppMixConfigItem &&);

    QXmppDataForm::Type formType() const;
    void setFormType(QXmppDataForm::Type formType);

    QString lastEditorJid() const;
    void setLastEditorJid(const QString &lastEditorJid);

    QStringList ownerJids() const;
    void setOwnerJids(const QStringList &ownerJids);

    QStringList administratorJids() const;
    void setAdministratorJids(const QStringList &administratorJids);

    QDateTime channelDeletion() const;
    void setChannelDeletion(const QDateTime &channelDeletion);

    Nodes nodes() const;
    void setNodes(Nodes nodes);

    std::optional<Role> messagesSubscribeRole() const;
    void setMessagesSubscribeRole(std::optional<Role> messagesSubscribeRole);

    std::optional<Role> messagesRetractRole() const;
    void setMessagesRetractRole(std::optional<Role> messagesRetractRole);

    std::optional<Role> presenceSubscribeRole() const;
    void setPresenceSubscribeRole(std::optional<Role> presenceSubscribeRole);

    std::optional<Role> participantsSubscribeRole() const;
    void setParticipantsSubscribeRole(std::optional<Role> participantsSubscribeRole);

    std::optional<Role> informationSubscribeRole() const;
    void setInformationSubscribeRole(std::optional<Role> informationSubscribeRole);

    std::optional<Role> informationUpdateRole() const;
    void setInformationUpdateRole(std::optional<Role> informationUpdateRole);

    std::optional<Role> allowedJidsSubscribeRole() const;
    void setAllowedJidsSubscribeRole(std::optional<Role> allowedJidsSubscribeRole);

    std::optional<Role> bannedJidsSubscribeRole() const;
    void setBannedJidsSubscribeRole(std::optional<Role> bannedJidsSubscribeRole);

    std::optional<Role> configurationReadRole() const;
    void setConfigurationReadRole(std::optional<Role> configurationReadRole);

    std::optional<Role> avatarUpdateRole() const;
    void setAvatarUpdateRole(std::optional<Role> avatarUpdateRole);

    std::optional<bool> nicknameRequired() const;
    void setNicknameRequired(std::optional<bool> nicknameRequired);

    std::optional<bool> presenceRequired() const;
    void setPresenceRequired(std::optional<bool> presenceRequired);

    std::optional<bool> onlyParticipantsPermittedToSubmitPresence() const;
    void setOnlyParticipantsPermittedToSubmitPresence(std::optional<bool> onlyParticipantsPermittedToSubmitPresence);

    std::optional<bool> ownMessageRetractionPermitted() const;
    void setOwnMessageRetractionPermitted(std::optional<bool> ownMessageRetractionPermitted);

    std::optional<bool> invitationsPermitted() const;
    void setInvitationsPermitted(std::optional<bool> invitationsPermitted);

    std::optional<bool> privateMessagesPermitted() const;
    void setPrivateMessagesPermitted(std::optional<bool> privateMessagesPermitted);

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMixConfigItemPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppMixConfigItem::Nodes)
/// \cond
// Scoped enums (enum class) are not implicitly converted to int.
inline auto qHash(QXmppMixConfigItem::Node key, uint seed) noexcept { return qHash(std::underlying_type_t<QXmppMixConfigItem::Node>(key), seed); }
/// \endcond

Q_DECLARE_METATYPE(QXmppMixConfigItem)
Q_DECLARE_METATYPE(QXmppMixConfigItem::Nodes)

#endif  // QXMPPMIXCONFIGITEM_H
