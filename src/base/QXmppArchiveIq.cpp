// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppArchiveIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
void QXmppArchiveMessage::setBody(const QString &body)
{
    m_body = body;
}

/// Returns the archived message's date.
QDateTime QXmppArchiveMessage::date() const
{
    return m_date;
}

/// Sets the archived message's date.
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
    m_with = element.attribute(u"with"_s);
    m_start = QXmppUtils::datetimeFromString(element.attribute(u"start"_s));
    m_subject = element.attribute(u"subject"_s);
    m_thread = element.attribute(u"thread"_s);
    m_version = element.attribute(u"version"_s).toInt();

    QDateTime timeAccu = m_start;

    for (const auto &child : iterChildElements(element)) {
        if ((child.tagName() == u"from") || (child.tagName() == u"to")) {
            QXmppArchiveMessage message;
            message.setBody(firstChildElement(child, u"body").text());
            timeAccu = timeAccu.addSecs(child.attribute(u"secs"_s).toInt());
            message.setDate(timeAccu);
            message.setReceived(child.tagName() == u"from");
            m_messages << message;
        }
    }
}

void QXmppArchiveChat::toXml(QXmlStreamWriter *writer, const QXmppResultSetReply &rsm) const
{
    writer->writeStartElement(QSL65("chat"));
    writer->writeDefaultNamespace(toString65(ns_archive));
    writeOptionalXmlAttribute(writer, u"with", m_with);
    if (m_start.isValid()) {
        writeOptionalXmlAttribute(writer, u"start", QXmppUtils::datetimeToString(m_start));
    }
    writeOptionalXmlAttribute(writer, u"subject", m_subject);
    writeOptionalXmlAttribute(writer, u"thread", m_thread);
    if (m_version) {
        writeOptionalXmlAttribute(writer, u"version", QString::number(m_version));
    }

    QDateTime prevTime = m_start;

    for (const QXmppArchiveMessage &message : m_messages) {
        writer->writeStartElement(message.isReceived() ? u"from"_s : u"to"_s);
        writeOptionalXmlAttribute(writer, u"secs", QString::number(prevTime.secsTo(message.date())));
        writer->writeTextElement(QSL65("body"), message.body());
        writer->writeEndElement();
        prevTime = message.date();
    }
    if (!rsm.isNull()) {
        rsm.toXml(writer);
    }
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

///
/// Returns the result set management reply.
///
/// This is used for paging through messages.
///
QXmppResultSetReply QXmppArchiveChatIq::resultSetReply() const
{
    return m_rsmReply;
}

///
/// Sets the result set management reply.
///
/// This is used for paging through messages.
///
void QXmppArchiveChatIq::setResultSetReply(const QXmppResultSetReply &rsm)
{
    m_rsmReply = rsm;
}

/// \cond
bool QXmppArchiveChatIq::isArchiveChatIq(const QDomElement &element)
{
    auto chatEl = firstChildElement(element, u"chat", ns_archive);
    return !chatEl.isNull() && !chatEl.attribute(u"with"_s).isEmpty();
}

void QXmppArchiveChatIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement chatElement = firstChildElement(element, u"chat");
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
QString QXmppArchiveListIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
void QXmppArchiveListIq::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the start date/time for the archived conversations.
QDateTime QXmppArchiveListIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
void QXmppArchiveListIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the end date/time for the archived conversations.
QDateTime QXmppArchiveListIq::end() const
{
    return m_end;
}

/// Sets the end date/time for the archived conversations.
void QXmppArchiveListIq::setEnd(const QDateTime &end)
{
    m_end = end;
}

///
/// Returns the result set management query.
///
/// This is used for paging through conversations.
///
QXmppResultSetQuery QXmppArchiveListIq::resultSetQuery() const
{
    return m_rsmQuery;
}

///
/// Sets the result set management query.
///
/// This is used for paging through conversations.
///
void QXmppArchiveListIq::setResultSetQuery(const QXmppResultSetQuery &rsm)
{
    m_rsmQuery = rsm;
}

///
/// Returns the result set management reply.
///
/// This is used for paging through conversations.
///
QXmppResultSetReply QXmppArchiveListIq::resultSetReply() const
{
    return m_rsmReply;
}

///
/// Sets the result set management reply.
///
/// This is used for paging through conversations.
///
void QXmppArchiveListIq::setResultSetReply(const QXmppResultSetReply &rsm)
{
    m_rsmReply = rsm;
}

/// \cond
bool QXmppArchiveListIq::isArchiveListIq(const QDomElement &element)
{
    return isIqType(element, u"list", ns_archive);
}

void QXmppArchiveListIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement listElement = firstChildElement(element, u"list");
    m_with = listElement.attribute(u"with"_s);
    m_start = QXmppUtils::datetimeFromString(listElement.attribute(u"start"_s));
    m_end = QXmppUtils::datetimeFromString(listElement.attribute(u"end"_s));

    m_rsmQuery.parse(listElement);
    m_rsmReply.parse(listElement);

    for (const auto &child : iterChildElements(listElement, u"chat")) {
        QXmppArchiveChat chat;
        chat.parse(child);
        m_chats << chat;
    }
}

void QXmppArchiveListIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("list"));
    writer->writeDefaultNamespace(toString65(ns_archive));
    if (!m_with.isEmpty()) {
        writeOptionalXmlAttribute(writer, u"with", m_with);
    }
    if (m_start.isValid()) {
        writeOptionalXmlAttribute(writer, u"start", QXmppUtils::datetimeToString(m_start));
    }
    if (m_end.isValid()) {
        writeOptionalXmlAttribute(writer, u"end", QXmppUtils::datetimeToString(m_end));
    }
    if (!m_rsmQuery.isNull()) {
        m_rsmQuery.toXml(writer);
    } else if (!m_rsmReply.isNull()) {
        m_rsmReply.toXml(writer);
    }
    for (const auto &chat : m_chats) {
        chat.toXml(writer);
    }
    writer->writeEndElement();
}

bool QXmppArchivePrefIq::isArchivePrefIq(const QDomElement &element)
{
    return isIqType(element, u"pref", ns_archive);
}

void QXmppArchivePrefIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = firstChildElement(element, u"pref");
    Q_UNUSED(queryElement)
}

void QXmppArchivePrefIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("pref"));
    writer->writeDefaultNamespace(toString65(ns_archive));
    writer->writeEndElement();
}
/// \endcond

/// Returns the JID which archived conversations must match.
QString QXmppArchiveRemoveIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
void QXmppArchiveRemoveIq::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the start date/time for the archived conversations.
QDateTime QXmppArchiveRemoveIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
void QXmppArchiveRemoveIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the end date/time for the archived conversations.
QDateTime QXmppArchiveRemoveIq::end() const
{
    return m_end;
}

/// Sets the end date/time for the archived conversations.
void QXmppArchiveRemoveIq::setEnd(const QDateTime &end)
{
    m_end = end;
}

/// \cond
bool QXmppArchiveRemoveIq::isArchiveRemoveIq(const QDomElement &element)
{
    return isIqType(element, u"remove", ns_archive);
}

void QXmppArchiveRemoveIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement listElement = firstChildElement(element, u"remove");
    m_with = listElement.attribute(u"with"_s);
    m_start = QXmppUtils::datetimeFromString(listElement.attribute(u"start"_s));
    m_end = QXmppUtils::datetimeFromString(listElement.attribute(u"end"_s));
}

void QXmppArchiveRemoveIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("remove"));
    writer->writeDefaultNamespace(toString65(ns_archive));
    if (!m_with.isEmpty()) {
        writeOptionalXmlAttribute(writer, u"with", m_with);
    }
    if (m_start.isValid()) {
        writeOptionalXmlAttribute(writer, u"start", QXmppUtils::datetimeToString(m_start));
    }
    if (m_end.isValid()) {
        writeOptionalXmlAttribute(writer, u"end", QXmppUtils::datetimeToString(m_end));
    }
    writer->writeEndElement();
}
/// \endcond

QXmppArchiveRetrieveIq::QXmppArchiveRetrieveIq()
    : QXmppIq(QXmppIq::Get)
{
}

/// Returns the start date/time for the archived conversations.
QDateTime QXmppArchiveRetrieveIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived conversations.
void QXmppArchiveRetrieveIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the JID which archived conversations must match.
QString QXmppArchiveRetrieveIq::with() const
{
    return m_with;
}

/// Sets the JID which archived conversations must match.
void QXmppArchiveRetrieveIq::setWith(const QString &with)
{
    m_with = with;
}

///
/// Returns the result set management query.
///
/// This is used for paging through messages.
///
QXmppResultSetQuery QXmppArchiveRetrieveIq::resultSetQuery() const
{
    return m_rsmQuery;
}

///
/// Sets the result set management query.
///
/// This is used for paging through messages.
///
void QXmppArchiveRetrieveIq::setResultSetQuery(const QXmppResultSetQuery &rsm)
{
    m_rsmQuery = rsm;
}

/// \cond
bool QXmppArchiveRetrieveIq::isArchiveRetrieveIq(const QDomElement &element)
{
    return isIqType(element, u"retrieve", ns_archive);
}

void QXmppArchiveRetrieveIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement retrieveElement = firstChildElement(element, u"retrieve", ns_archive);
    m_with = retrieveElement.attribute(u"with"_s);
    m_start = QXmppUtils::datetimeFromString(retrieveElement.attribute(u"start"_s));

    m_rsmQuery.parse(retrieveElement);
}

void QXmppArchiveRetrieveIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("retrieve"));
    writer->writeDefaultNamespace(toString65(ns_archive));
    writeOptionalXmlAttribute(writer, u"with", m_with);
    writeOptionalXmlAttribute(writer, u"start", QXmppUtils::datetimeToString(m_start));
    if (!m_rsmQuery.isNull()) {
        m_rsmQuery.toXml(writer);
    }
    writer->writeEndElement();
}
/// \endcond
