/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Authors:
 *	Manjeet Dahiya
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
#include <QXmlStreamWriter>

#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

static const char* chat_states[] = {
    "",
    "active",
    "inactive",
    "gone",
    "composing",
    "paused",
};

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const 
                         QString& body, const QString& thread)
    : QXmppStanza(from, to), m_type(Chat), m_state(None), m_body(body), m_thread(thread)
{
}

QXmppMessage::~QXmppMessage()
{

}

QXmppMessage::Type QXmppMessage::type() const
{
    return m_type;
}

QString QXmppMessage::getTypeStr() const
{
    switch(m_type)
    {
    case QXmppMessage::Error:
        return "error";
    case QXmppMessage::Normal:
        return "normal";
    case QXmppMessage::Chat:
        return "chat";
    case QXmppMessage::GroupChat:
        return "groupchat";
    case QXmppMessage::Headline:
        return "headline";
    default:
        qWarning("QXmppMessage::getTypeStr() invalid type %d", (int)m_type);
        return "";
    }
}

void QXmppMessage::setType(QXmppMessage::Type type)
{
    m_type = type;
}

void QXmppMessage::setTypeFromStr(const QString& str)
{
    if(str == "error")
    {
        setType(QXmppMessage::Error);
        return;
    }
    else if(str == "")   // if no type is specified 
    {
        setType(QXmppMessage::Normal);
        return;
    }
    else if(str == "normal")
    {
        setType(QXmppMessage::Normal);
        return;
    }
    else if(str == "chat")
    {
        setType(QXmppMessage::Chat);
        return;
    }
    else if(str == "groupchat")
    {
        setType(QXmppMessage::GroupChat);
        return;
    }
    else if(str == "headline")
    {
        setType(QXmppMessage::Headline);
        return;
    }
    else
    {
        setType(static_cast<QXmppMessage::Type>(-1));
        qWarning("QXmppMessage::setTypeFromStr() invalid input string type: %s",
                 qPrintable(str));
        return;
    }
}

/// Returns the message timestamp (if any).
///
/// XEP-0091: Legacy Delayed Delivery

QDateTime QXmppMessage::stamp() const
{
    return m_stamp;
}

/// Sets the message timestamp.
///
/// XEP-0091: Legacy Delayed Delivery

void QXmppMessage::setStamp(const QDateTime &stamp)
{
    m_stamp = stamp;
}

QXmppMessage::State QXmppMessage::state() const
{
    return m_state;
}

void QXmppMessage::setState(QXmppMessage::State state)
{
    m_state = state;
}

void QXmppMessage::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));
    setBody(unescapeString(
            element.firstChildElement("body").text()));
    setSubject(unescapeString(
            element.firstChildElement("subject").text()));
    setThread(element.firstChildElement("thread").text());

    // chat states
    for (int i = Active; i <= Paused; i++)
    {
        QDomElement stateElement = element.firstChildElement(chat_states[i]);
        if (!stateElement.isNull() &&
            stateElement.namespaceURI() == ns_chat_states)
        {
            m_state = static_cast<QXmppMessage::State>(i);
            break;
        }
    }

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement("x");
    while (!xElement.isNull())
    {
        if (xElement.namespaceURI() == ns_delay)
        {
            // XEP-0091: Legacy Delayed Delivery
            const QString str = xElement.attribute("stamp");
            m_stamp = QDateTime::fromString(str, "yyyyMMddThh:mm:ss");
            m_stamp.setTimeSpec(Qt::UTC);
        } else {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement("x");
    }
    setExtensions(extensions);
}

void QXmppMessage::toXml(QXmlStreamWriter *xmlWriter) const
{

    xmlWriter->writeStartElement("message");
    helperToXmlAddAttribute(xmlWriter, "xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", getTypeStr());
    if (!m_subject.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "subject", m_subject);
    if (!m_body.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "body", m_body);
    if (!m_thread.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "thread", m_thread);
    error().toXml(xmlWriter);

    // chat states
    if (m_state > None && m_state <= Paused)
    {
        xmlWriter->writeStartElement(chat_states[m_state]);
        helperToXmlAddAttribute(xmlWriter, "xmlns", ns_chat_states);
        xmlWriter->writeEndElement();
    }

    // XEP-0091: Legacy Delayed Delivery
    if (m_stamp.isValid())
    {
        QDateTime utcStamp = m_stamp.toUTC();
        xmlWriter->writeStartElement("x");
        helperToXmlAddAttribute(xmlWriter, "xmlns", ns_delay);
        helperToXmlAddAttribute(xmlWriter, "stamp", utcStamp.toString("yyyyMMddThh:mm:ss"));
        xmlWriter->writeEndElement();
    }

    // other extensions
    foreach (const QXmppElement &extension, extensions())
        extension.toXml(xmlWriter);
    xmlWriter->writeEndElement();
}

QString QXmppMessage::body() const
{
    return m_body;
}

void QXmppMessage::setBody(const QString& body)
{
    m_body = body;
}

QString QXmppMessage::subject() const
{
    return m_subject;
}

void QXmppMessage::setSubject(const QString& sub)
{
    m_subject = sub;
}

QString QXmppMessage::thread() const
{
    return m_thread;
}

void QXmppMessage::setThread(const QString& thread)
{
    m_thread = thread;
}

// deprecated

QXmppMessage::Type QXmppMessage::getType() const
{
    return m_type;
}

QXmppMessage::State QXmppMessage::getState() const
{
    return m_state;
}

QString QXmppMessage::getBody() const
{
    return m_body;
}

QString QXmppMessage::getSubject() const
{
    return m_subject;
}

QString QXmppMessage::getThread() const
{
    return m_thread;
}
