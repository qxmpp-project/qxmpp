/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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
#include <QSharedData>

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

class QXmppMixIqPrivate : public QSharedData
{
public:
    QString jid;
    QString channelName;
    QStringList nodes;
    QString nick;
    QXmppMixIq::Type actionType = QXmppMixIq::None;
};

QXmppMixIq::QXmppMixIq()
    : d(new QXmppMixIqPrivate)
{
}

QXmppMixIq::QXmppMixIq(const QXmppMixIq &) = default;

QXmppMixIq::~QXmppMixIq() = default;

QXmppMixIq &QXmppMixIq::operator=(const QXmppMixIq &) = default;

/// Returns the channel JID. It also contains a participant id for Join/
/// ClientJoin results.

QString QXmppMixIq::jid() const
{
    return d->jid;
}

/// Sets the channel JID. For results of Join/ClientJoin queries this also
/// needs to contain a participant id.

void QXmppMixIq::setJid(const QString& jid)
{
    d->jid = jid;
}

/// Returns the channel name (the name part of the channel JID). This may still
/// be empty, if a JID was set.

QString QXmppMixIq::channelName() const
{
    return d->channelName;
}

/// Sets the channel name for creating/destroying specific channels. When you
/// create a new channel, this can also be left empty to let the server
/// generate a name.

void QXmppMixIq::setChannelName(const QString& channelName)
{
    d->channelName = channelName;
}

/// Returns the list of nodes to subscribe to.

QStringList QXmppMixIq::nodes() const
{
    return d->nodes;
}

/// Sets the nodes to subscribe to. Note that for UpdateSubscription queries
/// you only need to include the new subscriptions.

void QXmppMixIq::setNodes(const QStringList& nodes)
{
    d->nodes = nodes;
}

/// Returns the user's nickname in the channel.

QString QXmppMixIq::nick() const
{
    return d->nick;
}

/// Sets the nickname for the channel.

void QXmppMixIq::setNick(const QString& nick)
{
    d->nick = nick;
}

/// Returns the MIX channel action type.

QXmppMixIq::Type QXmppMixIq::actionType() const
{
    return d->actionType;
}

/// Sets the channel action.

void QXmppMixIq::setActionType(QXmppMixIq::Type type)
{
    d->actionType = type;
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
    d->actionType = (QXmppMixIq::Type) MIX_ACTION_TYPES.indexOf(child.tagName());

    if (child.namespaceURI() == ns_mix_pam) {
        if (child.hasAttribute("channel"))
            d->jid = child.attribute("channel");

        child = child.firstChildElement();
    }

    if (!child.isNull() && child.namespaceURI() == ns_mix) {
        if (child.hasAttribute("jid"))
            d->jid = child.attribute("jid");
        if (child.hasAttribute("channel"))
            d->channelName = child.attribute("channel");

        QDomElement subChild = child.firstChildElement();
        while (!subChild.isNull()) {
            if (subChild.tagName() == "subscribe")
                d->nodes << subChild.attribute("node");
            else if (subChild.tagName() == "nick")
                d->nick = subChild.text();

            subChild = subChild.nextSiblingElement();
        }
    }
}

void QXmppMixIq::toXmlElementFromChild(QXmlStreamWriter* writer) const
{
    if (d->actionType == None)
        return;

    writer->writeStartElement(MIX_ACTION_TYPES.at(d->actionType));
    if (d->actionType == ClientJoin || d->actionType == ClientLeave) {
        writer->writeAttribute("xmlns", ns_mix_pam);
        if (type() == Set)
            helperToXmlAddAttribute(writer, "channel", d->jid);

        if (d->actionType == ClientJoin)
            writer->writeStartElement("join");
        else if (d->actionType == ClientLeave)
            writer->writeStartElement("leave");
    }

    writer->writeAttribute("xmlns", ns_mix);
    helperToXmlAddAttribute(writer, "channel", d->channelName);
    if (type() == Result)
        helperToXmlAddAttribute(writer, "jid", d->jid);

    for (const auto &node : d->nodes) {
        writer->writeStartElement("subscribe");
        writer->writeAttribute("node", node);
        writer->writeEndElement();
    }
    if (!d->nick.isEmpty())
        writer->writeTextElement("nick", d->nick);

    writer->writeEndElement();
    if (d->actionType == ClientJoin || d->actionType == ClientLeave)
        writer->writeEndElement();
}
/// \endcond
