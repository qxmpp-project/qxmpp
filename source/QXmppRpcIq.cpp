/*
 * Copyright (C) 2009-2010 Ian Reinhard Geiser
 *
 * Authors:
 *	Ian Reinhard Geiser
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

#include "QXmppConstants.h"
#include "QXmppRpcIq.h"
#include "QXmppUtils.h"
#include "xmlrpc.h"

#include <QDomElement>

QXmppRpcErrorIq::QXmppRpcErrorIq() : QXmppIq( QXmppIq::Error )
{

}

QXmppRpcInvokeIq QXmppRpcErrorIq::query() const
{
    return m_query;
}

void QXmppRpcErrorIq::setQuery(const QXmppRpcInvokeIq &query)
{
    m_query = query;
}

bool QXmppRpcErrorIq::isRpcErrorIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement errorElement = element.firstChildElement("error");
    QDomElement queryElement = element.firstChildElement("query");
    return (type == "error") &&
            !errorElement.isNull() &&
            queryElement.namespaceURI() == ns_rpc;
}

void QXmppRpcErrorIq::parseElementFromChild(const QDomElement &element)
{
    m_query.parseElementFromChild(element);
}

void QXmppRpcErrorIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    m_query.toXmlElementFromChild(writer);
}

QXmppRpcResponseIq::QXmppRpcResponseIq() : QXmppIq( QXmppIq::Result )
{
}

QVariantList QXmppRpcResponseIq::values() const
{
    return m_values;
}

void QXmppRpcResponseIq::setValues(const QVariantList &values)
{
    m_values = values;
}

bool QXmppRpcResponseIq::isRpcResponseIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement dataElement = element.firstChildElement("query");
    return dataElement.namespaceURI() == ns_rpc &&
           type == "result";
}

void QXmppRpcResponseIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    QDomElement methodElement = queryElement.firstChildElement("methodResponse");

    XMLRPC::ResponseMessage message;
    if (message.parse(methodElement))
        m_values = message.values();
}

void QXmppRpcResponseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_rpc);

    XMLRPC::ResponseMessage message;
    message.setValues(m_values);
    message.writeXml(writer);

    writer->writeEndElement();
}

QXmppRpcInvokeIq::QXmppRpcInvokeIq() : QXmppIq( QXmppIq::Set )
{
}

QVariantList QXmppRpcInvokeIq::arguments() const
{
    return m_arguments;
}

void QXmppRpcInvokeIq::setArguments(const QVariantList &arguments)
{
    m_arguments = arguments;
}

QString QXmppRpcInvokeIq::method() const
{
    return m_method;
}
void QXmppRpcInvokeIq::setMethod( const QString &method )
{
    m_method = method;
}

QString QXmppRpcInvokeIq::interface() const
{
    return m_interface;
}

void QXmppRpcInvokeIq::setInterface( const QString &interface )
{
    m_interface = interface;
}

bool QXmppRpcInvokeIq::isRpcInvokeIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement dataElement = element.firstChildElement("query");
    return dataElement.namespaceURI() == ns_rpc &&
           type == "set";
}

void QXmppRpcInvokeIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    QDomElement methodElement = queryElement.firstChildElement("methodCall");

    XMLRPC::RequestMessage message;
    if (message.parse(methodElement))
    {
        m_interface = message.method().split('.').value(0);
        m_method = message.method().split('.').value(1);
        m_arguments = message.arguments();
    }
}

void QXmppRpcInvokeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_rpc);

    QString methodName = m_interface + "." + m_method;
    XMLRPC::RequestMessage message;
    message.setMethod(methodName.toLatin1());
    message.setArguments(m_arguments);
    message.writeXml(writer);

    writer->writeEndElement();
}

