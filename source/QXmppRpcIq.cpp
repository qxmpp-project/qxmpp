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

#include <QDebug>
#include <QDomElement>
#include <QMap>
#include <QVariant>
#include <QDateTime>
#include <QStringList>

#include "QXmppConstants.h"
#include "QXmppRpcIq.h"
#include "QXmppUtils.h"

void XMLRPC::marshall(QXmlStreamWriter *writer, const QVariant &value)
{
    writer->writeStartElement("value");
    switch( value.type() )
    {
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            writer->writeTextElement("i4", value.toString());
            break;
        case QVariant::Double:
            writer->writeTextElement("double", value.toString());
            break;
        case QVariant::Bool:
            writer->writeTextElement("boolean", value.toBool() ? "1" : "0");
            break;
        case QVariant::Date:
            writer->writeTextElement("dateTime.iso8601", value.toDate().toString( Qt::ISODate ) );
            break;
        case QVariant::DateTime:
            writer->writeTextElement("dateTime.iso8601", value.toDateTime().toString( Qt::ISODate ) );
            break;
        case QVariant::Time:
            writer->writeTextElement("dateTime.iso8601", value.toTime().toString( Qt::ISODate ) );
            break;
        case QVariant::StringList:
        case QVariant::List:
        {
            writer->writeStartElement("array");
            writer->writeStartElement("data");
            foreach(const QVariant &item, value.toList())
                marshall(writer, item);
            writer->writeEndElement();
            writer->writeEndElement();
            break;
        }
        case QVariant::Map:
        {
            writer->writeStartElement("struct");
            QMap<QString, QVariant> map = value.toMap();
            QMap<QString, QVariant>::ConstIterator index = map.begin();
            while( index != map.end() )
            {
                writer->writeStartElement("member");
                writer->writeTextElement("name", index.key());
                marshall( writer, *index );
                writer->writeEndElement();
                ++index;
            }
            writer->writeEndElement();
            break;
        }
        case QVariant::ByteArray:
        {
            writer->writeTextElement("base64", value.toByteArray().toBase64() );
            break;
        }
        default:
        {
            if (value.isNull())
                writer->writeEmptyElement("nil");
            else if( value.canConvert(QVariant::String) )
            {
                writer->writeTextElement("string", value.toString() );
            }
            break;
        }
    }
    writer->writeEndElement();
}

QVariant XMLRPC::demarshall(const QDomElement &elem, QStringList &errors)
{
    if ( elem.tagName().toLower() != "value" )
    {
        errors << "Bad param value";
        return QVariant();
    }

    if ( !elem.firstChild().isElement() )
    {
        return QVariant( elem.text() );
    }

    const QDomElement typeData = elem.firstChild().toElement();
    const QString typeName = typeData.tagName().toLower();

    if (typeName == "nil")
    {
        return QVariant();
    }
    if ( typeName == "string" )
    {
        return QVariant( typeData.text() );
    }
    else if (typeName == "int" || typeName == "i4" )
    {
        bool ok = false;
        QVariant val( typeData.text().toInt( &ok ) );
        if (ok)
            return val;
        errors << "I was looking for an integer but data was courupt";
        return QVariant();
    }
    else if( typeName == "double" )
    {
        bool ok = false;
        QVariant val( typeData.text().toDouble( &ok ) );
        if (ok)
            return val;
        errors <<  "I was looking for an double but data was corrupt";
    }
    else if( typeName == "boolean" )
        return QVariant( typeData.text() == "1" || typeData.text().toLower() == "true" );
    else if( typeName == "datetime" || typeName == "datetime.iso8601" )
        return QVariant( QDateTime::fromString( typeData.text(), Qt::ISODate ) );
    else if( typeName == "array" )
    {
        QVariantList arr;
        QDomElement valueNode = typeData.firstChildElement("data").firstChildElement();
        while (!valueNode.isNull() && errors.isEmpty())
        {
            arr.append(demarshall(valueNode, errors));
            valueNode = valueNode.nextSiblingElement();
        }
        return QVariant( arr );
    }
    else if( typeName == "struct" )
    {
        QMap<QString,QVariant> stct;
        QDomNode valueNode = typeData.firstChild();
        while(!valueNode.isNull() && errors.isEmpty())
        {
            const QDomElement memberNode = valueNode.toElement().elementsByTagName("name").item(0).toElement();
            const QDomElement dataNode = valueNode.toElement().elementsByTagName("value").item(0).toElement();
            stct[ memberNode.text() ] = demarshall(dataNode, errors);
            valueNode = valueNode.nextSibling();
        }
        return QVariant(stct);
    }
    else if( typeName == "base64" )
    {
        QVariant returnVariant;
        QByteArray dest;
        QByteArray src = typeData.text().toLatin1();
        return QVariant(QByteArray::fromBase64(src));
    }

    errors << QString( "Cannot handle type %1").arg(typeName);
    return QVariant();
}

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

    const QDomElement contents = methodElement.firstChildElement();
    if( contents.tagName().toLower() == "params")
    {
        QDomNode param = contents.firstChildElement("param");
        while (!param.isNull())
        {
            QStringList errors;
            const QVariant value = XMLRPC::demarshall(param.firstChildElement(), errors);
            if (!errors.isEmpty())
                break;
            m_values << value;
            param = param.nextSiblingElement("param");
        }
    }
    else if( contents.tagName().toLower() == "fault")
    {
        QStringList errors;
        const QDomElement errElement = contents.firstChildElement();
        const QVariant error = XMLRPC::demarshall(errElement, errors);

        qWarning() << QString("XMLRPC Fault %1: %2")
                        .arg(error.toMap()["faultCode"].toString() )
                        .arg(error.toMap()["faultString"].toString() );
    }
}

void QXmppRpcResponseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_rpc);

    writer->writeStartElement("methodResponse");
    if (!m_values.isEmpty())
    {
        writer->writeStartElement("params");
        foreach (const QVariant &arg, m_values)
        {
            writer->writeStartElement("param");
            XMLRPC::marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

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

    const QString methodName = methodElement.firstChildElement("methodName").text();
    if (methodName.count('.') == 1)
    {
        m_interface = methodName.split('.').first();
        m_method = methodName.split('.').last();
    }

    const QDomElement methodParams = methodElement.firstChildElement("params");
    m_arguments.clear();
    if( !methodParams.isNull() )
    {
        QDomNode param = methodParams.firstChildElement("param");
        while (!param.isNull())
        {
            QStringList errors;
            QVariant arg = XMLRPC::demarshall(param.firstChildElement(), errors);
            if (!errors.isEmpty())
                break;
            m_arguments << arg;
            param = param.nextSiblingElement("param");
        }
    }
}

void QXmppRpcInvokeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_rpc);

    writer->writeStartElement("methodCall");
    writer->writeTextElement("methodName", m_interface + "." + m_method);
    if (!m_arguments.isEmpty())
    {
        writer->writeStartElement("params");
        foreach(const QVariant &arg, m_arguments)
        {
            writer->writeStartElement("param");
            XMLRPC::marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

    writer->writeEndElement();
}


