/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#include "QXmppStanza.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QDomElement>
#include <QXmlStreamWriter>

uint QXmppStanza::s_uniqeIdNo = 0;

QXmppStanza::Error::Error():
    m_code(0),
    m_type(static_cast<QXmppStanza::Error::Type>(-1)),
    m_condition(static_cast<QXmppStanza::Error::Condition>(-1))
{
}

QXmppStanza::Error::Error(Type type, Condition cond, const QString& text): 
    m_code(0),
    m_type(type),
    m_condition(cond),
    m_text(text)
{
}

QXmppStanza::Error::Error(const QString& type, const QString& cond,
                          const QString& text):
    m_code(0),
    m_text(text)
{
    setTypeFromStr(type);
    setConditionFromStr(cond);
}

QString QXmppStanza::Error::text() const
{
    return m_text;
}

void QXmppStanza::Error::setText(const QString& text)
{
    m_text = text;
}

int QXmppStanza::Error::code() const
{
    return m_code;
}

void QXmppStanza::Error::setCode(int code)
{
    m_code = code;
}

QXmppStanza::Error::Condition QXmppStanza::Error::condition() const
{
    return m_condition;
}

void QXmppStanza::Error::setCondition(QXmppStanza::Error::Condition cond)
{
    m_condition = cond;
}

QXmppStanza::Error::Type QXmppStanza::Error::type() const
{
    return m_type;
}

void QXmppStanza::Error::setType(QXmppStanza::Error::Type type)
{
    m_type = type;
}

QString QXmppStanza::Error::getTypeStr() const
{
    switch(m_type)
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
    switch(m_condition)
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

bool QXmppStanza::Error::isValid()
{
    return !(getTypeStr().isEmpty() && getConditionStr().isEmpty());
}

void QXmppStanza::Error::parse(const QDomElement &errorElement)
{
    setCode(errorElement.attribute("code").toInt());
    setTypeFromStr(errorElement.attribute("type"));

    QString text;
    QString cond;
    QDomElement element = errorElement.firstChildElement();
    while(!element.isNull())
    {
        if(element.tagName() == "text")
            text = element.text();
        else if(element.namespaceURI() == ns_stanza)
        {
            cond = element.tagName();
        }        
        element = element.nextSiblingElement();
    }

    setConditionFromStr(cond);
    setText(text);
}

void QXmppStanza::Error::toXml( QXmlStreamWriter *writer ) const
{
    QString cond = getConditionStr();
    QString type = getTypeStr();
    
    if(cond.isEmpty() && type.isEmpty())
        return;

    writer->writeStartElement("error");
    helperToXmlAddAttribute(writer, "type", type);

    if (m_code > 0)
        helperToXmlAddAttribute(writer, "code", QString::number(m_code));

    if(!cond.isEmpty())
    {
        writer->writeStartElement(cond);
        writer->writeAttribute("xmlns", ns_stanza);
        writer->writeEndElement();
    }
    if(!m_text.isEmpty())
    {
        writer->writeStartElement("text");
        writer->writeAttribute("xml:lang", "en");
        writer->writeAttribute("xmlns", ns_stanza);
        writer->writeCharacters(m_text);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}

/// Constructs a QXmppStanza with the specified sender and recipient.
///
/// \param from
/// \param to

QXmppStanza::QXmppStanza(const QString& from, const QString& to)
    : QXmppPacket(),
    m_to(to),
    m_from(from)
{
}

/// Destroys a QXmppStanza.

QXmppStanza::~QXmppStanza()
{
}

/// Returns the stanza's recipient JID.
///

QString QXmppStanza::to() const
{
    return m_to;
}

/// Sets the stanza's recipient JID.
///
/// \param to

void QXmppStanza::setTo(const QString& to)  
{
    m_to = to;
}

/// Returns the stanza's sender JID.

QString QXmppStanza::from() const
{
    return m_from;
}

/// Sets the stanza's sender JID.
///
/// \param from

void QXmppStanza::setFrom(const QString& from)
{
    m_from = from;
}

/// Returns the stanza's identifier.

QString QXmppStanza::id() const    
{
    return m_id;
}

/// Sets the stanza's identifier.
///
/// \param id

void QXmppStanza::setId(const QString& id)    
{
    m_id = id;
}

/// Returns the stanza's language.

QString QXmppStanza::lang() const    
{
    return m_lang;
}

/// Sets the stanza's language.
///
/// \param lang

void QXmppStanza::setLang(const QString& lang)    
{
    m_lang = lang;
}

/// Returns the stanza's error.

QXmppStanza::Error QXmppStanza::error() const
{
    return m_error;
}

/// Sets the stanza's error.
///
/// \param error

void QXmppStanza::setError(const QXmppStanza::Error& error)
{
    m_error = error;
}

/// Returns the stanza's "extensions".
///
/// Extensions are XML elements which are not handled internally by QXmpp.

QXmppElementList QXmppStanza::extensions() const
{
    return m_extensions;
}

/// Sets the stanza's "extensions".
///
/// \param extensions

void QXmppStanza::setExtensions(const QXmppElementList &extensions)
{
    m_extensions = extensions;
}

void QXmppStanza::generateAndSetNextId()
{
    // get back
    ++s_uniqeIdNo;
    m_id = "qxmpp" + QString::number(s_uniqeIdNo);
}

bool QXmppStanza::isErrorStanza()
{
    return m_error.isValid();
}

void QXmppStanza::parse(const QDomElement &element)
{
    m_from = element.attribute("from");
    m_to = element.attribute("to");
    m_id = element.attribute("id");
    m_lang = element.attribute("lang");

    QDomElement errorElement = element.firstChildElement("error");
    if(!errorElement.isNull())
        m_error.parse(errorElement);
}

// deprecated

QString QXmppStanza::Error::getText() const
{
    return m_text;
}

int QXmppStanza::Error::getCode() const
{
    return m_code;
}

QXmppStanza::Error::Condition QXmppStanza::Error::getCondition() const
{
    return m_condition;
}

QXmppStanza::Error::Type QXmppStanza::Error::getType() const
{
    return m_type;
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

QXmppStanza::Error QXmppStanza::getError() const
{
    return m_error;
}

