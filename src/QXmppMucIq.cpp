/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

QXmppMucAdminIq::Item::Item()
    : m_affiliation(QXmppMucAdminIq::Item::UnspecifiedAffiliation),
    m_role(QXmppMucAdminIq::Item::UnspecifiedRole)
{
}

QXmppMucAdminIq::Item::Affiliation QXmppMucAdminIq::Item::affiliation() const
{
    return m_affiliation;
}

QXmppMucAdminIq::Item::Affiliation QXmppMucAdminIq::Item::affiliationFromString(const QString &affiliationStr)
{
    if (affiliationStr == "owner")
        return QXmppMucAdminIq::Item::OwnerAffiliation;
    else if (affiliationStr == "admin")
        return QXmppMucAdminIq::Item::AdminAffiliation;
    else if (affiliationStr == "member")
        return QXmppMucAdminIq::Item::MemberAffiliation;
    else if (affiliationStr == "outcast")
        return QXmppMucAdminIq::Item::OutcastAffiliation;
    else if (affiliationStr == "none")
        return QXmppMucAdminIq::Item::NoAffiliation;
    else
        return QXmppMucAdminIq::Item::UnspecifiedAffiliation;
}

QString QXmppMucAdminIq::Item::affiliationToString(Affiliation affiliation)
{
    switch (affiliation) {
    case QXmppMucAdminIq::Item::OwnerAffiliation:
        return "owner";
    case QXmppMucAdminIq::Item::AdminAffiliation:
        return "admin";
    case QXmppMucAdminIq::Item::MemberAffiliation:
        return "member";
    case QXmppMucAdminIq::Item::OutcastAffiliation:
        return "outcast";
    case QXmppMucAdminIq::Item::NoAffiliation:
        return "none";
    default:
        return QString();
    }
}

void QXmppMucAdminIq::Item::setAffiliation(Affiliation affiliation)
{
    m_affiliation = affiliation;
}

QString QXmppMucAdminIq::Item::jid() const
{
    return m_jid;
}

void QXmppMucAdminIq::Item::setJid(const QString &jid)
{
    m_jid = jid;
}

QString QXmppMucAdminIq::Item::nick() const
{
    return m_nick;
}

void QXmppMucAdminIq::Item::setNick(const QString &nick)
{
    m_nick = nick;
}

QString QXmppMucAdminIq::Item::reason() const
{
    return m_reason;
}

void QXmppMucAdminIq::Item::setReason(const QString &reason)
{
    m_reason = reason;
}

QXmppMucAdminIq::Item::Role QXmppMucAdminIq::Item::role() const
{
    return m_role;
}

QXmppMucAdminIq::Item::Role QXmppMucAdminIq::Item::roleFromString(const QString &roleStr)
{
    if (roleStr == "moderator")
        return QXmppMucAdminIq::Item::ModeratorRole;
    else if (roleStr == "participant")
        return QXmppMucAdminIq::Item::ParticipantRole;
    else if (roleStr == "visitor")
        return QXmppMucAdminIq::Item::VisitorRole;
    else if (roleStr == "none")
        return QXmppMucAdminIq::Item::NoRole;
    else
        return QXmppMucAdminIq::Item::UnspecifiedRole;
}

QString QXmppMucAdminIq::Item::roleToString(Role role)
{
    switch (role) {
    case QXmppMucAdminIq::Item::ModeratorRole:
        return "moderator";
    case QXmppMucAdminIq::Item::ParticipantRole:
        return "participant";
    case QXmppMucAdminIq::Item::VisitorRole:
        return "visitor";
    case QXmppMucAdminIq::Item::NoRole:
        return "none";
    default:
        return QString();
    }
}

void QXmppMucAdminIq::Item::setRole(Role role)
{
    m_role = role;
}

void QXmppMucAdminIq::Item::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("item");
    helperToXmlAddAttribute(writer, "jid", m_jid);
    helperToXmlAddAttribute(writer, "affiliation", affiliationToString(m_affiliation));
    helperToXmlAddAttribute(writer, "nick", m_nick);
    helperToXmlAddAttribute(writer, "role", roleToString(m_role));
    if (!m_reason.isEmpty())
        helperToXmlAddTextElement(writer, "reason", m_reason);
    writer->writeEndElement();
}

/// Returns the IQ's items.

QList<QXmppMucAdminIq::Item> QXmppMucAdminIq::items() const
{
    return m_items;
}

/// Sets the IQ's items.
///
/// \param items

void QXmppMucAdminIq::setItems(const QList<QXmppMucAdminIq::Item> &items)
{
    m_items = items;
}

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
        QXmppMucAdminIq::Item item;
        item.setAffiliation(QXmppMucAdminIq::Item::affiliationFromString(child.attribute("affiliation").toLower()));
        item.setJid(child.attribute("jid"));
        item.setNick(child.attribute("nick"));
        item.setRole(QXmppMucAdminIq::Item::roleFromString(child.attribute("role").toLower()));
        item.setReason(child.firstChildElement("reason").text());
        m_items << item;
        child = child.nextSiblingElement("item");
    }
}

void QXmppMucAdminIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_muc_admin);
    foreach (const QXmppMucAdminIq::Item &item, m_items)
        item.toXml(writer);
    writer->writeEndElement();
}

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

