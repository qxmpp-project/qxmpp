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


#include "QXmppStanza.h"
#include "utils.h"
#include "QXmppConstants.h"

#include <QTextStream>

int QXmppStanza::s_uniqeIdNo = 0;

QXmppStanza::Error::Error(): m_type(static_cast<QXmppStanza::Error::Type>(-1)), 
            m_condition(static_cast<QXmppStanza::Error::Condition>(-1)), m_text("")
{
}

QXmppStanza::Error::Error(Type type, Condition cond, const QString& text): 
        m_type(type), m_condition(cond), m_text(text)
{
}

QXmppStanza::Error::Error(const QString& type, const QString& cond, const QString& text):
    m_text(text)
{
    setTypeFromStr(type);
    setConditionFromStr(cond);
}

void QXmppStanza::Error::setText(const QString& text)
{
    m_text = text;
}

void QXmppStanza::Error::setCondition(QXmppStanza::Error::Condition cond)
{
    m_condition = cond;
}

void QXmppStanza::Error::setType(QXmppStanza::Error::Type type)
{
    m_type = type;
}

QString QXmppStanza::Error::getText() const
{
    return m_text;
}

QXmppStanza::Error::Condition QXmppStanza::Error::getCondition() const
{
    return m_condition;
}

QXmppStanza::Error::Type QXmppStanza::Error::getType() const
{
    return m_type;
}

QString QXmppStanza::Error::getTypeStr() const
{
    switch(getType())
    {
    case Cancel:
        return "cancel";
    case Continue:
        return "continue";
    case Modify:
        return "modify";
    case Auth:
        return "auth";
    case Wait:
        return "wait";
    default:
        return "";
    }
}

QString QXmppStanza::Error::getConditionStr() const
{
    switch(getCondition())
    {
    case BadRequest:
        return "bad-request";
    case Conflict:
        return "conflict";
    case FeatureNotImplemented:
        return "feature-not-implemented";
    case Forbidden:
        return "forbidden";
    case Gone:
        return "gone";
    case InternalServerError:
        return "internal-server-error";
    case ItemNotFound:
        return "item-not-found";
    case JidMalformed:
        return "jid-malformed";
    case NotAcceptable:
        return "not-acceptable";
    case NotAllowed:
        return "not-allowed";
    case NotAuthorized:
        return "not-authorized";
    case PaymentRequired:
        return "payment-required";
    case RecipientUnavailable:
        return "recipient-unavailable";
    case Redirect:
        return "redirect";
    case RegistrationRequired:
        return "registration-required";
    case RemoteServerNotFound:
        return "remote-server-not-found";
    case RemoteServerTimeout:
        return "remote-server-timeout";
    case ResourceConstraint:
        return "resource-constraint";
    case ServiceUnavailable:
        return "service-unavailable";
    case SubscriptionRequired:
        return "subscription-required";
    case UndefinedCondition:
        return "undefined-condition";
    case UnexpectedRequest:
        return "unexpected-request";
    default:
        return "";
    }
}

void QXmppStanza::Error::setTypeFromStr(const QString& type)
{
    if(type == "cancel")
        setType(Cancel);
    else if(type == "continue")
        setType(Continue);
    else if(type == "modify")
        setType(Modify);
    else if(type == "auth")
        setType(Auth);
    else if(type == "wait")
        setType(Wait);
    else
        setType(static_cast<QXmppStanza::Error::Type>(-1));
}

void QXmppStanza::Error::setConditionFromStr(const QString& type)
{
    if(type == "bad-request")
        setCondition(BadRequest);
    else if(type == "conflict")
        setCondition(Conflict);
    else if(type == "feature-not-implemented")
        setCondition(FeatureNotImplemented);
    else if(type == "forbidden")
        setCondition(Forbidden);
    else if(type == "gone")
        setCondition(Gone);
    else if(type == "internal-server-error")
        setCondition(InternalServerError);
    else if(type == "item-not-found")
        setCondition(ItemNotFound);
    else if(type == "jid-malformed")
        setCondition(JidMalformed);
    else if(type == "not-acceptable")
        setCondition(NotAcceptable);
    else if(type == "not-allowed")
        setCondition(NotAllowed);
    else if(type == "not-authorized")
        setCondition(NotAuthorized);
    else if(type == "payment-required")
        setCondition(PaymentRequired);
    else if(type == "recipient-unavailable")
        setCondition(RecipientUnavailable);
    else if(type == "redirect")
        setCondition(Redirect);
    else if(type == "registration-required")
        setCondition(RegistrationRequired);
    else if(type == "remote-server-not-found")
        setCondition(RemoteServerNotFound);
    else if(type == "remote-server-timeout")
        setCondition(RemoteServerTimeout);
    else if(type == "resource-constraint")
        setCondition(ResourceConstraint);
    else if(type == "service-unavailable")
        setCondition(ServiceUnavailable);
    else if(type == "subscription-required")
        setCondition(SubscriptionRequired);
    else if(type == "undefined-condition")
        setCondition(UndefinedCondition);
    else if(type == "unexpected-request")
        setCondition(UnexpectedRequest);
    else
        setCondition(static_cast<QXmppStanza::Error::Condition>(-1));
}

QString QXmppStanza::Error::toXml() const
{
    QString data;
    QString cond = getConditionStr();
    QString type = getTypeStr();
    
    if(cond.isEmpty() && type.isEmpty())
        return data;

    QTextStream stream(&data);

    stream << "<error";
    helperToXmlAddAttribute(stream, "type", type);
    stream << ">";
    
    if(!cond.isEmpty())
    {
        stream << "<" << cond;
        helperToXmlAddAttribute(stream, "xmlns", ns_stanza);
        stream << "/>";
    }
    if(!m_text.isEmpty())
    {
        stream << "<text";
        helperToXmlAddAttribute(stream, "xml:lang", "en");
        helperToXmlAddAttribute(stream, "xmlns", ns_stanza);
        stream << ">";
        stream << m_text;
        stream << "</text>";
    }
    stream << "</error>";
    return data;
}


QXmppStanza::QXmppStanza(const QString& from, const QString& to) : QXmppPacket(),
m_to(to), m_from(from)
{
}

QXmppStanza::~QXmppStanza()
{

}

QString QXmppStanza::getTo() const
{
    return m_to;
}

QString QXmppStanza::getFrom() const
{
    return m_from;
}

QString QXmppStanza::getId() const    
{
    return m_id;
}

QString QXmppStanza::getLang() const    
{
    return m_lang;
}


void QXmppStanza::setTo(const QString& to)  
{
    m_to = to;
}

void QXmppStanza::setFrom(const QString& from)
{
    m_from = from;
}

void QXmppStanza::setId(const QString& id)    
{
    m_id = id;
}

void QXmppStanza::setLang(const QString& lang)    
{
    m_lang = lang;
}

void QXmppStanza::generateAndSetNextId()
{
    // get back
    ++s_uniqeIdNo;
    m_id = "qxmpp" + QString::number(s_uniqeIdNo);
}

QXmppStanza::Error QXmppStanza::getError() const
{
    return m_error;
}

void QXmppStanza::setError(QXmppStanza::Error& error)
{
    m_error = error;
}

bool QXmppStanza::isErrorStanza()
{
    return !(m_error.getTypeStr().isEmpty() && 
        m_error.getConditionStr().isEmpty());
}
