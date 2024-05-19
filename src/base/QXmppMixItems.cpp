// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppDataFormBase.h"
#include "QXmppMixConfigItem.h"
#include "QXmppMixInfoItem.h"
#include "QXmppMixParticipantItem.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

using namespace QXmpp::Private;

#include <QDateTime>

constexpr QStringView NAME = u"Name";
constexpr QStringView DESCRIPTION = u"Description";
constexpr QStringView CONTACT_JIDS = u"Contact";

constexpr QStringView LAST_EDITOR_JID_KEY = u"Last Change Made By";
constexpr QStringView OWNER_JIDS_KEY = u"Owner";
constexpr QStringView ADMINISTRATOR_JIDS_KEY = u"Administrator";
constexpr QStringView CHANNEL_DELETION_KEY = u"End of Life";
constexpr QStringView NODES_KEY = u"Nodes Present";
constexpr QStringView MESSAGES_SUBSCRIBE_ROLE_KEY = u"Messages Node Subscription";
constexpr QStringView MESSAGES_RETRACT_ROLE_KEY = u"Administrator Message Retraction Rights";
constexpr QStringView PRESENCE_SUBSCRIBE_ROLE_KEY = u"Presence Node Subscription";
constexpr QStringView PARTICIPANTS_SUBSCRIBE_ROLE_KEY = u"Participants Node Subscription";
constexpr QStringView INFORMATION_SUBSCRIBE_ROLE_KEY = u"Information Node Subscription";
constexpr QStringView INFORMATION_UPDATE_ROLE_KEY = u"Information Node Update Rights";
constexpr QStringView ALLOWED_JIDS_SUBSCRIBE_ROLE_KEY = u"Allowed Node Subscription";
constexpr QStringView BANNED_JIDS_SUBSCRIBE_ROLE_KEY = u"Banned Node Subscription";
constexpr QStringView CONFIGURATION_READ_ROLE_KEY = u"Configuration Node Access";
constexpr QStringView AVATARS_UPDATE_ROLE_KEY = u"Avatar Nodes Update Rights";
constexpr QStringView NICKNAME_REQUIRED_KEY = u"Mandatory Nicks";
constexpr QStringView PRESENCE_REQUIRED_KEY = u"Participants Must Provide Presence";
constexpr QStringView ONLY_PARTICIPANTS_PERMITTED_TO_SUBMIT_PRESENCE_KEY = u"Open Presence";
constexpr QStringView OWN_MESSAGE_RETRACTION_PERMITTED_KEY = u"User Message Retraction";
constexpr QStringView INVITATIONS_PERMITTED_KEY = u"Participation Addition by Invitation from Participant";
constexpr QStringView PRIVATE_MESSAGES_PERMITTED_KEY = u"Private Messages";

static const QMap<QXmppMixConfigItem::Role, QStringView> ROLES = {
    { QXmppMixConfigItem::Role::Owner, u"owners" },
    { QXmppMixConfigItem::Role::Administrator, u"admins" },
    { QXmppMixConfigItem::Role::Participant, u"participants" },
    { QXmppMixConfigItem::Role::Allowed, u"allowed" },
    { QXmppMixConfigItem::Role::Anyone, u"anyone" },
    { QXmppMixConfigItem::Role::Nobody, u"nobody" },
};

static const QMap<QXmppMixConfigItem::Node, QStringView> NODES = {
    { QXmppMixConfigItem::Node::AllowedJids, u"allowed" },
    { QXmppMixConfigItem::Node::AvatarData, u"avatar" },
    { QXmppMixConfigItem::Node::AvatarMetadata, u"avatar" },
    { QXmppMixConfigItem::Node::BannedJids, u"banned" },
    { QXmppMixConfigItem::Node::Information, u"information" },
    { QXmppMixConfigItem::Node::JidMap, u"jidmap-visible" },
    { QXmppMixConfigItem::Node::Participants, u"participants" },
    { QXmppMixConfigItem::Node::Presence, u"presence" },
};

class QXmppMixConfigItemPrivate : public QSharedData, public QXmppDataFormBase
{
public:
    QXmppDataForm::Type dataFormType = QXmppDataForm::None;
    QString lastEditorJid;
    QStringList ownerJids;
    QStringList administratorJids;
    QDateTime channelDeletion;
    QXmppMixConfigItem::Nodes nodes;
    std::optional<QXmppMixConfigItem::Role> messagesSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> messagesRetractRole;
    std::optional<QXmppMixConfigItem::Role> presenceSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> participantsSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> informationSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> informationUpdateRole;
    std::optional<QXmppMixConfigItem::Role> allowedJidsSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> bannedJidsSubscribeRole;
    std::optional<QXmppMixConfigItem::Role> configurationReadRole;
    std::optional<QXmppMixConfigItem::Role> avatarUpdateRole;
    std::optional<bool> nicknameRequired;
    std::optional<bool> presenceRequired;
    std::optional<bool> onlyParticipantsPermittedToSubmitPresence;
    std::optional<bool> ownMessageRetractionPermitted;
    std::optional<bool> invitationsPermitted;
    std::optional<bool> privateMessagesPermitted;

    void reset()
    {
        dataFormType = QXmppDataForm::None;
        lastEditorJid.clear();
        ownerJids.clear();
        administratorJids.clear();
        channelDeletion = {};
        nodes = {};
        messagesSubscribeRole = std::nullopt;
        messagesRetractRole = std::nullopt;
        presenceSubscribeRole = std::nullopt;
        participantsSubscribeRole = std::nullopt;
        informationSubscribeRole = std::nullopt;
        informationUpdateRole = std::nullopt;
        allowedJidsSubscribeRole = std::nullopt;
        bannedJidsSubscribeRole = std::nullopt;
        configurationReadRole = std::nullopt;
        avatarUpdateRole = std::nullopt;
        nicknameRequired = std::nullopt;
        presenceRequired = std::nullopt;
        onlyParticipantsPermittedToSubmitPresence = std::nullopt;
        ownMessageRetractionPermitted = std::nullopt;
        invitationsPermitted = std::nullopt;
        privateMessagesPermitted = std::nullopt;
    }

    QString formType() const override
    {
        return ns_mix_admin.toString();
    }

    void parseForm(const QXmppDataForm &form) override
    {
        dataFormType = form.type();
        const auto fields = form.fields();

        for (const auto &field : fields) {
            const auto key = field.key();
            const auto value = field.value();

            if (key == LAST_EDITOR_JID_KEY) {
                lastEditorJid = value.toString();
            } else if (key == OWNER_JIDS_KEY) {
                ownerJids = value.toStringList();
            } else if (key == ADMINISTRATOR_JIDS_KEY) {
                administratorJids = value.toStringList();
            } else if (key == CHANNEL_DELETION_KEY) {
                channelDeletion = value.toDateTime();
            } else if (key == NODES_KEY) {
                nodes = listToNodes(value.toStringList());
            } else if (key == MESSAGES_SUBSCRIBE_ROLE_KEY) {
                messagesSubscribeRole = stringToRole(value.toString());
            } else if (key == MESSAGES_RETRACT_ROLE_KEY) {
                messagesRetractRole = stringToRole(value.toString());
            } else if (key == PRESENCE_SUBSCRIBE_ROLE_KEY) {
                presenceSubscribeRole = stringToRole(value.toString());
            } else if (key == PARTICIPANTS_SUBSCRIBE_ROLE_KEY) {
                participantsSubscribeRole = stringToRole(value.toString());
            } else if (key == INFORMATION_SUBSCRIBE_ROLE_KEY) {
                informationSubscribeRole = stringToRole(value.toString());
            } else if (key == INFORMATION_UPDATE_ROLE_KEY) {
                informationUpdateRole = stringToRole(value.toString());
            } else if (key == ALLOWED_JIDS_SUBSCRIBE_ROLE_KEY) {
                allowedJidsSubscribeRole = stringToRole(value.toString());
            } else if (key == BANNED_JIDS_SUBSCRIBE_ROLE_KEY) {
                bannedJidsSubscribeRole = stringToRole(value.toString());
            } else if (key == CONFIGURATION_READ_ROLE_KEY) {
                configurationReadRole = stringToRole(value.toString());
            } else if (key == AVATARS_UPDATE_ROLE_KEY) {
                avatarUpdateRole = stringToRole(value.toString());
            } else if (key == NICKNAME_REQUIRED_KEY) {
                nicknameRequired = value.toBool();
            } else if (key == PRESENCE_REQUIRED_KEY) {
                presenceRequired = value.toBool();
            } else if (key == ONLY_PARTICIPANTS_PERMITTED_TO_SUBMIT_PRESENCE_KEY) {
                onlyParticipantsPermittedToSubmitPresence = value.toBool();
            } else if (key == OWN_MESSAGE_RETRACTION_PERMITTED_KEY) {
                ownMessageRetractionPermitted = value.toBool();
            } else if (key == INVITATIONS_PERMITTED_KEY) {
                invitationsPermitted = value.toBool();
            } else if (key == PRIVATE_MESSAGES_PERMITTED_KEY) {
                privateMessagesPermitted = value.toBool();
            }
        }
    }

    void serializeForm(QXmppDataForm &form) const override
    {
        form.setType(dataFormType);

        using Type = QXmppDataForm::Field::Type;

        serializeNullable(form, Type::JidSingleField, LAST_EDITOR_JID_KEY.toString(), lastEditorJid);
        serializeEmptyable(form, Type::JidMultiField, OWNER_JIDS_KEY.toString(), ownerJids);
        serializeEmptyable(form, Type::JidMultiField, ADMINISTRATOR_JIDS_KEY.toString(), administratorJids);
        serializeDatetime(form, CHANNEL_DELETION_KEY.toString(), channelDeletion);
        serializeEmptyable(form, Type::ListMultiField, NODES_KEY.toString(), nodesToList(nodes));
        serializeRole(form, MESSAGES_SUBSCRIBE_ROLE_KEY.toString(), messagesSubscribeRole);
        serializeRole(form, MESSAGES_RETRACT_ROLE_KEY.toString(), messagesRetractRole);
        serializeRole(form, PRESENCE_SUBSCRIBE_ROLE_KEY.toString(), presenceSubscribeRole);
        serializeRole(form, PARTICIPANTS_SUBSCRIBE_ROLE_KEY.toString(), participantsSubscribeRole);
        serializeRole(form, INFORMATION_SUBSCRIBE_ROLE_KEY.toString(), informationSubscribeRole);
        serializeRole(form, INFORMATION_UPDATE_ROLE_KEY.toString(), informationUpdateRole);
        serializeRole(form, ALLOWED_JIDS_SUBSCRIBE_ROLE_KEY.toString(), allowedJidsSubscribeRole);
        serializeRole(form, BANNED_JIDS_SUBSCRIBE_ROLE_KEY.toString(), bannedJidsSubscribeRole);
        serializeRole(form, CONFIGURATION_READ_ROLE_KEY.toString(), configurationReadRole);
        serializeRole(form, AVATARS_UPDATE_ROLE_KEY.toString(), avatarUpdateRole);
        serializeOptional(form, Type::ListSingleField, NICKNAME_REQUIRED_KEY.toString(), nicknameRequired);
        serializeOptional(form, Type::ListSingleField, PRESENCE_REQUIRED_KEY.toString(), presenceRequired);
        serializeOptional(form, Type::ListSingleField, ONLY_PARTICIPANTS_PERMITTED_TO_SUBMIT_PRESENCE_KEY.toString(), onlyParticipantsPermittedToSubmitPresence);
        serializeOptional(form, Type::ListSingleField, OWN_MESSAGE_RETRACTION_PERMITTED_KEY.toString(), ownMessageRetractionPermitted);
        serializeOptional(form, Type::ListSingleField, INVITATIONS_PERMITTED_KEY.toString(), invitationsPermitted);
        serializeOptional(form, Type::ListSingleField, PRIVATE_MESSAGES_PERMITTED_KEY.toString(), privateMessagesPermitted);
    }

    ///
    /// Serializes a role to a form field.
    ///
    /// \param form data form
    /// \param name name of the form field
    /// \param role role to serialize
    ///
    static void serializeRole(QXmppDataForm &form, const QString &name, std::optional<QXmppMixConfigItem::Role> role)
    {
        serializeNullable(form, QXmppDataForm::Field::Type::ListSingleField, name, roleToString(role).toString());
    }

    ///
    /// Converts a role to a string.
    ///
    /// \param role role to convert
    ///
    /// \return the string representation of the role
    ///
    static QStringView roleToString(std::optional<QXmppMixConfigItem::Role> role)
    {
        return role ? ROLES.value(*role) : QStringView();
    }

    ///
    /// Converts a string to a role.
    ///
    /// \param roleString string to convert
    ///
    /// \return the role for its string representation
    ///
    static QXmppMixConfigItem::Role stringToRole(QStringView roleString)
    {
        return ROLES.key(roleString);
    }

    ///
    /// Converts a nodes flag to a list of nodes.
    ///
    /// \param nodes nodes to convert
    ///
    /// \return the list of nodes
    ///
    static QStringList nodesToList(QXmppMixConfigItem::Nodes nodes)
    {
        QStringList nodeList;

        for (auto itr = NODES.cbegin(); itr != NODES.cend(); ++itr) {
            if (nodes.testFlag(itr.key())) {
                nodeList.append(itr.value().toString());
            }
        }

        return nodeList;
    }

    ///
    /// Converts a list of nodes to a nodes flag
    ///
    /// \param nodeList list of nodes to convert
    ///
    /// \return the nodes flag
    ///
    static QXmppMixConfigItem::Nodes listToNodes(const QStringList &nodeList)
    {
        QXmppMixConfigItem::Nodes nodes;

        for (auto itr = NODES.cbegin(); itr != NODES.cend(); ++itr) {
            if (nodeList.contains(itr.value())) {
                nodes |= itr.key();
            }
        }

        return nodes;
    }
};

///
/// \class QXmppMixConfigItem
///
/// \brief The QXmppMixConfigItem class represents a PubSub item of a MIX channel containing its
/// configuration as defined by \xep{0369, Mediated Information eXchange (MIX)}.
///
/// \since QXmpp 1.7
///
/// \ingroup Stanzas
///

///
/// \enum QXmppMixConfigItem::Node
///
/// PubSub node belonging to a MIX channel.
///
/// \var QXmppMixConfigItem::Node::AllowedJids
///
/// JIDs allowed to participate in the channel.
///
/// If this node does not exist, all JIDs are allowed to participate in the channel.
///
/// \var QXmppMixConfigItem::Node::AvatarData
///
/// Channel's avatar data.
///
/// \var QXmppMixConfigItem::Node::AvatarMetadata
///
/// Channel's avatar metadata.
///
/// \var QXmppMixConfigItem::Node::BannedJids
///
/// JIDs banned from participating in the channel.
///
/// \var QXmppMixConfigItem::Node::Configuration
///
/// Channel's onfiguration.
///
/// \var QXmppMixConfigItem::Node::Information
///
/// Channel's information.
///
/// \var QXmppMixConfigItem::Node::JidMap
///
/// Mappings from the partipants' IDs to their JIDs.
///
/// This is needed for JID hidden channels.
///
/// \var QXmppMixConfigItem::Node::Messages
///
/// Messages sent through the channel.
///
/// \var QXmppMixConfigItem::Node::Participants
///
/// Users participating in the channel.
///
/// \var QXmppMixConfigItem::Node::Presence
///
/// Presence of users participating in the channel.
///

///
/// \enum QXmppMixConfigItem::Role
///
/// Roles for a MIX channel with various rights.
///
/// The rights are defined in a strictly hierarchical manner following the order of this
/// enumeration, so that for example Owners will always have rights that Administrators have.
///
/// \var QXmppMixConfigItem::Role::Owner
///
/// Allowed to update the channel configuration.
/// Specified by the channel configuration.
///
/// \var QXmppMixConfigItem::Role::Administrator
///
/// Allowed to update the JIDs that are allowed to participate or banned from participating in a
/// channel.
/// Specified in the channel configuration.
///
/// \var QXmppMixConfigItem::Role::Participant
///
/// Participant of the channel.
///
/// \var QXmppMixConfigItem::Role::Allowed
///
/// User that is allowed to participate in the channel.
/// Users are allowed if their JIDs do not match a JID in the node Node::BannedJids and either there
/// is no node Node::AllowedJids or their JIDs match a JID in it.
///
/// \var QXmppMixConfigItem::Role::Anyone
///
/// Any user, including users in the node BannedJids.
///
/// \var QXmppMixConfigItem::Role::Nobody
///
/// No user, including owners and administrators.
///

QXmppMixConfigItem::QXmppMixConfigItem()
    : d(new QXmppMixConfigItemPrivate)
{
}

/// Default copy-constructor
QXmppMixConfigItem::QXmppMixConfigItem(const QXmppMixConfigItem &) = default;
/// Default move-constructor
QXmppMixConfigItem::QXmppMixConfigItem(QXmppMixConfigItem &&) = default;
/// Default assignment operator
QXmppMixConfigItem &QXmppMixConfigItem::operator=(const QXmppMixConfigItem &) = default;
/// Default move-assignment operator
QXmppMixConfigItem &QXmppMixConfigItem::operator=(QXmppMixConfigItem &&) = default;
QXmppMixConfigItem::~QXmppMixConfigItem() = default;

///
/// Returns the type of the data form that contains the channel's configuration.
///
/// \return the data form's type
///
QXmppDataForm::Type QXmppMixConfigItem::formType() const
{
    return d->dataFormType;
}

///
/// Sets the type of the data form that contains the channel's configuration.
///
/// \param formType data form's type
///
void QXmppMixConfigItem::setFormType(QXmppDataForm::Type formType)
{
    d->dataFormType = formType;
}

///
/// Returns the bare JID of the user that made the latest change to the channel's configuration.
///
/// The JID is set by the server on each configuration change.
///
/// \return the JID of the last editor
///
QString QXmppMixConfigItem::lastEditorJid() const
{
    return d->lastEditorJid;
}

///
/// Sets the bare JID of the user that made the latest change to the channel's configuration.
///
/// \see lastEditorJid()
///
/// \param lastEditorJid last editor JID
///
void QXmppMixConfigItem::setLastEditorJid(const QString &lastEditorJid)
{
    d->lastEditorJid = lastEditorJid;
}

///
/// Returns the bare JIDs of the channel owners.
///
/// When a channel is created, the JID of the user that created it is set as the first owner.
///
/// \see Role::Owner
///
/// \return the JIDs of the owners
///
QStringList QXmppMixConfigItem::ownerJids() const
{
    return d->ownerJids;
}

///
/// Sets the bare JIDs of the channel owners.
///
/// \see ownerJids()
///
/// \param ownerJids JIDs of the owners
///
void QXmppMixConfigItem::setOwnerJids(const QStringList &ownerJids)
{
    d->ownerJids = ownerJids;
}

///
/// Returns the bare JIDs of the channel administrators.
///
/// \see Role::Administrator
///
/// \return the JIDs of the administrators
///
QStringList QXmppMixConfigItem::administratorJids() const
{
    return d->administratorJids;
}

///
/// Sets the bare JIDs of the channel administrators.
///
/// \see administratorJids()
///
/// \param administratorJids JIDs of the administrators
///
void QXmppMixConfigItem::setAdministratorJids(const QStringList &administratorJids)
{
    d->administratorJids = administratorJids;
}

///
/// Returns the date and time when the channel is automatically deleted.
///
/// If no date/time is set, the channel is permanent.
///
/// \return the channel deletion date/time
///
QDateTime QXmppMixConfigItem::channelDeletion() const
{
    return d->channelDeletion;
}

///
/// Sets the date and time when the channel is automatically deleted.
///
/// \see channelDeletion()
///
/// \param channelDeletion channel deletion date/time
///
void QXmppMixConfigItem::setChannelDeletion(const QDateTime &channelDeletion)
{
    d->channelDeletion = channelDeletion;
}

///
/// Returns which nodes are present for the channel.
///
/// \return the present nodes
///
QXmppMixConfigItem::Nodes QXmppMixConfigItem::nodes() const
{
    return d->nodes;
}

///
/// Sets which nodes are present for the channel.
///
/// \param nodes present nodes
///
void QXmppMixConfigItem::setNodes(Nodes nodes)
{
    d->nodes = nodes;
}

///
/// Returns the role that is permitted to subscribe to messages sent through the channel.
///
/// \return the role permitted to subscribe to the messages
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::messagesSubscribeRole() const
{
    return d->messagesSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to messages sent through the channel.
///
/// Only the following roles are valid:
///     * Role::Participant
///     * Role::Allowed
///     * Role::Anyone
///
/// \param messagesSubscribeRole role permitted to subscribe to the messages
///
void QXmppMixConfigItem::setMessagesSubscribeRole(std::optional<Role> messagesSubscribeRole)
{
    d->messagesSubscribeRole = messagesSubscribeRole;
}

///
/// Returns the role that is permitted to retract any message sent through the channel.
///
/// \return the role permitted to retract any message
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::messagesRetractRole() const
{
    return d->messagesRetractRole;
}

///
/// Sets the role that is permitted to retract any message sent through the channel.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Nobody
///
/// \param messagesRetractRole role permitted to retract any message
///
void QXmppMixConfigItem::setMessagesRetractRole(std::optional<Role> messagesRetractRole)
{
    d->messagesRetractRole = messagesRetractRole;
}

///
/// Returns the role that is permitted to subscribe to the channel's user' presence.
///
/// \return the role permitted to subscribe to the presence
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::presenceSubscribeRole() const
{
    return d->presenceSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to the channel's users' presence.
///
/// Only the following roles are valid:
///     * Role::Participant
///     * Role::Allowed
///     * Role::Anyone
///
/// \param presenceSubscribeRole role permitted to subscribe to the presence
///
void QXmppMixConfigItem::setPresenceSubscribeRole(std::optional<Role> presenceSubscribeRole)
{
    d->presenceSubscribeRole = presenceSubscribeRole;
}

///
/// Returns the role that is permitted to subscribe to the channel's participants.
///
/// \return the role permitted to subscribe to the participants
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::participantsSubscribeRole() const
{
    return d->participantsSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to the channel's participants.
///
/// \param participantsSubscribeRole role permitted to subscribe to the participants
///
void QXmppMixConfigItem::setParticipantsSubscribeRole(std::optional<Role> participantsSubscribeRole)
{
    d->participantsSubscribeRole = participantsSubscribeRole;
}

///
/// Returns the role that is permitted to subscribe to the channel's information.
///
/// \return the role permitted to subscribe to the information
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::informationSubscribeRole() const
{
    return d->informationSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to the channel's information.
///
/// Only the following roles are valid:
///     * Role::Participant
///     * Role::Allowed
///     * Role::Anyone
///
/// \param informationSubscribeRole role permitted to subscribe to the information
///
void QXmppMixConfigItem::setInformationSubscribeRole(std::optional<Role> informationSubscribeRole)
{
    d->informationSubscribeRole = informationSubscribeRole;
}

///
/// Returns the role that is permitted to update the channel's information.
///
/// \return the role permitted to update the information
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::informationUpdateRole() const
{
    return d->informationUpdateRole;
}

///
/// Sets the role that is permitted to update the channel's information.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Participant
///
/// \param informationUpdateRole role permitted to update the information
///
void QXmppMixConfigItem::setInformationUpdateRole(std::optional<Role> informationUpdateRole)
{
    d->informationUpdateRole = informationUpdateRole;
}

///
/// Returns the role that is permitted to subscribe to the JIDs that are allowed to participate in
/// the channel.
///
/// \return the role permitted to subscribe to the allowed JIDs
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::allowedJidsSubscribeRole() const
{
    return d->allowedJidsSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to the JIDs that are allowed to participate in the
/// channel.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Participant
///     * Role::Allowed
///     * Role::Nobody
///
/// \param allowedJidsSubscribeRole role permitted to subscribe to the allowed JIDs
///
void QXmppMixConfigItem::setAllowedJidsSubscribeRole(std::optional<Role> allowedJidsSubscribeRole)
{
    d->allowedJidsSubscribeRole = allowedJidsSubscribeRole;
}

///
/// Returns the role that is permitted to subscribe to the JIDs that are banned from participating
/// in the channel.
///
/// \return the role permitted to subscribe to the banned JIDs
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::bannedJidsSubscribeRole() const
{
    return d->bannedJidsSubscribeRole;
}

///
/// Sets the role that is permitted to subscribe to the JIDs that are banned from participating in
/// the channel.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Participant
///     * Role::Allowed
///     * Role::Nobody
///
/// \param bannedJidsSubscribeRole role permitted to subscribe to the banned JIDs
///
void QXmppMixConfigItem::setBannedJidsSubscribeRole(std::optional<Role> bannedJidsSubscribeRole)
{
    d->bannedJidsSubscribeRole = bannedJidsSubscribeRole;
}

///
/// Returns the role that is permitted to subscribe to and read the channel's configuration.
///
/// \return the role permitted to subscribe to and read the configuration
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::configurationReadRole() const
{
    return d->configurationReadRole;
}

///
/// Sets the role that is permitted to subscribe to and read the channel's configuration.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Participant
///     * Role::Allowed
///     * Role::Nobody
///
/// \param configurationReadRole role permitted to subscribe to and read the configuration
///
void QXmppMixConfigItem::setConfigurationReadRole(std::optional<Role> configurationReadRole)
{
    d->configurationReadRole = configurationReadRole;
}

///
/// Returns the role that is permitted to update the channel's avatar.
///
/// \return the role permitted to update the avatar
///
std::optional<QXmppMixConfigItem::Role> QXmppMixConfigItem::avatarUpdateRole() const
{
    return d->avatarUpdateRole;
}

///
/// Sets the role that is permitted to update the channel's avatar.
///
/// Only the following roles are valid:
///     * Role::Owner
///     * Role::Administrator
///     * Role::Participant
///
/// \param avatarUpdateRole role permitted to update the avatar
///
void QXmppMixConfigItem::setAvatarUpdateRole(std::optional<Role> avatarUpdateRole)
{
    d->avatarUpdateRole = avatarUpdateRole;
}

///
/// Returns whether participants need nicknames.
///
/// \return whether nicknames are required
///
std::optional<bool> QXmppMixConfigItem::nicknameRequired() const
{
    return d->nicknameRequired;
}

///
/// Sets whether participants need nicknames.
///
/// \param nicknameRequired whether nicknames are required
///
void QXmppMixConfigItem::setNicknameRequired(std::optional<bool> nicknameRequired)
{
    d->nicknameRequired = nicknameRequired;
}

///
/// Returns whether participants need to share their presence.
///
/// \return whether presence is required
///
std::optional<bool> QXmppMixConfigItem::presenceRequired() const
{
    return d->presenceRequired;
}

///
/// Sets whether participants need to share their presence.
///
/// \param presenceRequired whether presence is required
///
void QXmppMixConfigItem::setPresenceRequired(std::optional<bool> presenceRequired)
{
    d->presenceRequired = presenceRequired;
}

///
/// Returns whether only participants are permitted to share their presence.
///
/// \return whether only participants are permitted to share their presence
///
std::optional<bool> QXmppMixConfigItem::onlyParticipantsPermittedToSubmitPresence() const
{
    return d->onlyParticipantsPermittedToSubmitPresence;
}

///
/// Sets whether only participants are permitted to share their presence.
///
/// \param onlyParticipantsPermittedToSubmitPresence whether only participants are permitted to
///        share their presence
///
void QXmppMixConfigItem::setOnlyParticipantsPermittedToSubmitPresence(std::optional<bool> onlyParticipantsPermittedToSubmitPresence)
{
    d->onlyParticipantsPermittedToSubmitPresence = onlyParticipantsPermittedToSubmitPresence;
}

///
/// Returns whether users are permitted to retract their own messages sent through the channel.
///
/// \return whether users are permitted to retract their own messages
///
std::optional<bool> QXmppMixConfigItem::ownMessageRetractionPermitted() const
{
    return d->ownMessageRetractionPermitted;
}

///
/// Sets whether users are permitted to retract their own messages sent through the channel.
///
/// \param ownMessageRetractionPermitted whether users are permitted to retract their own messages
///
void QXmppMixConfigItem::setOwnMessageRetractionPermitted(std::optional<bool> ownMessageRetractionPermitted)
{
    d->ownMessageRetractionPermitted = ownMessageRetractionPermitted;
}

///
/// Returns whether participants are permitted to invite users to the channel.
///
/// In order to use that feature, the participant must request the invitation from the channel and
/// send it to the invitee.
/// The invitee can use the invitation to join the channel.
///
/// \sa QXmppMixInvitation
///
/// \return whether channel participants are permitted to invite users
///
std::optional<bool> QXmppMixConfigItem::invitationsPermitted() const
{
    return d->invitationsPermitted;
}

///
/// Sets whether participants are permitted to invite users to the channel.
///
/// \see invitationsPermitted()
///
/// \param invitationsPermitted whether participants are permitted to invite users
///
void QXmppMixConfigItem::setInvitationsPermitted(std::optional<bool> invitationsPermitted)
{
    d->invitationsPermitted = invitationsPermitted;
}

///
/// Returns whether participants are permitted to exchange private messages through the channel.
///
/// \return whether participants are permitted to exchange private messages
///
std::optional<bool> QXmppMixConfigItem::privateMessagesPermitted() const
{
    return d->privateMessagesPermitted;
}

///
/// Sets whether participants are permitted to exchange private messages through the channel.
///
/// \param privateMessagesPermitted whether participants are permitted to exchange private messages
///
void QXmppMixConfigItem::setPrivateMessagesPermitted(std::optional<bool> privateMessagesPermitted)
{
    d->privateMessagesPermitted = privateMessagesPermitted;
}

///
/// Returns true if the given DOM element is a MIX channel config item.
///
bool QXmppMixConfigItem::isItem(const QDomElement &element)
{
    return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
        // Check FORM_TYPE without parsing a full QXmppDataForm.
        if (payload.tagName() != u'x' || payload.namespaceURI() != ns_data) {
            return false;
        }
        for (auto fieldEl = payload.firstChildElement();
             !fieldEl.isNull();
             fieldEl = fieldEl.nextSiblingElement()) {
            if (fieldEl.attribute(u"var"_s) == u"FORM_TYPE") {
                return fieldEl.firstChildElement(u"value"_s).text() == ns_mix_admin;
            }
        }
        return false;
    });
}

/// \cond
void QXmppMixConfigItem::parsePayload(const QDomElement &payload)
{
    d->reset();

    QXmppDataForm form;
    form.parse(payload);

    d->parseForm(form);
}

void QXmppMixConfigItem::serializePayload(QXmlStreamWriter *writer) const
{
    d->toDataForm().toXml(writer);
}
/// \endcond

class QXmppMixInfoItemPrivate : public QSharedData, public QXmppDataFormBase
{
public:
    QXmppDataForm::Type dataFormType = QXmppDataForm::None;
    QString name;
    QString description;
    QStringList contactJids;

    void reset()
    {
        dataFormType = QXmppDataForm::None;
        name.clear();
        description.clear();
        contactJids.clear();
    }

    QString formType() const override
    {
        return ns_mix.toString();
    }

    void parseForm(const QXmppDataForm &form) override
    {
        dataFormType = form.type();
        const auto fields = form.fields();

        for (const auto &field : fields) {
            const auto key = field.key();
            const auto value = field.value();

            if (key == NAME) {
                name = value.toString();
            } else if (key == DESCRIPTION) {
                description = value.toString();
            } else if (key == CONTACT_JIDS) {
                contactJids = value.toStringList();
            }
        }
    }

    void serializeForm(QXmppDataForm &form) const override
    {
        form.setType(dataFormType);

        using Type = QXmppDataForm::Field::Type;
        serializeNullable(form, Type::TextSingleField, NAME.toString(), name);
        serializeNullable(form, Type::TextSingleField, DESCRIPTION.toString(), description);
        serializeEmptyable(form, Type::JidMultiField, CONTACT_JIDS.toString(), contactJids);
    }
};

///
/// \class QXmppMixInfoItem
///
/// \brief The QXmppMixInfoItem class represents a PubSub item of a MIX
/// channel containing channel information as defined by \xep{0369, Mediated
/// Information eXchange (MIX)}.
///
/// \since QXmpp 1.5
///
/// \ingroup Stanzas
///

QXmppMixInfoItem::QXmppMixInfoItem()
    : d(new QXmppMixInfoItemPrivate)
{
}

/// Default copy-constructor
QXmppMixInfoItem::QXmppMixInfoItem(const QXmppMixInfoItem &) = default;
/// Default move-constructor
QXmppMixInfoItem::QXmppMixInfoItem(QXmppMixInfoItem &&) = default;
/// Default assignment operator
QXmppMixInfoItem &QXmppMixInfoItem::operator=(const QXmppMixInfoItem &) = default;
/// Default move-assignment operator
QXmppMixInfoItem &QXmppMixInfoItem::operator=(QXmppMixInfoItem &&) = default;
QXmppMixInfoItem::~QXmppMixInfoItem() = default;

///
/// Returns the type of the data form that contains the channel information.
///
/// \return the data form's type
///
QXmppDataForm::Type QXmppMixInfoItem::formType() const
{
    return d->dataFormType;
}

///
/// Sets the type of the data form that contains the channel information.
///
/// \param formType data form's type
///
void QXmppMixInfoItem::setFormType(QXmppDataForm::Type formType)
{
    d->dataFormType = formType;
}

///
/// Returns the user-specified name of the MIX channel. This is not the name
/// part of the channel's JID.
///
const QString &QXmppMixInfoItem::name() const
{
    return d->name;
}

///
/// Sets the name of the channel.
///
void QXmppMixInfoItem::setName(QString name)
{
    d->name = std::move(name);
}

///
/// Returns the description of the channel. This string might be very long.
///
const QString &QXmppMixInfoItem::description() const
{
    return d->description;
}

///
/// Sets the longer channel description.
///
void QXmppMixInfoItem::setDescription(QString description)
{
    d->description = std::move(description);
}

///
/// Returns a list of JIDs that are responsible for this channel.
///
const QStringList &QXmppMixInfoItem::contactJids() const
{
    return d->contactJids;
}

///
/// Sets a list of public JIDs that are responsible for this channel.
///
void QXmppMixInfoItem::setContactJids(QStringList contactJids)
{
    d->contactJids = std::move(contactJids);
}

///
/// Returns true, if the given dom element is a MIX channel info item.
///
bool QXmppMixInfoItem::isItem(const QDomElement &element)
{
    return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
        // check FORM_TYPE without parsing a full QXmppDataForm
        if (payload.tagName() != u'x' || payload.namespaceURI() != ns_data) {
            return false;
        }
        for (const auto &fieldEl : iterChildElements(payload)) {
            if (fieldEl.attribute(u"var"_s) == u"FORM_TYPE") {
                return fieldEl.firstChildElement(u"value"_s).text() == ns_mix;
            }
        }
        return false;
    });
}

/// \cond
void QXmppMixInfoItem::parsePayload(const QDomElement &payload)
{
    d->reset();

    QXmppDataForm form;
    form.parse(payload);

    d->parseForm(form);
}

void QXmppMixInfoItem::serializePayload(QXmlStreamWriter *writer) const
{
    d->toDataForm().toXml(writer);
}
/// \endcond

class QXmppMixParticipantItemPrivate : public QSharedData
{
public:
    QString nick;
    QString jid;
};

///
/// \class QXmppMixParticipantItem
///
/// The QXmppMixParticipantItem class represents a PubSub item of a MIX channel
/// participant as defined by \xep{0369, Mediated Information eXchange (MIX)}.
///
/// \since QXmpp 1.5
///
/// \ingroup Stanzas
///

QXmppMixParticipantItem::QXmppMixParticipantItem()
    : d(new QXmppMixParticipantItemPrivate)
{
}

/// Default copy-constructor
QXmppMixParticipantItem::QXmppMixParticipantItem(const QXmppMixParticipantItem &) = default;
/// Default move-constructor
QXmppMixParticipantItem::QXmppMixParticipantItem(QXmppMixParticipantItem &&) = default;
/// Default assignment operator
QXmppMixParticipantItem &QXmppMixParticipantItem::operator=(const QXmppMixParticipantItem &) = default;
/// Default move-assignment operator
QXmppMixParticipantItem &QXmppMixParticipantItem::operator=(QXmppMixParticipantItem &&) = default;
QXmppMixParticipantItem::~QXmppMixParticipantItem() = default;

///
/// Returns the participant's nickname.
///
const QString &QXmppMixParticipantItem::nick() const
{
    return d->nick;
}

///
/// Sets the participants nickname.
///
void QXmppMixParticipantItem::setNick(QString nick)
{
    d->nick = std::move(nick);
}

///
/// Returns the participant's JID.
///
const QString &QXmppMixParticipantItem::jid() const
{
    return d->jid;
}

///
/// Sets the participant's JID.
///
void QXmppMixParticipantItem::setJid(QString jid)
{
    d->jid = std::move(jid);
}

/// \cond
void QXmppMixParticipantItem::parsePayload(const QDomElement &payload)
{
    d->nick = payload.firstChildElement(u"nick"_s).text();
    d->jid = payload.firstChildElement(u"jid"_s).text();
}

void QXmppMixParticipantItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("participant"));
    writer->writeDefaultNamespace(toString65(ns_mix));
    if (!d->jid.isEmpty()) {
        writer->writeTextElement(QSL65("jid"), d->jid);
    }
    if (!d->nick.isEmpty()) {
        writer->writeTextElement(QSL65("nick"), d->nick);
    }
    writer->writeEndElement();
}
/// \endcond

///
/// Returns true, if this dom element is a MIX participant item.
///
bool QXmppMixParticipantItem::isItem(const QDomElement &element)
{
    return QXmppPubSubBaseItem::isItem(element, [](const QDomElement &payload) {
        return payload.tagName() == u"participant" &&
            payload.namespaceURI() == ns_mix;
    });
}
