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


#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include <QXmlStreamWriter>

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const 
                         QString& body, const QString& thread)
    : QXmppStanza(from, to), m_type(Chat), m_body(body), m_thread(thread)
{
}

QXmppMessage::~QXmppMessage()
{

}

QXmppMessage::Type QXmppMessage::getType() const
{
    return m_type;
}

QString QXmppMessage::getTypeStr() const
{
    switch(getType())
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
        qWarning("QXmppMessage::getTypeStr() invalid type %d", (int)getType());
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

void QXmppMessage::toXml(QXmlStreamWriter *xmlWriter) const
{

    xmlWriter->writeStartElement("message");
    helperToXmlAddAttribute(xmlWriter, "xml:lang", getLang());
    helperToXmlAddAttribute(xmlWriter,  "id", getId());
    helperToXmlAddAttribute(xmlWriter, "to", getTo());
    helperToXmlAddAttribute(xmlWriter, "from", getFrom());
    helperToXmlAddAttribute(xmlWriter,  "type", getTypeStr());
    if (!getSubject().isEmpty())
        helperToXmlAddTextElement(xmlWriter, "subject", getSubject());
    helperToXmlAddTextElement(xmlWriter,"body", getBody());
    if (!getThread().isEmpty())
        helperToXmlAddTextElement(xmlWriter,"thread", getThread());
    getError().toXml(xmlWriter);
    xmlWriter->writeEndElement();
}

QString QXmppMessage::getBody() const
{
    return m_body;
}

void QXmppMessage::setBody(const QString& body)
{
    m_body = body;
}

QString QXmppMessage::getSubject() const
{
    return m_subject;
}

void QXmppMessage::setSubject(const QString& sub)
{
    m_subject = sub;
}

QString QXmppMessage::getThread() const
{
    return m_thread;
}

void QXmppMessage::setThread(const QString& thread)
{
    m_thread = thread;
}

