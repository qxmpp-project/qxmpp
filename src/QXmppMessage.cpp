/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

/// Constructs a QXmppMessage.
///
/// \param from
/// \param to
/// \param body
/// \param thread

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const
                         QString& body, const QString& thread)
    : QXmppStanza(from, to),
      m_type(Chat),
      m_stampType(QXmppMessage::DelayedDelivery),
      m_state(None),
      m_attentionRequested(false),
      m_body(body),
      m_thread(thread)
{
}

QXmppMessage::~QXmppMessage()
{

}

/// Returns the message's body.
///

QString QXmppMessage::body() const
{
    return m_body;
}

/// Sets the message's body.
///
/// \param body

void QXmppMessage::setBody(const QString& body)
{
    m_body = body;
}

/// Returns true if the user's attention is requested, as defined
/// by XEP-0224: Attention.

bool QXmppMessage::isAttentionRequested() const
{
    return m_attentionRequested;
}

/// Sets whether the user's attention is requested, as defined
/// by XEP-0224: Attention.
///
/// \a param requested

void QXmppMessage::setAttentionRequested(bool requested)
{
    m_attentionRequested = requested;
}

/// Returns the message's type.
///

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

/// Sets the message's type.
///
/// \param type

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

/// Returns the message's timestamp (if any).

QDateTime QXmppMessage::stamp() const
{
    return m_stamp;
}

/// Sets the message's timestamp.
///
/// \param stamp

void QXmppMessage::setStamp(const QDateTime &stamp)
{
    m_stamp = stamp;
}

/// Returns the message's chat state.
///

QXmppMessage::State QXmppMessage::state() const
{
    return m_state;
}

/// Sets the message's chat state.
///
/// \param state

void QXmppMessage::setState(QXmppMessage::State state)
{
    m_state = state;
}

void QXmppMessage::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));
    m_body = element.firstChildElement("body").text();
    m_subject = element.firstChildElement("subject").text();
    m_thread = element.firstChildElement("thread").text();

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

    // XEP-0203: Delayed Delivery
    QDomElement delayElement = element.firstChildElement("delay");
    if (!delayElement.isNull() && delayElement.namespaceURI() == ns_delayed_delivery)
    {
        const QString str = delayElement.attribute("stamp");
        m_stamp = datetimeFromString(str);
        m_stampType = QXmppMessage::DelayedDelivery;
    }

    // XEP-0224: Attention
    m_attentionRequested = element.firstChildElement("attention").namespaceURI() == ns_attention;

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement("x");
    while (!xElement.isNull())
    {
        if (xElement.namespaceURI() == ns_legacy_delayed_delivery)
        {
            // XEP-0091: Legacy Delayed Delivery
            const QString str = xElement.attribute("stamp");
            m_stamp = QDateTime::fromString(str, "yyyyMMddThh:mm:ss");
            m_stamp.setTimeSpec(Qt::UTC);
            m_stampType = QXmppMessage::LegacyDelayedDelivery;
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
        xmlWriter->writeAttribute("xmlns", ns_chat_states);
        xmlWriter->writeEndElement();
    }

    // time stamp
    if (m_stamp.isValid())
    {
        QDateTime utcStamp = m_stamp.toUTC();
        if (m_stampType == QXmppMessage::DelayedDelivery)
        {
            // XEP-0203: Delayed Delivery
            xmlWriter->writeStartElement("delay");
            xmlWriter->writeAttribute("xmlns", ns_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", datetimeToString(utcStamp));
            xmlWriter->writeEndElement();
        } else {
            // XEP-0091: Legacy Delayed Delivery
            xmlWriter->writeStartElement("x");
            xmlWriter->writeAttribute("xmlns", ns_legacy_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", utcStamp.toString("yyyyMMddThh:mm:ss"));
            xmlWriter->writeEndElement();
        }
    }

    // XEP-0224: Attention
    if (m_attentionRequested) {
        xmlWriter->writeStartElement("attention");
        xmlWriter->writeAttribute("xmlns", ns_attention);
        xmlWriter->writeEndElement();
    }

    // other extensions
    foreach (const QXmppElement &extension, extensions())
        extension.toXml(xmlWriter);
    xmlWriter->writeEndElement();
}

/// Returns the message's subject.
///

QString QXmppMessage::subject() const
{
    return m_subject;
}

/// Sets the message's subject.
///
/// \param subject

void QXmppMessage::setSubject(const QString& subject)
{
    m_subject = subject;
}

/// Returns the message's thread.

QString QXmppMessage::thread() const
{
    return m_thread;
}

/// Sets the message's thread.
///
/// \param thread

void QXmppMessage::setThread(const QString& thread)
{
    m_thread = thread;
}

