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

#include "QXmppArchiveIq.h"
#include "QXmppUtils.h"

#include <QDebug>
#include <QDomElement>

static const char *ns_archive = "urn:xmpp:archive";

bool QXmppArchiveChatIq::isArchiveChatIq( QDomElement &element )
{
    QDomElement chatElement = element.firstChildElement("chat");
    return !chatElement.attribute("with").isEmpty();
    //return (chatElement.namespaceURI() == ns_archive);
}

QXmppArchiveChat QXmppArchiveChatIq::getChat() const
{
    return m_chat;
}

void QXmppArchiveChatIq::parse( QDomElement &element )
{
    QDomElement chatElement = element.firstChildElement("chat");
    m_chat.subject = chatElement.attribute("subject");
    m_chat.start = datetimeFromString(chatElement.attribute("start"));
    m_chat.version = chatElement.attribute("version").toInt();
    m_chat.with = chatElement.attribute("with");

    QDomElement child = chatElement.firstChildElement();
    while (!child.isNull())
    {
        if ((child.tagName() == "from") || (child.tagName() == "to"))
        {
            QXmppArchiveMessage message;
            message.datetime = m_chat.start.addSecs(child.attribute("secs").toInt());
            message.body = child.firstChildElement("body").text();
            message.local = (child.tagName() == "to");
            m_chat.messages << message;
        }
        child = child.nextSiblingElement();
    }
}

QXmppArchiveListIq::QXmppArchiveListIq()
    : QXmppIq(QXmppIq::Get), m_max(0)
{
}

QList<QXmppArchiveChat> QXmppArchiveListIq::getChats() const
{
    return m_chats;
}

int QXmppArchiveListIq::getMax() const
{
    return m_max;
}

void QXmppArchiveListIq::setMax(int max)
{
    m_max = max;
}

QString QXmppArchiveListIq::getWith() const
{
    return m_with;
}

void QXmppArchiveListIq::setWith( const QString &with )
{
    m_with = with;
}

QDateTime QXmppArchiveListIq::getStart() const
{
    return m_start;
}

void QXmppArchiveListIq::setStart( const QDateTime &start )
{
    m_start = start;
}

QDateTime QXmppArchiveListIq::getEnd() const
{
    return m_end;
}

void QXmppArchiveListIq::setEnd( const QDateTime &end )
{
    m_end = end;
}

bool QXmppArchiveListIq::isArchiveListIq( QDomElement &element )
{
    QDomElement listElement = element.firstChildElement("list");
    return (listElement.namespaceURI() == ns_archive);
}

void QXmppArchiveListIq::parse( QDomElement &element )
{
    QDomElement listElement = element.firstChildElement("list");
    m_with = element.attribute("with");

    QDomElement child = listElement.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "chat")
        {
            QXmppArchiveChat chat;
            chat.with = child.attribute("with");
            chat.start = datetimeFromString(child.attribute("start"));
            m_chats << chat;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppArchiveListIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("list");
    helperToXmlAddAttribute(writer, "xmlns", ns_archive);
    if (!m_with.isEmpty())
        helperToXmlAddAttribute(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddAttribute(writer, "start", datetimeToString(m_start));
    if (m_end.isValid())
        helperToXmlAddAttribute(writer, "end", datetimeToString(m_start));
    if (m_max > 0)
    {
        writer->writeStartElement("set");
        helperToXmlAddAttribute(writer, "xmlns", "http://jabber.org/protocol/rsm");
        if (m_max > 0)
            helperToXmlAddTextElement(writer, "max", QString::number(m_max));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

bool QXmppArchivePrefIq::isArchivePrefIq( QDomElement &element )
{
    QDomElement prefElement = element.firstChildElement("pref");
    return (prefElement.namespaceURI() == ns_archive);
}

void QXmppArchivePrefIq::parse( QDomElement &element )
{
    QDomElement queryElement = element.firstChildElement("pref");
    //setId( element.attribute("id"));
}

void QXmppArchivePrefIq::toXmlElementFromChild( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("pref");
    helperToXmlAddAttribute(writer, "xmlns", ns_archive);
    writer->writeEndElement();
}

QXmppArchiveRetrieveIq::QXmppArchiveRetrieveIq()
    : QXmppIq(QXmppIq::Get), m_max(0)
{
}

int QXmppArchiveRetrieveIq::getMax() const
{
    return m_max;
}

void QXmppArchiveRetrieveIq::setMax(int max)
{
    m_max = max;
}

QDateTime QXmppArchiveRetrieveIq::getStart() const
{
    return m_start;
}

void QXmppArchiveRetrieveIq::setStart( const QDateTime &start )
{
    m_start = start;
}

QString QXmppArchiveRetrieveIq::getWith() const
{
    return m_with;
}

void QXmppArchiveRetrieveIq::setWith( const QString &with )
{
    m_with = with;
}

void QXmppArchiveRetrieveIq::toXmlElementFromChild( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("retrieve");
    helperToXmlAddAttribute(writer, "xmlns", ns_archive);
    helperToXmlAddAttribute(writer, "with", m_with);
    helperToXmlAddAttribute(writer, "start", datetimeToString(m_start));
    if (m_max > 0)
    {
        writer->writeStartElement("set");
        helperToXmlAddAttribute(writer, "xmlns", "http://jabber.org/protocol/rsm");
        if (m_max > 0)
            helperToXmlAddTextElement(writer, "max", QString::number(m_max));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
