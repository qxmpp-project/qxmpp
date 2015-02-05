/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *  Georg Rudoy
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


#include "QXmppStanza.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

#include <QDomElement>
#include <QXmlStreamWriter>

uint QXmppStanza::s_uniqeIdNo = 0;

class QXmppExtendedAddressPrivate : public QSharedData
{
public:
    bool delivered;
    QString description;
    QString jid;
    QString type;
};

/// Constructs an empty extended address.

QXmppExtendedAddress::QXmppExtendedAddress()
    : d(new QXmppExtendedAddressPrivate())
{
    d->delivered = false;
}

/// Constructs a copy of other.
///
/// \param other
///
QXmppExtendedAddress::QXmppExtendedAddress(const QXmppExtendedAddress &other)
    : d(other.d)
{
}

QXmppExtendedAddress::~QXmppExtendedAddress()
{
}

/// Assigns the other address to this one.
///
/// \param other
///
QXmppExtendedAddress& QXmppExtendedAddress::operator=(const QXmppExtendedAddress& other)
{
    d = other.d;
    return *this;
}

/// Returns the human-readable description of the address.

QString QXmppExtendedAddress::description() const
{
    return d->description;
}

/// Sets the human-readable \a description of the address.

void QXmppExtendedAddress::setDescription(const QString &description)
{
    d->description = description;
}

/// Returns the JID of the address.

QString QXmppExtendedAddress::jid() const
{
    return d->jid;
}

/// Sets the JID of the address.

void QXmppExtendedAddress::setJid(const QString &jid)
{
    d->jid = jid;
}

/// Returns the type of the address.

QString QXmppExtendedAddress::type() const
{
    return d->type;
}

/// Sets the \a type of the address.

void QXmppExtendedAddress::setType(const QString &type)
{
    d->type = type;
}

/// Returns whether the stanza has been delivered to this address.

bool QXmppExtendedAddress::isDelivered() const
{
    return d->delivered;
}

/// Sets whether the stanza has been \a delivered to this address.

void QXmppExtendedAddress::setDelivered(bool delivered)
{
    d->delivered = delivered;
}

/// Checks whether this address is valid. The extended address is considered
/// to be valid if at least type and JID fields are non-empty.

bool QXmppExtendedAddress::isValid() const
{
    return !d->type.isEmpty() && !d->jid.isEmpty();
}

/// \cond
void QXmppExtendedAddress::parse(const QDomElement &element)
{
    d->delivered = element.attribute("delivered") == "true";
    d->description = element.attribute("desc");
    d->jid = element.attribute("jid");
    d->type = element.attribute("type");
}

void QXmppExtendedAddress::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("address");
    if (d->delivered)
        xmlWriter->writeAttribute("delivered", "true");
    if (!d->description.isEmpty())
        xmlWriter->writeAttribute("desc", d->description);
    xmlWriter->writeAttribute("jid", d->jid);
    xmlWriter->writeAttribute("type", d->type);
    xmlWriter->writeEndElement();
}
/// \endcond

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

/// \cond
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
    case BadAuth:
        return "bad-auth";
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
    else if(type == "bad-auth")
        setCondition(BadAuth);
    else
        setCondition(static_cast<QXmppStanza::Error::Condition>(-1));
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
/// \endcond

class QXmppStanzaPrivate : public QSharedData
{
public:
    QString to;
    QString from;
    QString id;
    QString lang;
    QXmppStanza::Error error;
    QXmppElementList extensions;
    QList<QXmppExtendedAddress> extendedAddresses;
};

/// Constructs a QXmppStanza with the specified sender and recipient.
///
/// \param from
/// \param to

QXmppStanza::QXmppStanza(const QString& from, const QString& to)
    : d(new QXmppStanzaPrivate)
{
    d->to = to;
    d->from = from;
}

/// Constructs a copy of \a other.

QXmppStanza::QXmppStanza(const QXmppStanza &other)
    : d(other.d)
{
}

/// Destroys a QXmppStanza.

QXmppStanza::~QXmppStanza()
{
}

/// Assigns \a other to this stanza.

QXmppStanza& QXmppStanza::operator=(const QXmppStanza &other)
{
    d = other.d;
    return *this;
}

/// Returns the stanza's recipient JID.
///

QString QXmppStanza::to() const
{
    return d->to;
}

/// Sets the stanza's recipient JID.
///
/// \param to

void QXmppStanza::setTo(const QString& to)
{
    d->to = to;
}

/// Returns the stanza's sender JID.

QString QXmppStanza::from() const
{
    return d->from;
}

/// Sets the stanza's sender JID.
///
/// \param from

void QXmppStanza::setFrom(const QString& from)
{
    d->from = from;
}

/// Returns the stanza's identifier.

QString QXmppStanza::id() const
{
    return d->id;
}

/// Sets the stanza's identifier.
///
/// \param id

void QXmppStanza::setId(const QString& id)
{
    d->id = id;
}

/// Returns the stanza's language.

QString QXmppStanza::lang() const
{
    return d->lang;
}

/// Sets the stanza's language.
///
/// \param lang

void QXmppStanza::setLang(const QString& lang)
{
    d->lang = lang;
}

/// Returns the stanza's error.

QXmppStanza::Error QXmppStanza::error() const
{
    return d->error;
}

/// Sets the stanza's error.
///
/// \param error

void QXmppStanza::setError(const QXmppStanza::Error& error)
{
    d->error = error;
}

/// Returns the stanza's "extensions".
///
/// Extensions are XML elements which are not handled internally by QXmpp.

QXmppElementList QXmppStanza::extensions() const
{
    return d->extensions;
}

/// Sets the stanza's "extensions".
///
/// \param extensions

void QXmppStanza::setExtensions(const QXmppElementList &extensions)
{
    d->extensions = extensions;
}

/// Returns the stanza's extended addresses as defined by
/// XEP-0033: Extended Stanza Addressing.

QList<QXmppExtendedAddress> QXmppStanza::extendedAddresses() const
{
    return d->extendedAddresses;
}

/// Sets the stanza's extended addresses as defined by
/// XEP-0033: Extended Stanza Addressing.

void QXmppStanza::setExtendedAddresses(const QList<QXmppExtendedAddress> &addresses)
{
    d->extendedAddresses = addresses;
}

/// \cond
void QXmppStanza::generateAndSetNextId()
{
    // get back
    ++s_uniqeIdNo;
    d->id = "qxmpp" + QString::number(s_uniqeIdNo);
}

void QXmppStanza::parse(const QDomElement &element)
{
    d->from = element.attribute("from");
    d->to = element.attribute("to");
    d->id = element.attribute("id");
    d->lang = element.attribute("lang");

    QDomElement errorElement = element.firstChildElement("error");
    if(!errorElement.isNull())
        d->error.parse(errorElement);

    // XEP-0033: Extended Stanza Addressing
    QDomElement addressElement = element.firstChildElement("addresses").firstChildElement("address");
    while (!addressElement.isNull()) {
        QXmppExtendedAddress address;
        address.parse(addressElement);
        if (address.isValid())
            d->extendedAddresses << address;
        addressElement = addressElement.nextSiblingElement("address");
    }
}

void QXmppStanza::extensionsToXml(QXmlStreamWriter *xmlWriter) const
{
    // XEP-0033: Extended Stanza Addressing
    if (!d->extendedAddresses.isEmpty()) {
        xmlWriter->writeStartElement("addresses");
        xmlWriter->writeAttribute("xmlns", ns_extended_addressing);
        foreach (const QXmppExtendedAddress &address, d->extendedAddresses)
            address.toXml(xmlWriter);
        xmlWriter->writeEndElement();
    }

    // other extensions
    foreach (const QXmppElement &extension, d->extensions)
        extension.toXml(xmlWriter);
}

/// \endcond
