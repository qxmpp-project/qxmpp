// SPDX-FileCopyrightText: 2009 Ian Reinhart Geiser <geiseri@kde.org>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRpcIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDateTime>
#include <QDomElement>
#include <QMap>
#include <QStringList>
#include <QVariant>

void QXmppRpcMarshaller::marshall(QXmlStreamWriter *writer, const QVariant &value)
{
    writer->writeStartElement(QStringLiteral("value"));
    switch (value.type()) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        writer->writeTextElement(QStringLiteral("i4"), value.toString());
        break;
    case QVariant::Double:
        writer->writeTextElement(QStringLiteral("double"), value.toString());
        break;
    case QVariant::Bool:
        writer->writeTextElement(QStringLiteral("boolean"), value.toBool() ? QStringLiteral("1") : QStringLiteral("0"));
        break;
    case QVariant::Date:
        writer->writeTextElement(QStringLiteral("dateTime.iso8601"), value.toDate().toString(Qt::ISODate));
        break;
    case QVariant::DateTime:
        writer->writeTextElement(QStringLiteral("dateTime.iso8601"), value.toDateTime().toString(Qt::ISODate));
        break;
    case QVariant::Time:
        writer->writeTextElement(QStringLiteral("dateTime.iso8601"), value.toTime().toString(Qt::ISODate));
        break;
    case QVariant::StringList:
    case QVariant::List: {
        writer->writeStartElement(QStringLiteral("array"));
        writer->writeStartElement(QStringLiteral("data"));
        for (const auto &item : value.toList())
            marshall(writer, item);
        writer->writeEndElement();
        writer->writeEndElement();
        break;
    }
    case QVariant::Map: {
        writer->writeStartElement(QStringLiteral("struct"));
        const QMap<QString, QVariant> map = value.toMap();
        QMap<QString, QVariant>::ConstIterator index = map.begin();
        while (index != map.end()) {
            writer->writeStartElement(QStringLiteral("member"));
            writer->writeTextElement(QStringLiteral("name"), index.key());
            marshall(writer, *index);
            writer->writeEndElement();
            ++index;
        }
        writer->writeEndElement();
        break;
    }
    case QVariant::ByteArray: {
        writer->writeTextElement(QStringLiteral("base64"), value.toByteArray().toBase64());
        break;
    }
    default: {
        if (value.isNull())
            writer->writeEmptyElement(QStringLiteral("nil"));
        else if (value.canConvert(QVariant::String)) {
            writer->writeTextElement(QStringLiteral("string"), value.toString());
        }
        break;
    }
    }
    writer->writeEndElement();
}

QVariant QXmppRpcMarshaller::demarshall(const QDomElement &elem, QStringList &errors)
{
    if (elem.tagName().toLower() != QStringLiteral("value")) {
        errors << "Bad param value";
        return QVariant();
    }

    if (!elem.firstChild().isElement()) {
        return QVariant(elem.text());
    }

    const QDomElement typeData = elem.firstChild().toElement();
    const QString typeName = typeData.tagName().toLower();

    if (typeName == QStringLiteral("nil")) {
        return QVariant();
    }
    if (typeName == QStringLiteral("string")) {
        return QVariant(typeData.text());
    } else if (typeName == QStringLiteral("int") || typeName == QStringLiteral("i4")) {
        bool ok = false;
        QVariant val(typeData.text().toInt(&ok));
        if (ok)
            return val;
        errors << "I was looking for an integer but data was courupt";
        return QVariant();
    } else if (typeName == QStringLiteral("double")) {
        bool ok = false;
        QVariant val(typeData.text().toDouble(&ok));
        if (ok)
            return val;
        errors << "I was looking for an double but data was corrupt";
    } else if (typeName == QStringLiteral("boolean"))
        return QVariant(typeData.text() == QStringLiteral("1") || typeData.text().toLower() == QStringLiteral("true"));
    else if (typeName == QStringLiteral("datetime") || typeName == QStringLiteral("datetime.iso8601"))
        return QVariant(QDateTime::fromString(typeData.text(), Qt::ISODate));
    else if (typeName == QStringLiteral("array")) {
        QVariantList arr;
        QDomElement valueNode = typeData.firstChildElement(QStringLiteral("data")).firstChildElement();
        while (!valueNode.isNull() && errors.isEmpty()) {
            arr.append(demarshall(valueNode, errors));
            valueNode = valueNode.nextSiblingElement();
        }
        return QVariant(arr);
    } else if (typeName == QStringLiteral("struct")) {
        QMap<QString, QVariant> stct;
        QDomNode valueNode = typeData.firstChild();
        while (!valueNode.isNull() && errors.isEmpty()) {
            const QDomElement memberNode = valueNode.toElement().elementsByTagName(QStringLiteral("name")).item(0).toElement();
            const QDomElement dataNode = valueNode.toElement().elementsByTagName(QStringLiteral("value")).item(0).toElement();
            stct[memberNode.text()] = demarshall(dataNode, errors);
            valueNode = valueNode.nextSibling();
        }
        return QVariant(stct);
    } else if (typeName == QStringLiteral("base64")) {
        QVariant returnVariant;
        QByteArray dest;
        QByteArray src = typeData.text().toLatin1();
        return QVariant(QByteArray::fromBase64(src));
    }

    errors << QStringLiteral("Cannot handle type %1").arg(typeName);
    return QVariant();
}

QXmppRpcErrorIq::QXmppRpcErrorIq() : QXmppIq(QXmppIq::Error)
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

/// \cond
bool QXmppRpcErrorIq::isRpcErrorIq(const QDomElement &element)
{
    QString type = element.attribute(QStringLiteral("type"));
    QDomElement errorElement = element.firstChildElement(QStringLiteral("error"));
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    return (type == QStringLiteral("error")) &&
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
/// \endcond

QXmppRpcResponseIq::QXmppRpcResponseIq()
    : QXmppIq(QXmppIq::Result),
      m_faultCode(0)
{
}

/// Returns the fault code.
///

int QXmppRpcResponseIq::faultCode() const
{
    return m_faultCode;
}

/// Sets the fault code.
///
/// \param faultCode

void QXmppRpcResponseIq::setFaultCode(int faultCode)
{
    m_faultCode = faultCode;
}

/// Returns the fault string.
///

QString QXmppRpcResponseIq::faultString() const
{
    return m_faultString;
}

/// Sets the fault string.
///
/// \param faultString

void QXmppRpcResponseIq::setFaultString(const QString &faultString)
{
    m_faultString = faultString;
}

/// Returns the response values.
///

QVariantList QXmppRpcResponseIq::values() const
{
    return m_values;
}

/// Sets the response values.
///
/// \param values

void QXmppRpcResponseIq::setValues(const QVariantList &values)
{
    m_values = values;
}

/// \cond
bool QXmppRpcResponseIq::isRpcResponseIq(const QDomElement &element)
{
    QString type = element.attribute(QStringLiteral("type"));
    QDomElement dataElement = element.firstChildElement(QStringLiteral("query"));
    return dataElement.namespaceURI() == ns_rpc &&
        type == QStringLiteral("result");
}

void QXmppRpcResponseIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    QDomElement methodElement = queryElement.firstChildElement(QStringLiteral("methodResponse"));

    const QDomElement contents = methodElement.firstChildElement();
    if (contents.tagName().toLower() == QStringLiteral("params")) {
        QDomNode param = contents.firstChildElement(QStringLiteral("param"));
        while (!param.isNull()) {
            QStringList errors;
            const QVariant value = QXmppRpcMarshaller::demarshall(param.firstChildElement(QStringLiteral("value")), errors);
            if (!errors.isEmpty())
                break;
            m_values << value;
            param = param.nextSiblingElement(QStringLiteral("param"));
        }
    } else if (contents.tagName().toLower() == QStringLiteral("fault")) {
        QStringList errors;
        const QDomElement errElement = contents.firstChildElement(QStringLiteral("value"));
        const QVariant error = QXmppRpcMarshaller::demarshall(errElement, errors);
        if (!errors.isEmpty())
            return;
        m_faultCode = error.toMap()[QStringLiteral("faultCode")].toInt();
        m_faultString = error.toMap()[QStringLiteral("faultString")].toString();
    }
}

void QXmppRpcResponseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_rpc);

    writer->writeStartElement(QStringLiteral("methodResponse"));
    if (m_faultCode) {
        writer->writeStartElement(QStringLiteral("fault"));
        QMap<QString, QVariant> fault;
        fault[QStringLiteral("faultCode")] = m_faultCode;
        fault[QStringLiteral("faultString")] = m_faultString;
        QXmppRpcMarshaller::marshall(writer, fault);
        writer->writeEndElement();
    } else if (!m_values.isEmpty()) {
        writer->writeStartElement(QStringLiteral("params"));
        for (const auto &arg : m_values) {
            writer->writeStartElement(QStringLiteral("param"));
            QXmppRpcMarshaller::marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

    writer->writeEndElement();
}
/// \endcond

QXmppRpcInvokeIq::QXmppRpcInvokeIq()
    : QXmppIq(QXmppIq::Set)
{
}

/// Returns the method arguments.
///

QVariantList QXmppRpcInvokeIq::arguments() const
{
    return m_arguments;
}

/// Sets the method arguments.
///
/// \param arguments

void QXmppRpcInvokeIq::setArguments(const QVariantList &arguments)
{
    m_arguments = arguments;
}

/// Returns the method name.
///

QString QXmppRpcInvokeIq::method() const
{
    return m_method;
}

/// Sets the method name.
///
/// \param method

void QXmppRpcInvokeIq::setMethod(const QString &method)
{
    m_method = method;
}

/// \cond
bool QXmppRpcInvokeIq::isRpcInvokeIq(const QDomElement &element)
{
    QString type = element.attribute(QStringLiteral("type"));
    QDomElement dataElement = element.firstChildElement(QStringLiteral("query"));
    return dataElement.namespaceURI() == ns_rpc &&
        type == QStringLiteral("set");
}

void QXmppRpcInvokeIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(QStringLiteral("query"));
    QDomElement methodElement = queryElement.firstChildElement(QStringLiteral("methodCall"));

    m_method = methodElement.firstChildElement(QStringLiteral("methodName")).text();

    const QDomElement methodParams = methodElement.firstChildElement(QStringLiteral("params"));
    m_arguments.clear();
    if (!methodParams.isNull()) {
        QDomNode param = methodParams.firstChildElement(QStringLiteral("param"));
        while (!param.isNull()) {
            QStringList errors;
            QVariant arg = QXmppRpcMarshaller::demarshall(param.firstChildElement(QStringLiteral("value")), errors);
            if (!errors.isEmpty())
                break;
            m_arguments << arg;
            param = param.nextSiblingElement(QStringLiteral("param"));
        }
    }
}

void QXmppRpcInvokeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("query"));
    writer->writeDefaultNamespace(ns_rpc);

    writer->writeStartElement(QStringLiteral("methodCall"));
    writer->writeTextElement(QStringLiteral("methodName"), m_method);
    if (!m_arguments.isEmpty()) {
        writer->writeStartElement(QStringLiteral("params"));
        for (const auto &arg : m_arguments) {
            writer->writeStartElement(QStringLiteral("param"));
            QXmppRpcMarshaller::marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

    writer->writeEndElement();
}
/// \endcond
