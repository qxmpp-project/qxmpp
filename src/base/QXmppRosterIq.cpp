/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#include "QXmppRosterIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QSharedData>
#include <QXmlStreamWriter>

class QXmppRosterIqPrivate : public QSharedData
{
public:
    QList<QXmppRosterIq::Item> items;
    // XEP-0237 Roster Versioning
    QString version;
    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    bool mixAnnotate = false;
};

QXmppRosterIq::QXmppRosterIq()
    : d(new QXmppRosterIqPrivate)
{
}

QXmppRosterIq::~QXmppRosterIq() = default;

QXmppRosterIq &QXmppRosterIq::operator=(const QXmppRosterIq &) = default;

/// Adds an item to the roster IQ.
///
/// \param item

void QXmppRosterIq::addItem(const Item &item)
{
    d->items.append(item);
}

/// Returns the roster IQ's items.

QList<QXmppRosterIq::Item> QXmppRosterIq::items() const
{
    return d->items;
}

/// Returns the roster version of IQ.
///
/// \return version as a QString
///

QString QXmppRosterIq::version() const
{
    return d->version;
}

/// Sets the roster version of IQ.
///
/// \param version as a QString
///

void QXmppRosterIq::setVersion(const QString &version)
{
    d->version = version;
}

/// Whether to annotate which items are MIX channels.

bool QXmppRosterIq::mixAnnotate() const
{
    return d->mixAnnotate;
}

/// Sets whether to include which roster items are MIX channels. This MUST only
/// be enabled in get requests.

void QXmppRosterIq::setMixAnnotate(bool mixAnnotate)
{
    d->mixAnnotate = mixAnnotate;
}

/// \cond
bool QXmppRosterIq::isRosterIq(const QDomElement &element)
{
    return (element.firstChildElement("query").namespaceURI() == ns_roster);
}

void QXmppRosterIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    setVersion(queryElement.attribute("ver"));

    QDomElement itemElement = queryElement.firstChildElement("item");
    while (!itemElement.isNull()) {
        QXmppRosterIq::Item item;
        item.parse(itemElement);
        d->items.append(item);
        itemElement = itemElement.nextSiblingElement("item");
    }

    QDomElement annotateElement = queryElement.firstChildElement("annotate");
    setMixAnnotate(!annotateElement.isNull() && annotateElement.namespaceURI()
                   == ns_mix_roster);
}

void QXmppRosterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeDefaultNamespace(ns_roster);

    // XEP-0237 roster versioning - If the server does not advertise support for roster versioning, the client MUST NOT include the 'ver' attribute.
    if (!version().isEmpty())
        writer->writeAttribute("ver", version());

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    if (d->mixAnnotate) {
        writer->writeStartElement("annotate");
        writer->writeAttribute("xmlns", ns_mix_roster);
        writer->writeEndElement();
    }

    for (int i = 0; i < d->items.count(); ++i)
        d->items.at(i).toXml(writer);
    writer->writeEndElement();
}
/// \endcond

class QXmppRosterIq::ItemPrivate : public QSharedData
{
public:
    QString bareJid;
    Item::SubscriptionType type;
    QString name;
    // can be subscribe/unsubscribe (attribute "ask")
    QString subscriptionStatus;
    QSet<QString> groups;
    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    bool isMixChannel = false;
    QString mixParticipantId;
};

/// Constructs a new roster entry.

QXmppRosterIq::Item::Item()
    : d(new ItemPrivate)
{
    d->type = NotSet;
}

QXmppRosterIq::Item::Item(const QXmppRosterIq::Item &other) = default;

QXmppRosterIq::Item::~Item() = default;

QXmppRosterIq::Item &QXmppRosterIq::Item::operator=(const Item &other) = default;

/// Returns the bareJid of the roster entry.
///
/// \return bareJid as a QString
///

QString QXmppRosterIq::Item::bareJid() const
{
    return d->bareJid;
}

/// Sets the bareJid of the roster entry.
///
/// \param bareJid as a QString
///

void QXmppRosterIq::Item::setBareJid(const QString &bareJid)
{
    d->bareJid = bareJid;
}

/// Returns the groups of the roster entry.
///
/// \return QSet<QString> list of all the groups
///

QSet<QString> QXmppRosterIq::Item::groups() const
{
    return d->groups;
}

/// Sets the groups of the roster entry.
///
/// \param groups list of all the groups as a QSet<QString>
///

void QXmppRosterIq::Item::setGroups(const QSet<QString> &groups)
{
    d->groups = groups;
}

/// Returns the name of the roster entry.
///
/// \return name as a QString
///

QString QXmppRosterIq::Item::name() const
{
    return d->name;
}

/// Sets the name of the roster entry.
///
/// \param name as a QString
///

void QXmppRosterIq::Item::setName(const QString &name)
{
    d->name = name;
}

/// Returns the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \return subscription status as a QString
///
///

QString QXmppRosterIq::Item::subscriptionStatus() const
{
    return d->subscriptionStatus;
}

/// Sets the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \param status as a QString
///

void QXmppRosterIq::Item::setSubscriptionStatus(const QString &status)
{
    d->subscriptionStatus = status;
}

/// Returns the subscription type of the roster entry.
///

QXmppRosterIq::Item::SubscriptionType
QXmppRosterIq::Item::subscriptionType() const
{
    return d->type;
}

/// Sets the subscription type of the roster entry.
///
/// \param type
///

void QXmppRosterIq::Item::setSubscriptionType(SubscriptionType type)
{
    d->type = type;
}

QString QXmppRosterIq::Item::getSubscriptionTypeStr() const
{
    switch (d->type) {
    case NotSet:
        return "";
    case None:
        return "none";
    case Both:
        return "both";
    case From:
        return "from";
    case To:
        return "to";
    case Remove:
        return "remove";
    default: {
        qWarning("QXmppRosterIq::Item::getTypeStr(): invalid type");
        return "";
    }
    }
}

void QXmppRosterIq::Item::setSubscriptionTypeFromStr(const QString &type)
{
    if (type == "")
        setSubscriptionType(NotSet);
    else if (type == "none")
        setSubscriptionType(None);
    else if (type == "both")
        setSubscriptionType(Both);
    else if (type == "from")
        setSubscriptionType(From);
    else if (type == "to")
        setSubscriptionType(To);
    else if (type == "remove")
        setSubscriptionType(Remove);
    else
        qWarning("QXmppRosterIq::Item::setTypeFromStr(): invalid type");
}

/// Returns whether this is a MIX channel.

bool QXmppRosterIq::Item::isMixChannel() const
{
    return d->isMixChannel;
}

/// Sets whether this is a MIX channel.

void QXmppRosterIq::Item::setIsMixChannel(bool isMixChannel)
{
    d->isMixChannel = isMixChannel;
}

/// Returns the participant id for this MIX channel.

QString QXmppRosterIq::Item::mixParticipantId() const
{
    return d->mixParticipantId;
}

/// Sets the participant id for this MIX channel.

void QXmppRosterIq::Item::setMixParticipantId(const QString& participantId)
{
    d->mixParticipantId = participantId;
}

/// \cond
void QXmppRosterIq::Item::parse(const QDomElement &element)
{
    d->name = element.attribute("name");
    d->bareJid = element.attribute("jid");
    setSubscriptionTypeFromStr(element.attribute("subscription"));
    setSubscriptionStatus(element.attribute("ask"));

    QDomElement groupElement = element.firstChildElement("group");
    while (!groupElement.isNull()) {
        d->groups << groupElement.text();
        groupElement = groupElement.nextSiblingElement("group");
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    QDomElement channelElement = element.firstChildElement("channel");
    if (!channelElement.isNull() && channelElement.namespaceURI() == ns_mix_roster) {
        d->isMixChannel = true;
        d->mixParticipantId = channelElement.attribute("participant-id");
    }
}

void QXmppRosterIq::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "jid", d->bareJid);
    helperToXmlAddAttribute(writer, "name", d->name);
    helperToXmlAddAttribute(writer, "subscription", getSubscriptionTypeStr());
    helperToXmlAddAttribute(writer, "ask", subscriptionStatus());

    QSet<QString>::const_iterator i = d->groups.constBegin();
    while (i != d->groups.constEnd()) {
        helperToXmlAddTextElement(writer, "group", *i);
        ++i;
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    if (d->isMixChannel) {
        writer->writeStartElement("channel");
        writer->writeAttribute("xmlns", ns_mix_roster);
        helperToXmlAddAttribute(writer, "participant-id", d->mixParticipantId);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
