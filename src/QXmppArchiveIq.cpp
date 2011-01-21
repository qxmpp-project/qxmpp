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

#include "QXmppArchiveIq.h"
#include "QXmppUtils.h"

static const char *ns_archive = "urn:xmpp:archive";
static const char *ns_rsm = "http://jabber.org/protocol/rsm";

QXmppArchiveMessage::QXmppArchiveMessage()
    : m_received(false)
{
}

/// Returns the archived message's body.

QString QXmppArchiveMessage::body() const
{
    return m_body;
}

/// Sets the archived message's body.
///
/// \param body
void QXmppArchiveMessage::setBody(const QString &body)
{
    m_body = body;
}

/// Returns the archived message's date.

QDateTime QXmppArchiveMessage::date() const
{
    return m_date;
}

//// Sets the archived message's date.
///
/// \param date

void QXmppArchiveMessage::setDate(const QDateTime &date)
{
    m_date = date;
}

/// Returns true if the archived message was received, false if it was sent.

bool QXmppArchiveMessage::isReceived() const
{
    return m_received;
}

/// Set to true if the archived message was received, false if it was sent.
///
/// \param isReceived

void QXmppArchiveMessage::setReceived(bool isReceived)
{
    m_received = isReceived;
}

QXmppArchiveChat::QXmppArchiveChat()
    : m_version(0)
{
}

void QXmppArchiveChat::parse(const QDomElement &element)
{
    m_with = element.attribute("with");
    m_start = datetimeFromString(element.attribute("start"));
    m_subject = element.attribute("subject");
    m_thread = element.attribute("thread");
    m_version = element.attribute("version").toInt();

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if ((child.tagName() == "from") || (child.tagName() == "to"))
        {
            QXmppArchiveMessage message;
            message.setBody(child.firstChildElement("body").text());
            message.setDate(m_start.addSecs(child.attribute("secs").toInt()));
            message.setReceived(child.tagName() == "from");
            m_messages << message;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppArchiveChat::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("chat");
    writer->writeAttribute("xmlns", ns_archive);
    helperToXmlAddAttribute(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddAttribute(writer, "start", datetimeToString(m_start));
    helperToXmlAddAttribute(writer, "subject", m_subject);
    helperToXmlAddAttribute(writer, "thread", m_thread);
    if (m_version)
        helperToXmlAddAttribute(writer, "version", QString::number(m_version));
    foreach (const QXmppArchiveMessage &message, m_messages)
    {
        writer->writeStartElement(message.isReceived() ? "from" : "to");
        helperToXmlAddAttribute(writer, "secs", QString::number(m_start.secsTo(message.date())));
        writer->writeTextElement("body", message.body());
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

/// Returns the conversation's messages.

QList<QXmppArchiveMessage> QXmppArchiveChat::messages() const
{
    return m_messages;
}

/// Sets the conversation's messages.

void QXmppArchiveChat::setMessages(const QList<QXmppArchiveMessage> &messages)
{
    m_messages = messages;
}

/// Returns the start of this conversation.

QDateTime QXmppArchiveChat::start() const
{
    return m_start;
}

/// Sets the start of this conversation.

void QXmppArchiveChat::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the conversation's subject.

QString QXmppArchiveChat::subject() const
{
    return m_subject;
}

/// Sets the conversation's subject.

void QXmppArchiveChat::setSubject(const QString &subject)
{
    m_subject = subject;
}

/// Returns the conversation's thread.

QString QXmppArchiveChat::thread() const
{
    return m_thread;
}

/// Sets the conversation's thread.

void QXmppArchiveChat::setThread(const QString &thread)
{
    m_thread = thread;
}

/// Returns the conversation's version.

int QXmppArchiveChat::version() const
{
    return m_version;
}

/// Sets the conversation's version.

void QXmppArchiveChat::setVersion(int version)
{
    m_version = version;
}

/// Returns the JID of the remote party.

QString QXmppArchiveChat::with() const
{
    return m_with;
}

/// Sets the JID of the remote party.

void QXmppArchiveChat::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the chat conversation carried by this IQ.

QXmppArchiveChat QXmppArchiveChatIq::chat() const
{
    return m_chat;
}

/// Sets the chat conversation carried by this IQ.

void QXmppArchiveChatIq::setChat(const QXmppArchiveChat &chat)
{
    m_chat = chat;
}

bool QXmppArchiveChatIq::isArchiveChatIq(const QDomElement &element)
{
    QDomElement chatElement = element.firstChildElement("chat");
    return !chatElement.attribute("with").isEmpty();
    //return (chatElement.namespaceURI() == ns_archive);
}

void QXmppArchiveChatIq::parseElementFromChild(const QDomElement &element)
{
    m_chat.parse(element.firstChildElement("chat"));
}

void QXmppArchiveChatIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    m_chat.toXml(writer);
}

/// Constructs a QXmppArchiveListIq.

QXmppArchiveListIq::QXmppArchiveListIq()
    : QXmppIq(QXmppIq::Get), m_max(0)
{
}

/// Returns the list of chat conversations.

QList<QXmppArchiveChat> QXmppArchiveListIq::chats() const
{
    return m_chats;
}

/// Sets the list of chat conversations.

void QXmppArchiveListIq::setChats(const QList<QXmppArchiveChat> &chats)
{
    m_chats = chats;
}

/// Returns the maximum number of results.
///

int QXmppArchiveListIq::max() const
{
    return m_max;
}

/// Sets the maximum number of results.
///
/// \param max

void QXmppArchiveListIq::setMax(int max)
{
    m_max = max;
}

/// Returns the JID which archived conversations must match.
///

QString QXmppArchiveListIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
///
/// \param with

void QXmppArchiveListIq::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the start date/time for the archived conversations.
///

QDateTime QXmppArchiveListIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
///
/// \param start

void QXmppArchiveListIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the end date/time for the archived conversations.
///

QDateTime QXmppArchiveListIq::end() const
{
    return m_end;
}

/// Sets the end date/time for the archived conversations.
///
/// \param end

void QXmppArchiveListIq::setEnd(const QDateTime &end)
{
    m_end = end;
}

bool QXmppArchiveListIq::isArchiveListIq(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("list");
    return (listElement.namespaceURI() == ns_archive);
}

void QXmppArchiveListIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("list");
    m_with = listElement.attribute("with");
    m_start = datetimeFromString(listElement.attribute("start"));
    m_end = datetimeFromString(listElement.attribute("end"));

    QDomElement setElement = listElement.firstChildElement("set");
    if (setElement.namespaceURI() == ns_rsm)
        m_max = setElement.firstChildElement("max").text().toInt();

    QDomElement child = listElement.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "chat")
        {
            QXmppArchiveChat chat;
            chat.parse(child);
            m_chats << chat;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppArchiveListIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("list");
    writer->writeAttribute("xmlns", ns_archive);
    if (!m_with.isEmpty())
        helperToXmlAddAttribute(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddAttribute(writer, "start", datetimeToString(m_start));
    if (m_end.isValid())
        helperToXmlAddAttribute(writer, "end", datetimeToString(m_end));
    if (m_max > 0)
    {
        writer->writeStartElement("set");
        writer->writeAttribute("xmlns", ns_rsm);
        helperToXmlAddTextElement(writer, "max", QString::number(m_max));
        writer->writeEndElement();
    }
    foreach (const QXmppArchiveChat &chat, m_chats)
        chat.toXml(writer);
    writer->writeEndElement();
}

bool QXmppArchivePrefIq::isArchivePrefIq(const QDomElement &element)
{
    QDomElement prefElement = element.firstChildElement("pref");
    return (prefElement.namespaceURI() == ns_archive);
}

void QXmppArchivePrefIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("pref");
    Q_UNUSED(queryElement);
}

void QXmppArchivePrefIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("pref");
    writer->writeAttribute("xmlns", ns_archive);
    writer->writeEndElement();
}

QXmppArchiveRetrieveIq::QXmppArchiveRetrieveIq()
    : QXmppIq(QXmppIq::Get), m_max(0)
{
}

/// Returns the maximum number of results.
///

int QXmppArchiveRetrieveIq::max() const
{
    return m_max;
}

/// Sets the maximum number of results.
///
/// \param max

void QXmppArchiveRetrieveIq::setMax(int max)
{
    m_max = max;
}

/// Returns the start date/time for the archived conversations.
///

QDateTime QXmppArchiveRetrieveIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
///
/// \param start

void QXmppArchiveRetrieveIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the JID which archived conversations must match.
///

QString QXmppArchiveRetrieveIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
///
/// \param with

void QXmppArchiveRetrieveIq::setWith(const QString &with)
{
    m_with = with;
}

bool QXmppArchiveRetrieveIq::isArchiveRetrieveIq(const QDomElement &element)
{
    QDomElement retrieveElement = element.firstChildElement("retrieve");
    return (retrieveElement.namespaceURI() == ns_archive);
}

void QXmppArchiveRetrieveIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement retrieveElement = element.firstChildElement("retrieve");
    m_with = retrieveElement.attribute("with");
    m_start = datetimeFromString(retrieveElement.attribute("start"));
    QDomElement setElement = retrieveElement.firstChildElement("set");
    if (setElement.namespaceURI() == ns_rsm)
        m_max = setElement.firstChildElement("max").text().toInt();
}

void QXmppArchiveRetrieveIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("retrieve");
    writer->writeAttribute("xmlns", ns_archive);
    helperToXmlAddAttribute(writer, "with", m_with);
    helperToXmlAddAttribute(writer, "start", datetimeToString(m_start));
    if (m_max > 0)
    {
        writer->writeStartElement("set");
        writer->writeAttribute("xmlns", ns_rsm);
        helperToXmlAddTextElement(writer, "max", QString::number(m_max));
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
