/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#include "QXmppPresence.h"
#include "QXmppUtils.h"
#include <QtDebug>
#include <QDomElement>
#include <QXmlStreamWriter>

QXmppPresence::QXmppPresence(QXmppPresence::Type type,
                             const QXmppPresence::Status& status)
    : QXmppStanza(), m_type(type), m_status(status)
{

}

QXmppPresence::~QXmppPresence()
{

}

QXmppPresence::Type QXmppPresence::type() const
{
    return m_type;
}

void QXmppPresence::setType(QXmppPresence::Type type)
{
    m_type = type;
}

const QXmppPresence::Status& QXmppPresence::status() const
{
    return m_status;
}

QXmppPresence::Status& QXmppPresence::status()
{
    return m_status;
}

void QXmppPresence::setStatus(const QXmppPresence::Status& status)
{
    m_status = status;
}

void QXmppPresence::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));
    m_status.parse(element);

    QDomElement xElement = element.firstChildElement("x");
    if(!xElement.isNull())
        setExtensions(QXmppElement(xElement));
}

void QXmppPresence::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("presence");
    helperToXmlAddAttribute(xmlWriter,"xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter,"id", id());
    helperToXmlAddAttribute(xmlWriter,"to", to());
    helperToXmlAddAttribute(xmlWriter,"from", from());
    helperToXmlAddAttribute(xmlWriter,"type", getTypeStr());
    m_status.toXml(xmlWriter);

    error().toXml(xmlWriter);
    foreach (const QXmppElement &extension, extensions())
        extension.toXml(xmlWriter);
    
    xmlWriter->writeEndElement();
}

QString QXmppPresence::getTypeStr() const
{
    QString text;
    switch(m_type)
    {
    case QXmppPresence::Error:
        text = "error"; 
        break;
    case QXmppPresence::Available:
        // no type-attribute if available
        text = ""; 
        break;
    case QXmppPresence::Unavailable:
        text = "unavailable"; 
        break;
    case QXmppPresence::Subscribe:
        text = "subscribe"; 
        break;
    case QXmppPresence::Subscribed:
        text = "subscribed"; 
        break;
    case QXmppPresence::Unsubscribe:
        text = "unsubscribe"; 
        break;
    case QXmppPresence::Unsubscribed:
        text = "unsubscribed"; 
        break;
    case QXmppPresence::Probe:
        text = "probe"; 
        break;
    default:
        qWarning("QXmppPresence::getTypeStr() invalid type %d", (int)m_type);
        break;
    }
    return text;
}

void QXmppPresence::setTypeFromStr(const QString& str)
{
    QXmppPresence::Type type;
    if(str == "error")
    {
        type = QXmppPresence::Error;
        setType(type);
        return;
    }
    else if(str == "unavailable")
    {
        type = QXmppPresence::Unavailable;
        setType(type);
        return;
    }
    else if(str == "subscribe")
    {
        type = QXmppPresence::Subscribe;
        setType(type);
        return;
    }
    else if(str == "subscribed")
    {
        type = QXmppPresence::Subscribed;
        setType(type);
        return;
    }
    else if(str == "unsubscribe")
    {
        type = QXmppPresence::Unsubscribe;
        setType(type);
        return;
    }
    else if(str == "unsubscribed")
    {
        type = QXmppPresence::Unsubscribed;
        setType(type);
        return;
    }
    else if(str == "probe")
    {
        type = QXmppPresence::Probe;
        setType(type);
        return;
    }
    else if(str == "")
    {
        type = QXmppPresence::Available;
        setType(type);
        return;
    }
    else
    {
        type = static_cast<QXmppPresence::Type>(-1);
        qWarning("QXmppPresence::setTypeFromStr() invalid input string type: %s",
                 qPrintable(str));
        setType(type);
        return;
    }
}

QXmppPresence::Status::Status(QXmppPresence::Status::Type type,
                             const QString statusText, int priority) :
                                m_type(type),
                                m_statusText(statusText), m_priority(priority)
{
}

QXmppPresence::Status::Type QXmppPresence::Status::type() const
{
    return m_type;
}

void QXmppPresence::Status::setType(QXmppPresence::Status::Type type)
{
    m_type = type;
}

void QXmppPresence::Status::setTypeFromStr(const QString& str)
{
    // there is no keyword for Offline

    QXmppPresence::Status::Type type;
    if(str == "")   // not type-attribute means online
    {
        type = QXmppPresence::Status::Online;
        setType(type);
        return;
    }
    else if(str == "away")
    {
        type = QXmppPresence::Status::Away;
        setType(type);
        return;
    }
    else if(str == "xa")
    {
        type = QXmppPresence::Status::XA;
        setType(type);
        return;
    }
    else if(str == "dnd")
    {
        type = QXmppPresence::Status::DND;
        setType(type);
        return;
    }
    else if(str == "chat")
    {
        type = QXmppPresence::Status::Chat;
        setType(type);
        return;
    }
    else
    {
        type = static_cast<QXmppPresence::Status::Type>(-1);
        qWarning("QXmppPresence::Status::setTypeFromStr() invalid input string type %s", 
            qPrintable(str));
        setType(type);
    }
}

QString QXmppPresence::Status::getTypeStr() const
{
    QString text;
    switch(m_type)
    {
    case QXmppPresence::Status::Online:
        // no type-attribute if available
        text = ""; 
        break;
    case QXmppPresence::Status::Offline:
        text = ""; 
        break;
    case QXmppPresence::Status::Away:
        text = "away"; 
        break;
    case QXmppPresence::Status::XA:
        text = "xa"; 
        break;
    case QXmppPresence::Status::DND:
        text = "dnd"; 
        break;
    case QXmppPresence::Status::Chat:
        text = "chat"; 
        break;
    default:
        qWarning("QXmppPresence::Status::getTypeStr() invalid type %d",
                 (int)m_type);
        break;
    }
    return text;
}

QString QXmppPresence::Status::statusText() const
{
    return m_statusText;
}

void QXmppPresence::Status::setStatusText(const QString& str)
{
    m_statusText = str;
}

int QXmppPresence::Status::priority() const
{
    return m_priority;
}

void QXmppPresence::Status::setPriority(int priority)
{
    m_priority = priority;
}

void QXmppPresence::Status::parse(const QDomElement &element)
{
    setTypeFromStr(element.firstChildElement("show").text());
    m_statusText = element.firstChildElement("status").text();
    m_priority = element.firstChildElement("priority").text().toInt();
}

void QXmppPresence::Status::toXml(QXmlStreamWriter *xmlWriter) const
{
    const QString show = getTypeStr();
    if (!show.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "show", getTypeStr());
    if (!m_statusText.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "status", m_statusText);
    if (m_priority != 0)
        helperToXmlAddNumberElement(xmlWriter, "priority", m_priority);
}

/// \cond

QXmppPresence::Type QXmppPresence::getType() const
{
    return m_type;
}

const QXmppPresence::Status& QXmppPresence::getStatus() const
{
    return m_status;
}

QXmppPresence::Status& QXmppPresence::getStatus()
{
    return m_status;
}

QXmppPresence::Status::Type QXmppPresence::Status::getType() const
{
    return m_type;
}

QString QXmppPresence::Status::getStatusText() const
{
    return m_statusText;
}

int QXmppPresence::Status::getPriority() const
{
    return m_priority;
}

/// \endcond
