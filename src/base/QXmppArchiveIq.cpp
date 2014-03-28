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

#include "QXmppArchiveIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"

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

/// \cond
void QXmppArchiveChat::parse(const QDomElement &element)
{
    m_with = element.attribute("with");
    m_start = QXmppUtils::datetimeFromString(element.attribute("start"));
    m_subject = element.attribute("subject");
    m_thread = element.attribute("thread");
    m_version = element.attribute("version").toInt();

    QDateTime timeAccu = m_start;

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if ((child.tagName() == "from") || (child.tagName() == "to"))
        {
            QXmppArchiveMessage message;
            message.setBody(child.firstChildElement("body").text());
            timeAccu = timeAccu.addSecs(child.attribute("secs").toInt());
            message.setDate(timeAccu);
            message.setReceived(child.tagName() == "from");
            m_messages << message;
        }
        child = child.nextSiblingElement();
    }
}

void QXmppArchiveChat::toXml(QXmlStreamWriter *writer, const QXmppResultSetReply &rsm) const
{
    writer->writeStartElement("chat");
    writer->writeAttribute("xmlns", ns_archive);
    helperToXmlAddAttribute(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddAttribute(writer, "start", QXmppUtils::datetimeToString(m_start));
    helperToXmlAddAttribute(writer, "subject", m_subject);
    helperToXmlAddAttribute(writer, "thread", m_thread);
    if (m_version)
        helperToXmlAddAttribute(writer, "version", QString::number(m_version));

    QDateTime prevTime = m_start;

    foreach (const QXmppArchiveMessage &message, m_messages)
    {
        writer->writeStartElement(message.isReceived() ? "from" : "to");
        helperToXmlAddAttribute(writer, "secs", QString::number(prevTime.secsTo(message.date())));
        writer->writeTextElement("body", message.body());
        writer->writeEndElement();
        prevTime = message.date();
    }
    if (!rsm.isNull())
        rsm.toXml(writer);
    writer->writeEndElement();
}
/// \endcond

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

/// Returns the result set management reply.
///
/// This is used for paging through messages.

QXmppResultSetReply QXmppArchiveChatIq::resultSetReply() const
{
    return m_rsmReply;
}

/// Sets the result set management reply.
///
/// This is used for paging through messages.

void QXmppArchiveChatIq::setResultSetReply(const QXmppResultSetReply& rsm)
{
    m_rsmReply = rsm;
}

/// \cond
bool QXmppArchiveChatIq::isArchiveChatIq(const QDomElement &element)
{
    QDomElement chatElement = element.firstChildElement("chat");
    return !chatElement.attribute("with").isEmpty();
    //return (chatElement.namespaceURI() == ns_archive);
}

void QXmppArchiveChatIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement chatElement = element.firstChildElement("chat");
    m_chat.parse(chatElement);
    m_rsmReply.parse(chatElement);
}

void QXmppArchiveChatIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    m_chat.toXml(writer, m_rsmReply);
}
/// \endcond

/// Constructs a QXmppArchiveListIq.

QXmppArchiveListIq::QXmppArchiveListIq()
    : QXmppIq(QXmppIq::Get)
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

/// Returns the result set management query.
///
/// This is used for paging through conversations.

QXmppResultSetQuery QXmppArchiveListIq::resultSetQuery() const
{
    return m_rsmQuery;
}

/// Sets the result set management query.
///
/// This is used for paging through conversations.

void QXmppArchiveListIq::setResultSetQuery(const QXmppResultSetQuery& rsm)
{
    m_rsmQuery = rsm;
}

/// Returns the result set management reply.
///
/// This is used for paging through conversations.

QXmppResultSetReply QXmppArchiveListIq::resultSetReply() const
{
    return m_rsmReply;
}

/// Sets the result set management reply.
///
/// This is used for paging through conversations.

void QXmppArchiveListIq::setResultSetReply(const QXmppResultSetReply& rsm)
{
    m_rsmReply = rsm;
}

/// \cond
bool QXmppArchiveListIq::isArchiveListIq(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("list");
    return (listElement.namespaceURI() == ns_archive);
}

void QXmppArchiveListIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("list");
    m_with = listElement.attribute("with");
    m_start = QXmppUtils::datetimeFromString(listElement.attribute("start"));
    m_end = QXmppUtils::datetimeFromString(listElement.attribute("end"));

    m_rsmQuery.parse(listElement);
    m_rsmReply.parse(listElement);

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
        helperToXmlAddAttribute(writer, "start", QXmppUtils::datetimeToString(m_start));
    if (m_end.isValid())
        helperToXmlAddAttribute(writer, "end", QXmppUtils::datetimeToString(m_end));
    if (!m_rsmQuery.isNull())
        m_rsmQuery.toXml(writer);
    else if (!m_rsmReply.isNull())
        m_rsmReply.toXml(writer);
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
/// \endcond

/// Returns the JID which archived conversations must match.
///

QString QXmppArchiveRemoveIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
///
/// \param with

void QXmppArchiveRemoveIq::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the start date/time for the archived conversations.
///

QDateTime QXmppArchiveRemoveIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
///
/// \param start

void QXmppArchiveRemoveIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the end date/time for the archived conversations.
///

QDateTime QXmppArchiveRemoveIq::end() const
{
    return m_end;
}

/// Sets the end date/time for the archived conversations.
///
/// \param end

void QXmppArchiveRemoveIq::setEnd(const QDateTime &end)
{
    m_end = end;
}

/// \cond
bool QXmppArchiveRemoveIq::isArchiveRemoveIq(const QDomElement &element)
{
    QDomElement retrieveElement = element.firstChildElement("remove");
    return (retrieveElement.namespaceURI() == ns_archive);
}

void QXmppArchiveRemoveIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("remove");
    m_with = listElement.attribute("with");
    m_start = QXmppUtils::datetimeFromString(listElement.attribute("start"));
    m_end = QXmppUtils::datetimeFromString(listElement.attribute("end"));
}

void QXmppArchiveRemoveIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("remove");
    writer->writeAttribute("xmlns", ns_archive);
    if (!m_with.isEmpty())
        helperToXmlAddAttribute(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddAttribute(writer, "start", QXmppUtils::datetimeToString(m_start));
    if (m_end.isValid())
        helperToXmlAddAttribute(writer, "end", QXmppUtils::datetimeToString(m_end));
    writer->writeEndElement();
}
/// \endcond

QXmppArchiveRetrieveIq::QXmppArchiveRetrieveIq()
    : QXmppIq(QXmppIq::Get)
{
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

/// Returns the result set management query.
///
/// This is used for paging through messages.

QXmppResultSetQuery QXmppArchiveRetrieveIq::resultSetQuery() const
{
    return m_rsmQuery;
}

/// Sets the result set management query.
///
/// This is used for paging through messages.

void QXmppArchiveRetrieveIq::setResultSetQuery(const QXmppResultSetQuery& rsm)
{
    m_rsmQuery = rsm;
}

/// \cond
bool QXmppArchiveRetrieveIq::isArchiveRetrieveIq(const QDomElement &element)
{
    QDomElement retrieveElement = element.firstChildElement("retrieve");
    return (retrieveElement.namespaceURI() == ns_archive);
}

void QXmppArchiveRetrieveIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement retrieveElement = element.firstChildElement("retrieve");
    m_with = retrieveElement.attribute("with");
    m_start = QXmppUtils::datetimeFromString(retrieveElement.attribute("start"));

    m_rsmQuery.parse(retrieveElement);
}

void QXmppArchiveRetrieveIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("retrieve");
    writer->writeAttribute("xmlns", ns_archive);
    helperToXmlAddAttribute(writer, "with", m_with);
    helperToXmlAddAttribute(writer, "start", QXmppUtils::datetimeToString(m_start));
    if (!m_rsmQuery.isNull())
        m_rsmQuery.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
