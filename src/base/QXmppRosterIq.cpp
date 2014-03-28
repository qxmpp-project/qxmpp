/*
 * Copyright (C) 2008-2014 The QXmpp developers
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

#include <QDomElement>
#include <QXmlStreamWriter>

#include "QXmppRosterIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"

/// Adds an item to the roster IQ.
///
/// \param item

void QXmppRosterIq::addItem(const Item& item)
{
    m_items.append(item);
}

/// Returns the roster IQ's items.

QList<QXmppRosterIq::Item> QXmppRosterIq::items() const
{
    return m_items;
}

/// \cond
bool QXmppRosterIq::isRosterIq(const QDomElement &element)
{
    return (element.firstChildElement("query").namespaceURI() == ns_roster);
}

void QXmppRosterIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement itemElement = element.
                              firstChildElement("query").
                              firstChildElement("item");
    while(!itemElement.isNull())
    {
        QXmppRosterIq::Item item;
        item.parse(itemElement);
        m_items.append(item);
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppRosterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute( "xmlns", ns_roster);

    for(int i = 0; i < m_items.count(); ++i)
        m_items.at(i).toXml(writer);
    writer->writeEndElement();
}
/// \endcond

/// Constructs a new roster entry.

QXmppRosterIq::Item::Item()
    : m_type(NotSet)
{
}

/// Returns the bareJid of the roster entry.
///
/// \return bareJid as a QString
///

QString QXmppRosterIq::Item::bareJid() const
{
    return m_bareJid;
}

/// Sets the bareJid of the roster entry.
///
/// \param bareJid as a QString
///

void QXmppRosterIq::Item::setBareJid(const QString &bareJid)
{
    m_bareJid = bareJid;
}

/// Returns the groups of the roster entry.
///
/// \return QSet<QString> list of all the groups
///

QSet<QString> QXmppRosterIq::Item::groups() const
{
    return m_groups;
}

/// Sets the groups of the roster entry.
///
/// \param groups list of all the groups as a QSet<QString>
///

void QXmppRosterIq::Item::setGroups(const QSet<QString>& groups)
{
    m_groups = groups;
}

/// Returns the name of the roster entry.
///
/// \return name as a QString
///

QString QXmppRosterIq::Item::name() const
{
    return m_name;
}

/// Sets the name of the roster entry.
///
/// \param name as a QString
///

void QXmppRosterIq::Item::setName(const QString &name)
{
    m_name = name;
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
    return m_subscriptionStatus;
}

/// Sets the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \param status as a QString
///

void QXmppRosterIq::Item::setSubscriptionStatus(const QString &status)
{
    m_subscriptionStatus = status;
}

/// Returns the subscription type of the roster entry.
///

QXmppRosterIq::Item::SubscriptionType
        QXmppRosterIq::Item::subscriptionType() const
{
    return m_type;
}

/// Sets the subscription type of the roster entry.
///
/// \param type
///

void QXmppRosterIq::Item::setSubscriptionType(SubscriptionType type)
{
    m_type = type;
}

QString QXmppRosterIq::Item::getSubscriptionTypeStr() const
{
    switch(m_type)
    {
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
    default:
        {
            qWarning("QXmppRosterIq::Item::getTypeStr(): invalid type");
            return "";
        }
    }
}

void QXmppRosterIq::Item::setSubscriptionTypeFromStr(const QString& type)
{
    if(type == "")
        setSubscriptionType(NotSet);
    else if(type == "none")
        setSubscriptionType(None);
    else if(type == "both")
        setSubscriptionType(Both);
    else if(type == "from")
        setSubscriptionType(From);
    else if(type == "to")
        setSubscriptionType(To);
    else if(type == "remove")
        setSubscriptionType(Remove);
    else
        qWarning("QXmppRosterIq::Item::setTypeFromStr(): invalid type");
}

/// \cond
void QXmppRosterIq::Item::parse(const QDomElement &element)
{
    m_name = element.attribute("name");
    m_bareJid = element.attribute("jid");
    setSubscriptionTypeFromStr(element.attribute("subscription"));
    setSubscriptionStatus(element.attribute("ask"));

    QDomElement groupElement = element.firstChildElement("group");
    while(!groupElement.isNull())
    {
        m_groups << groupElement.text();
        groupElement = groupElement.nextSiblingElement("group");
    }
}

void QXmppRosterIq::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer,"jid", m_bareJid);
    helperToXmlAddAttribute(writer,"name", m_name);
    helperToXmlAddAttribute(writer,"subscription", getSubscriptionTypeStr());
    helperToXmlAddAttribute(writer, "ask", subscriptionStatus());

    QSet<QString>::const_iterator i = m_groups.constBegin();
    while(i != m_groups.constEnd())
    {
        helperToXmlAddTextElement(writer,"group", *i);
        ++i;
    }
    writer->writeEndElement();
}
/// \endcond
