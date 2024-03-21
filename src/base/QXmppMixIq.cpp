// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMixIq.h"

#include "QXmppConstants_p.h"
#include "QXmppMixIq_p.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QSharedData>
#include <QStringBuilder>

using namespace QXmpp::Private;

static const QStringList MIX_ACTION_TYPES = {
    QString(),
    QStringLiteral("client-join"),
    QStringLiteral("client-leave"),
    QStringLiteral("join"),
    QStringLiteral("leave"),
    QStringLiteral("update-subscription"),
    QStringLiteral("setnick"),
    QStringLiteral("create"),
    QStringLiteral("destroy")
};

static const QMap<QXmppMixConfigItem::Node, QStringView> NODES = {
    { QXmppMixConfigItem::Node::AllowedJids, ns_mix_node_allowed },
    { QXmppMixConfigItem::Node::AvatarData, ns_user_avatar_data },
    { QXmppMixConfigItem::Node::AvatarMetadata, ns_user_avatar_metadata },
    { QXmppMixConfigItem::Node::BannedJids, ns_mix_node_banned },
    { QXmppMixConfigItem::Node::Configuration, ns_mix_node_config },
    { QXmppMixConfigItem::Node::Information, ns_mix_node_info },
    { QXmppMixConfigItem::Node::JidMap, ns_mix_node_jidmap },
    { QXmppMixConfigItem::Node::Messages, ns_mix_node_messages },
    { QXmppMixConfigItem::Node::Participants, ns_mix_node_participants },
    { QXmppMixConfigItem::Node::Presence, ns_mix_node_presence },
};

///
/// \class QXmppMixSubscriptionUpdateIq
///
/// This class represents an IQ used to subscribe to nodes and unsubcribe from nodes of a MIX
/// channel as defined by \xep{0369, Mediated Information eXchange (MIX)}.
///
/// \since QXmpp 1.7
///
/// \ingroup Stanzas
///

/// \cond
///
/// Constructs a MIX subscription update IQ.
///
QXmppMixSubscriptionUpdateIq::QXmppMixSubscriptionUpdateIq()
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppMixSubscriptionUpdateIq)

///
/// Returns the nodes to subscribe to.
///
/// \return the nodes being subscribed to
///
QXmppMixConfigItem::Nodes QXmppMixSubscriptionUpdateIq::additions() const
{
    return m_additions;
}

///
/// Sets the nodes to subscribe to.
///
/// \param additions nodes being subscribed to
///
void QXmppMixSubscriptionUpdateIq::setAdditions(QXmppMixConfigItem::Nodes additions)
{
    m_additions = additions;
}

///
/// Returns the nodes to unsubscribe from.
///
/// \return the nodes being unsubscribed from
///
QXmppMixConfigItem::Nodes QXmppMixSubscriptionUpdateIq::removals() const
{
    return m_removals;
}

///
/// Sets the nodes to unsubscribe from.
///
/// \param removals nodes being unsubscribed from
///
void QXmppMixSubscriptionUpdateIq::setRemovals(QXmppMixConfigItem::Nodes removals)
{
    m_removals = removals;
}

bool QXmppMixSubscriptionUpdateIq::isMixSubscriptionUpdateIq(const QDomElement &element)
{
    const QDomElement &child = element.firstChildElement(QStringLiteral("update-subscription"));
    return !child.isNull() && (child.namespaceURI() == ns_mix);
}

void QXmppMixSubscriptionUpdateIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();

    QVector<QString> additions;
    QVector<QString> removals;

    for (const auto &node : iterChildElements(child, u"subscribe")) {
        additions << node.attribute(QStringLiteral("node"));
    }
    for (const auto &node : iterChildElements(child, u"unsubscribe")) {
        removals << node.attribute(QStringLiteral("node"));
    }

    m_additions = listToMixNodes(additions);
    m_removals = listToMixNodes(removals);
}

void QXmppMixSubscriptionUpdateIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("update-subscription"));
    writer->writeDefaultNamespace(toString65(ns_mix));

    const auto additions = mixNodesToList(m_additions);
    for (const auto &addition : additions) {
        writer->writeStartElement(QSL65("subscribe"));
        writer->writeAttribute(QSL65("node"), addition);
        writer->writeEndElement();
    }

    const auto removals = mixNodesToList(m_removals);
    for (const auto &removal : removals) {
        writer->writeStartElement(QSL65("unsubscribe"));
        writer->writeAttribute(QSL65("node"), removal);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppMixInvitationRequestIq
///
/// This class represents an IQ used to request an invitation to a MIX channel as defined by
/// \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}.
///
/// \since QXmpp 1.7
///
/// \ingroup Stanzas
///

/// \cond
///
/// Constructs a MIX invitation request IQ.
///
QXmppMixInvitationRequestIq::QXmppMixInvitationRequestIq()
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppMixInvitationRequestIq)

///
/// Returns the JID of the invitee for whom an invitation is requested from a channel.
///
/// \return the invitee's JID
///
QString QXmppMixInvitationRequestIq::inviteeJid() const
{
    return m_inviteeJid;
}

///
/// Sets the JID of the invitee for whom an invitation is requested from a channel.
///
/// \param inviteeJid invitee's JID
///
void QXmppMixInvitationRequestIq::setInviteeJid(const QString &inviteeJid)
{
    m_inviteeJid = inviteeJid;
}

bool QXmppMixInvitationRequestIq::isMixInvitationRequestIq(const QDomElement &element)
{
    const QDomElement &child = element.firstChildElement(QStringLiteral("invite"));
    return !child.isNull() && (child.namespaceURI() == ns_mix_misc);
}

void QXmppMixInvitationRequestIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    const auto subChild = child.firstChildElement(QStringLiteral("invitee"));
    m_inviteeJid = subChild.text();
}

void QXmppMixInvitationRequestIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("invite"));
    writer->writeDefaultNamespace(toString65(ns_mix_misc));
    writeXmlTextElement(writer, u"invitee", m_inviteeJid);
    writer->writeEndElement();
}
/// \endcond

///
/// \class QXmppMixInvitationResponseIq
///
/// This class represents an IQ that contains a requested invitation to a MIX channel as defined by
/// \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}.
///
/// \since QXmpp 1.7
///
/// \ingroup Stanzas
///

/// \cond
///
/// Constructs a MIX invitation response IQ.
///
QXmppMixInvitationResponseIq::QXmppMixInvitationResponseIq()
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppMixInvitationResponseIq)

///
/// Returns the invitation to a channel.
///
/// \return the channel invitation
///
QXmppMixInvitation QXmppMixInvitationResponseIq::invitation() const
{
    return m_invitation;
}

///
/// Sets the invitation to a channel.
///
/// \param invitation channel invitation
///
void QXmppMixInvitationResponseIq::setInvitation(const QXmppMixInvitation &invitation)
{
    m_invitation = invitation;
}

bool QXmppMixInvitationResponseIq::isMixInvitationResponseIq(const QDomElement &element)
{
    const QDomElement &child = element.firstChildElement(QStringLiteral("invite"));
    return !child.isNull() && (child.namespaceURI() == ns_mix_misc);
}

void QXmppMixInvitationResponseIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    const auto subChild = child.firstChildElement(QStringLiteral("invitation"));
    m_invitation = QXmppMixInvitation();
    m_invitation.parse(subChild);
}

void QXmppMixInvitationResponseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("invite"));
    writer->writeDefaultNamespace(toString65(ns_mix_misc));
    m_invitation.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

class QXmppMixIqPrivate : public QSharedData
{
public:
    QString participantId;
    QString channelId;
    QString channelJid;
    QXmppMixConfigItem::Nodes subscriptions;
    QString nick;
    QXmppMixIq::Type actionType = QXmppMixIq::None;
};

///
/// \class QXmppMixIq
///
/// This class represents an IQ used to do actions on a MIX channel as defined by
/// \xep{0369, Mediated Information eXchange (MIX)} and
/// \xep{0405, Mediated Information eXchange (MIX): Participant Server Requirements}.
///
/// \since QXmpp 1.1
///
/// \ingroup Stanzas
///

///
/// \enum QXmppMixIq::Type
///
/// Action type of the MIX IQ stanza.
///
/// \var QXmppMixIq::None
///
/// Nothing is done.
///
/// \var QXmppMixIq::ClientJoin
///
/// The client sends a request to join a MIX channel to the user's server.
///
/// \var QXmppMixIq::ClientLeave
///
/// The client sends a request to leave a MIX channel to the user's server.
///
/// \var QXmppMixIq::Join
///
/// The user's server forwards a join request from the client to the MIX channel.
///
/// \var QXmppMixIq::Leave
///
/// The user's server forwards a leave request from the client to the MIX channel.
///
/// \var QXmppMixIq::UpdateSubscription
///
/// The client subscribes to MIX nodes or unsubscribes from MIX nodes.
///
/// \deprecated This is deprecated since QXmpp 1.7. Use QXmppMixManager instead.
///
/// \var QXmppMixIq::SetNick
///
/// The client changes the user's nickname within the MIX channel.
///
/// \var QXmppMixIq::Create
///
/// The client creates a MIX channel.
///
/// \var QXmppMixIq::Destroy
///
/// The client destroys a MIX channel.
///

QXmppMixIq::QXmppMixIq()
    : d(new QXmppMixIqPrivate)
{
}

/// Default copy-constructor
QXmppMixIq::QXmppMixIq(const QXmppMixIq &) = default;
/// Default move-constructor
QXmppMixIq::QXmppMixIq(QXmppMixIq &&) = default;
QXmppMixIq::~QXmppMixIq() = default;
/// Default assignment operator
QXmppMixIq &QXmppMixIq::operator=(const QXmppMixIq &) = default;
/// Default move-assignment operator
QXmppMixIq &QXmppMixIq::operator=(QXmppMixIq &&) = default;
///
/// Returns the channel JID, in case of a Join/ClientJoin query result, containing the participant
/// ID.
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::participantId() and
/// QXmppMixIq::channelJid() instead.
///
QString QXmppMixIq::jid() const
{
    if (d->participantId.isEmpty()) {
        return d->channelJid;
    }

    if (d->channelJid.isEmpty()) {
        return {};
    }

    return d->participantId % u'#' % d->channelJid;
}

///
/// Sets the channel JID, in case of a Join/ClientJoin query result, containing the participant ID.
///
/// \param jid channel JID including a possible participant ID
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::setParticipantId() and
/// QXmppMixIq::setChannelJid() instead.
///
void QXmppMixIq::setJid(const QString &jid)
{
    const auto jidParts = jid.split(u'#');

    if (jidParts.size() == 1) {
        d->channelJid = jid;
    } else if (jidParts.size() == 2) {
        d->participantId = jidParts.at(0);
        d->channelJid = jidParts.at(1);
    }
}

///
/// Returns the participant ID for a Join/ClientJoin result.
///
/// \return the participant ID
///
/// \since QXmpp 1.7
///
QString QXmppMixIq::participantId() const
{
    return d->participantId;
}

///
/// Sets the participant ID for a Join/ClientJoin result.
///
/// @param participantId ID of the user in the channel
///
/// \since QXmpp 1.7
///
void QXmppMixIq::setParticipantId(const QString &participantId)
{
    d->participantId = participantId;
}

///
/// Returns the channel's ID (the local part of the channel JID).
///
/// It can be empty if a JID was set.
///
/// \return the ID of the channel
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::channelId() instead.
///
QString QXmppMixIq::channelName() const
{
    return d->channelId;
}

///
/// Sets the channel's ID (the local part of the channel JID) for creating or destroying a channel.
///
/// If you create a new channel, the channel ID can be left empty to let the server generate an ID.
///
/// \param channelName ID of the channel
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::setChannelId()
/// instead.
///
void QXmppMixIq::setChannelName(const QString &channelName)
{
    d->channelId = channelName;
}

///
/// Returns the channel's ID (the local part of the channel JID).
///
/// It can be empty if a JID was set.
///
/// \return the ID of the channel
///
/// \since QXmpp 1.7
///
QString QXmppMixIq::channelId() const
{
    return d->channelId;
}

///
/// Sets the channel's ID (the local part of the channel JID) for creating or destroying a channel.
///
/// If you create a new channel, the channel ID can be left empty to let the server generate an ID.
///
/// @param channelId channel ID to be set
///
/// \since QXmpp 1.7
///
void QXmppMixIq::setChannelId(const QString &channelId)
{
    d->channelId = channelId;
}

///
/// Returns the channel's JID.
///
/// \return the JID of the channel
///
/// \since QXmpp 1.7
///
QString QXmppMixIq::channelJid() const
{
    return d->channelJid;
}

///
/// Sets the channel's JID.
///
/// @param channelJid JID to be set
///
/// \since QXmpp 1.7
///
void QXmppMixIq::setChannelJid(const QString &channelJid)
{
    d->channelJid = channelJid;
}

///
/// Returns the nodes being subscribed to.
///
/// \return the nodes being subscribed to
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::subscriptions() instead.
///
QStringList QXmppMixIq::nodes() const
{
    return mixNodesToList(d->subscriptions).toList();
}

///
/// Sets the nodes being subscribed to.
///
/// \param nodes nodes being subscribed to
///
/// \deprecated This method is deprecated since QXmpp 1.7. Use QXmppMixIq::setSubscriptions()
/// instead.
///
void QXmppMixIq::setNodes(const QStringList &nodes)
{
    d->subscriptions = listToMixNodes(nodes.toVector());
}

///
/// Returns the nodes to subscribe to.
///
/// \return the nodes being subscribed to
///
/// \since QXmpp 1.7
///
QXmppMixConfigItem::Nodes QXmppMixIq::subscriptions() const
{
    return d->subscriptions;
}

///
/// Sets the nodes to subscribe to.
///
/// \param subscriptions nodes being subscribed to
///
/// \since QXmpp 1.7
///
void QXmppMixIq::setSubscriptions(QXmppMixConfigItem::Nodes subscriptions)
{
    d->subscriptions = subscriptions;
}

///
/// Returns the user's nickname in the channel.
///
/// \return the nickname of the user
///
QString QXmppMixIq::nick() const
{
    return d->nick;
}

///
/// Sets the user's nickname used for the channel.
///
/// \param nick nick of the user to be set
///
void QXmppMixIq::setNick(const QString &nick)
{
    d->nick = nick;
}

/// Returns the MIX channel action type.

/// Returns the MIX channel's action type.
///
/// \return the action type of the channel
///
QXmppMixIq::Type QXmppMixIq::actionType() const
{
    return d->actionType;
}

///
/// Sets the MIX channel's action type.
///
/// \param type action type of the channel
///
void QXmppMixIq::setActionType(QXmppMixIq::Type type)
{
    d->actionType = type;
}

/// \cond
bool QXmppMixIq::isMixIq(const QDomElement &element)
{
    const QDomElement &child = element.firstChildElement();
    return !child.isNull() && (child.namespaceURI() == ns_mix || child.namespaceURI() == ns_mix_pam);
}

void QXmppMixIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();

    const auto actionTypeIndex = MIX_ACTION_TYPES.indexOf(child.tagName());
    d->actionType = actionTypeIndex == -1 ? None : (QXmppMixIq::Type)actionTypeIndex;

    if (child.namespaceURI() == ns_mix_pam) {
        if (child.hasAttribute(QStringLiteral("channel"))) {
            d->channelJid = child.attribute(QStringLiteral("channel"));
        }

        child = child.firstChildElement();
    }

    if (!child.isNull() && child.namespaceURI() == ns_mix) {
        if (child.hasAttribute(QStringLiteral("id"))) {
            d->participantId = child.attribute(QStringLiteral("id"));
        }
        if (child.hasAttribute(QStringLiteral("jid"))) {
            d->channelJid = (child.attribute(QStringLiteral("jid"))).split(u'#').last();
        }
        if (child.hasAttribute(QStringLiteral("channel"))) {
            d->channelId = child.attribute(QStringLiteral("channel"));
        }

        d->nick = firstChildElement(child, u"nick").text();

        QVector<QString> subscriptions;

        for (const auto &node : iterChildElements(child, u"subscribe")) {
            subscriptions << node.attribute(QStringLiteral("node"));
        }

        d->subscriptions = listToMixNodes(subscriptions);
    }
}

void QXmppMixIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    if (d->actionType == None) {
        return;
    }

    writer->writeStartElement(MIX_ACTION_TYPES.at(d->actionType));

    if (d->actionType == ClientJoin || d->actionType == ClientLeave) {
        writer->writeDefaultNamespace(toString65(ns_mix_pam));
        if (type() == Set) {
            writeOptionalXmlAttribute(writer, u"channel", d->channelJid);
        }
        if (d->actionType == ClientJoin) {
            writer->writeStartElement(QSL65("join"));
        } else if (d->actionType == ClientLeave) {
            writer->writeStartElement(QSL65("leave"));
        }
    }

    writer->writeDefaultNamespace(toString65(ns_mix));
    writeOptionalXmlAttribute(writer, u"channel", d->channelId);
    if (type() == Result) {
        writeOptionalXmlAttribute(writer, u"id", d->participantId);
    }

    const auto subscriptions = mixNodesToList(d->subscriptions);
    for (const auto &subscription : subscriptions) {
        writer->writeStartElement(QSL65("subscribe"));
        writer->writeAttribute(QSL65("node"), subscription);
        writer->writeEndElement();
    }

    if (!d->nick.isEmpty()) {
        writer->writeTextElement(QSL65("nick"), d->nick);
    }

    writer->writeEndElement();

    if (d->actionType == ClientJoin || d->actionType == ClientLeave) {
        writer->writeEndElement();
    }
}
/// \endcond

namespace QXmpp::Private {

///
/// Converts a nodes flag to a list of nodes.
///
/// \param nodes nodes to convert
///
/// \return the list of nodes
///
QVector<QString> mixNodesToList(QXmppMixConfigItem::Nodes nodes)
{
    QVector<QString> nodeList;

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
QXmppMixConfigItem::Nodes listToMixNodes(const QVector<QString> &nodeList)
{
    QXmppMixConfigItem::Nodes nodes;

    for (auto itr = NODES.cbegin(); itr != NODES.cend(); ++itr) {
        if (nodeList.contains(itr.value().toString())) {
            nodes |= itr.key();
        }
    }

    return nodes;
}

}  // namespace QXmpp::Private
