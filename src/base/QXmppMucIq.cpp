/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
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

#include "QXmppConstants.h"
#include "QXmppMucIq.h"
#include "QXmppUtils.h"

QXmppMucItem::QXmppMucItem()
    : m_affiliation(QXmppMucItem::UnspecifiedAffiliation),
    m_role(QXmppMucItem::UnspecifiedRole)
{
}

/// Returns true if the current item is null.

bool QXmppMucItem::isNull() const
{
    return m_actor.isEmpty() &&
           m_affiliation == UnspecifiedAffiliation &&
           m_jid.isEmpty() &&
           m_nick.isEmpty() &&
           m_reason.isEmpty() &&
           m_role == UnspecifiedRole;
}

/// Returns the actor for this item, for instance the admin who kicked
/// a user out of a room.

QString QXmppMucItem::actor() const
{
    return m_actor;
}

/// Sets the \a actor for this item, for instance the admin who kicked
/// a user out of a room.

void QXmppMucItem::setActor(const QString &actor)
{
    m_actor = actor;
}

/// Returns the user's affiliation, i.e. long-lived permissions.

QXmppMucItem::Affiliation QXmppMucItem::affiliation() const
{
    return m_affiliation;
}

/// \cond
QXmppMucItem::Affiliation QXmppMucItem::affiliationFromString(const QString &affiliationStr)
{
    if (affiliationStr == "owner")
        return QXmppMucItem::OwnerAffiliation;
    else if (affiliationStr == "admin")
        return QXmppMucItem::AdminAffiliation;
    else if (affiliationStr == "member")
        return QXmppMucItem::MemberAffiliation;
    else if (affiliationStr == "outcast")
        return QXmppMucItem::OutcastAffiliation;
    else if (affiliationStr == "none")
        return QXmppMucItem::NoAffiliation;
    else
        return QXmppMucItem::UnspecifiedAffiliation;
}

QString QXmppMucItem::affiliationToString(Affiliation affiliation)
{
    switch (affiliation) {
    case QXmppMucItem::OwnerAffiliation:
        return "owner";
    case QXmppMucItem::AdminAffiliation:
        return "admin";
    case QXmppMucItem::MemberAffiliation:
        return "member";
    case QXmppMucItem::OutcastAffiliation:
        return "outcast";
    case QXmppMucItem::NoAffiliation:
        return "none";
    default:
        return QString();
    }
}
/// \endcond

/// Sets the user's affiliation, i.e. long-lived permissions.
///
/// \param affiliation

void QXmppMucItem::setAffiliation(Affiliation affiliation)
{
    m_affiliation = affiliation;
}

/// Returns the user's real JID.

QString QXmppMucItem::jid() const
{
    return m_jid;
}

/// Sets the user's real JID.
///
/// \param jid

void QXmppMucItem::setJid(const QString &jid)
{
    m_jid = jid;
}

/// Returns the user's nickname.

QString QXmppMucItem::nick() const
{
    return m_nick;
}

/// Sets the user's nickname.
///
/// \param nick

void QXmppMucItem::setNick(const QString &nick)
{
    m_nick = nick;
}

/// Returns the reason for this item, for example the reason for kicking
/// a user out of a room.

QString QXmppMucItem::reason() const
{
    return m_reason;
}

/// Sets the \a reason for this item, for example the reason for kicking
/// a user out of a room.

void QXmppMucItem::setReason(const QString &reason)
{
    m_reason = reason;
}

/// Returns the user's role, i.e. short-lived permissions.

QXmppMucItem::Role QXmppMucItem::role() const
{
    return m_role;
}

/// \cond
QXmppMucItem::Role QXmppMucItem::roleFromString(const QString &roleStr)
{
    if (roleStr == "moderator")
        return QXmppMucItem::ModeratorRole;
    else if (roleStr == "participant")
        return QXmppMucItem::ParticipantRole;
    else if (roleStr == "visitor")
        return QXmppMucItem::VisitorRole;
    else if (roleStr == "none")
        return QXmppMucItem::NoRole;
    else
        return QXmppMucItem::UnspecifiedRole;
}

QString QXmppMucItem::roleToString(Role role)
{
    switch (role) {
    case QXmppMucItem::ModeratorRole:
        return "moderator";
    case QXmppMucItem::ParticipantRole:
        return "participant";
    case QXmppMucItem::VisitorRole:
        return "visitor";
    case QXmppMucItem::NoRole:
        return "none";
    default:
        return QString();
    }
}
/// \endcond

/// Sets the user's role, i.e. short-lived permissions.
///
/// \param role

void QXmppMucItem::setRole(Role role)
{
    m_role = role;
}

/// \cond
void QXmppMucItem::parse(const QDomElement &element)
{
    m_affiliation = QXmppMucItem::affiliationFromString(element.attribute("affiliation").toLower());
    m_jid = element.attribute("jid");
    m_nick = element.attribute("nick");
    m_role = QXmppMucItem::roleFromString(element.attribute("role").toLower());
    m_actor = element.firstChildElement("actor").attribute("jid");
    m_reason = element.firstChildElement("reason").text();
}

void QXmppMucItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "affiliation", affiliationToString(m_affiliation));
    helperToXmlAddAttribute(writer, "jid", m_jid);
    helperToXmlAddAttribute(writer, "nick", m_nick);
    helperToXmlAddAttribute(writer, "role", roleToString(m_role));
    if (!m_actor.isEmpty()) {
        writer->writeStartElement("actor");
        helperToXmlAddAttribute(writer, "jid", m_actor);
        writer->writeEndElement();
    }
    if (!m_reason.isEmpty())
        helperToXmlAddTextElement(writer, "reason", m_reason);
    writer->writeEndElement();
}
/// \endcond

/// Returns the IQ's items.

QList<QXmppMucItem> QXmppMucAdminIq::items() const
{
    return m_items;
}

/// Sets the IQ's items.
///
/// \param items

void QXmppMucAdminIq::setItems(const QList<QXmppMucItem> &items)
{
    m_items = items;
}

/// \cond
bool QXmppMucAdminIq::isMucAdminIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_muc_admin);
}

void QXmppMucAdminIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    QDomElement child = queryElement.firstChildElement("item");
    while (!child.isNull())
    {
        QXmppMucItem item;
        item.parse(child);
        m_items << item;
        child = child.nextSiblingElement("item");
    }
}

void QXmppMucAdminIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_muc_admin);
    foreach (const QXmppMucItem &item, m_items)
        item.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

/// Returns the IQ's data form.

QXmppDataForm QXmppMucOwnerIq::form() const
{
    return m_form;
}

/// Sets the IQ's data form.
///
/// \param form

void QXmppMucOwnerIq::setForm(const QXmppDataForm &form)
{
    m_form = form;
}

/// \cond
bool QXmppMucOwnerIq::isMucOwnerIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_muc_owner);
}

void QXmppMucOwnerIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    m_form.parse(queryElement.firstChildElement("x"));
}

void QXmppMucOwnerIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_muc_owner);
    m_form.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
