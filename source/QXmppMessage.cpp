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
#include "utils.h"
#include <QTextStream>

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
        qWarning("QXmppMessage::setTypeFromStr() invalid input string type: %s", qPrintable(str));
        return;
    }
}

QByteArray QXmppMessage::toXml() const
{
// yet to take care of escaping xml chars
// and what if there are multiple bodies in diff langs
// also error

    // why not qbytearray getback
    // or use bytearray without text stream..using append
    QString data;
    QTextStream stream(&data);

    stream << "<message";
    helperToXmlAddAttribute(stream, "xml:lang", getLang());
    helperToXmlAddAttribute(stream, "id", getId());
    helperToXmlAddAttribute(stream, "to", getTo());
    helperToXmlAddAttribute(stream, "from", getFrom());
    helperToXmlAddAttribute(stream, "type", getTypeStr());
    stream << ">";
    helperToXmlAddElement(stream, "subject", getSubject());
    helperToXmlAddElement(stream, "body", getBody());
    helperToXmlAddElement(stream, "thread", getThread());
    stream << getError().toXml();
    stream << "</message>";
    return data.toAscii();    
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

