/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn <lnj@kaidan.im>
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppMixIq.h"
#include "QXmppDataForm.h"
#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include <QDomElement>

static const QStringList MIX_ACTION_TYPES = QStringList() << ""
    << "client-join" << "client-leave" << "join" << "leave"
    << "update-subscription" << "setnick" << "create" << "destroy";

/// Returns the channel JID. It also contains a participant id for Join/
/// ClientJoin results.

QString QXmppMixIq::jid() const
{
    return m_jid;
}

/// Sets the channel JID. For results of Join/ClientJoin queries this also
/// needs to contain a participant id.

void QXmppMixIq::setJid(const QString& jid)
{
    m_jid = jid;
}

/// Returns the channel name (the name part of the channel JID). This may still
/// be empty, if a JID was set.

QString QXmppMixIq::channelName() const
{
    return m_channelName;
}

/// Sets the channel name for creating/destroying specific channels. When you
/// create a new channel, this can also be left empty to let the server
/// generate a name.

void QXmppMixIq::setChannelName(const QString& channelName)
{
    m_channelName = channelName;
}

/// Returns the list of nodes to subscribe to.

QStringList QXmppMixIq::nodes() const
{
    return m_nodes;
}

/// Sets the nodes to subscribe to. Note that for UpdateSubscription queries
/// you only need to include the new subscriptions.

void QXmppMixIq::setNodes(const QStringList& nodes)
{
    m_nodes = nodes;
}

/// Returns the user's nickname in the channel.

QString QXmppMixIq::nick() const
{
    return m_nick;
}

/// Sets the nickname for the channel.

void QXmppMixIq::setNick(const QString& nick)
{
    m_nick = nick;
}

/// Returns the MIX channel action type.

QXmppMixIq::Type QXmppMixIq::actionType() const
{
    return m_actionType;
}

/// Sets the channel action.

void QXmppMixIq::setActionType(QXmppMixIq::Type type)
{
    m_actionType = type;
}

/// \cond
bool QXmppMixIq::isMixIq(const QDomElement& element)
{
    const QDomElement& child = element.firstChildElement();
    return !child.isNull() && (child.namespaceURI() == ns_mix
           || child.namespaceURI() == ns_mix_pam);
}

void QXmppMixIq::parseElementFromChild(const QDomElement& element)
{
    QDomElement child = element.firstChildElement();
    // determine action type
    m_actionType = (QXmppMixIq::Type) MIX_ACTION_TYPES.indexOf(child.tagName());

    if (child.namespaceURI() == ns_mix_pam) {
        if (child.hasAttribute("channel"))
            m_jid = child.attribute("channel");

        child = child.firstChildElement();
    }

    if (!child.isNull() && child.namespaceURI() == ns_mix) {
        if (child.hasAttribute("jid"))
            m_jid = child.attribute("jid");
        if (child.hasAttribute("channel"))
            m_channelName = child.attribute("channel");

        QDomElement subChild = child.firstChildElement();
        while (!subChild.isNull()) {
            if (subChild.tagName() == "subscribe")
                m_nodes << subChild.attribute("node");
            else if (subChild.tagName() == "nick")
                m_nick = subChild.text();

            subChild = subChild.nextSiblingElement();
        }
    }
}

void QXmppMixIq::toXmlElementFromChild(QXmlStreamWriter* writer) const
{
    if (m_actionType == None)
        return;

    writer->writeStartElement(MIX_ACTION_TYPES.at(m_actionType));
    if (m_actionType == ClientJoin || m_actionType == ClientLeave) {
        writer->writeAttribute("xmlns", ns_mix_pam);
        if (type() == Set)
            helperToXmlAddAttribute(writer, "channel", m_jid);

        if (m_actionType == ClientJoin)
            writer->writeStartElement("join");
        else if (m_actionType == ClientLeave)
            writer->writeStartElement("leave");
    }

    writer->writeAttribute("xmlns", ns_mix);
    helperToXmlAddAttribute(writer, "channel", m_channelName);
    if (type() == Result)
        helperToXmlAddAttribute(writer, "jid", m_jid);

    for (auto node : m_nodes) {
        writer->writeStartElement("subscribe");
        writer->writeAttribute("node", node);
        writer->writeEndElement();
    }
    if (!m_nick.isEmpty())
        writer->writeTextElement("nick", m_nick);

    writer->writeEndElement();
    if (m_actionType == ClientJoin || m_actionType == ClientLeave)
        writer->writeEndElement();
}
/// \endcond
