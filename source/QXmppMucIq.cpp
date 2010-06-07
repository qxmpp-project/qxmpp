/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

QString QXmppMucAdminIq::Item::affiliation() const
{
    return m_affiliation;
}

void QXmppMucAdminIq::Item::setAffiliation(const QString &affiliation)
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

QString QXmppMucAdminIq::Item::role() const
{
    return m_role;
}

void QXmppMucAdminIq::Item::setRole(const QString &role)
{
    m_role = role;
}

QList<QXmppMucAdminIq::Item> QXmppMucAdminIq::items() const
{
    return m_items;
}

void QXmppMucAdminIq::setItems(const QList<QXmppMucAdminIq::Item> &items)
{
    m_items = items;
}

bool QXmppMucAdminIq::isMucAdminIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_muc_admin);
}

void QXmppMucAdminIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    QDomElement child = queryElement.firstChildElement("item");
    while (!child.isNull())
    {
        QXmppMucAdminIq::Item item;
        item.setAffiliation(child.attribute("affiliation"));
        item.setJid(child.attribute("jid"));
        item.setNick(child.attribute("nick"));
        item.setRole(child.attribute("role"));
        item.setReason(child.firstChildElement("reason").text());
        m_items << item;
        child = child.nextSiblingElement("item");
    }
}
void QXmppMucAdminIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_muc_admin);
    foreach (const QXmppMucAdminIq::Item &item, m_items)
    {
        writer->writeStartElement("item");
        helperToXmlAddAttribute(writer, "jid", item.jid());
        helperToXmlAddAttribute(writer, "affiliation", item.affiliation());
        helperToXmlAddAttribute(writer, "nick", item.nick());
        helperToXmlAddAttribute(writer, "role", item.role());
        if (!item.reason().isEmpty())
            helperToXmlAddTextElement(writer, "reason", item.reason());
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

QXmppDataForm QXmppMucOwnerIq::form() const
{
    return m_form;
}

void QXmppMucOwnerIq::setForm(const QXmppDataForm &form)
{
    m_form = form;
}

bool QXmppMucOwnerIq::isMucOwnerIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_muc_owner);
}

void QXmppMucOwnerIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    m_form.parse(queryElement.firstChildElement("x"));
}

void QXmppMucOwnerIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_muc_owner);
    m_form.toXml(writer);
    writer->writeEndElement();
}

