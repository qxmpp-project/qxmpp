#include "xmlrpc.h"
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>

void XMLRPC::marshall( QXmlStreamWriter *writer, const QVariant &value)
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
            writer->writeTextElement("boolean", (value.toBool()?"true":"false") );
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
            if( value.canConvert(QVariant::String) )
            {
                writer->writeTextElement( "string", value.toString() );
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
        return QVariant( ( typeData.text().toLower() == "true" || typeData.text() == "1")?true:false );
    else if( typeName == "datetime" || typeName == "dateTime.iso8601" )
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
        dest = QByteArray::fromBase64( src );
        QDataStream ds(&dest, QIODevice::ReadOnly);
        ds.setVersion(QDataStream::Qt_4_0);
        ds >> returnVariant;
        if (returnVariant.isValid())
            return returnVariant;
        else
            return QVariant( dest );
    }

    errors << QString( "Cannot handle type %1").arg(typeName);
    return QVariant();
}

bool XMLRPC::RequestMessage::parse(const QDomElement &element)
{
    QStringList errors;

    m_args.clear();
    m_method.clear();

    const QDomElement methodName = element.firstChildElement("methodName");
    if( !methodName.isNull() )
    {
        m_method = methodName.text().toLatin1();
    }
    else
    {
        errors << "Missing methodName property";
        return false;
    }

    const QDomElement methodParams = element.firstChildElement("params");
    if( !methodParams.isNull() )
    {
        QDomNode param = methodParams.firstChildElement("param");
        while (!param.isNull())
        {
            QVariant arg = demarshall(param.firstChild().toElement(), errors);
            if (!errors.isEmpty())
                return false;
            m_args << arg;
            param = param.nextSiblingElement("param");
        }
    }
    return true;
}

QVariantList XMLRPC::RequestMessage::arguments() const
{
    return m_args;
}

void XMLRPC::RequestMessage::setArguments(const QVariantList &args)
{
    m_args = args;
}

QByteArray XMLRPC::RequestMessage::method() const
{
    return m_method;
}

void XMLRPC::RequestMessage::setMethod(const QByteArray &method)
{
    m_method = method;
}

void XMLRPC::RequestMessage::writeXml( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("methodCall");
    writer->writeTextElement("methodName", m_method );
    if( !m_args.isEmpty() )
    {
        writer->writeStartElement("params");
        foreach(const QVariant &arg, m_args)
        {
            writer->writeStartElement("param");
            marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

bool XMLRPC::ResponseMessage::parse(const QDomElement &element)
{
    QStringList errors;
    const QDomElement contents = element.firstChild().toElement();
    if( contents.tagName().toLower() == "params")
    {
        QDomNode param = contents.firstChildElement("param");
        while (!param.isNull())
        {
            const QVariant value = demarshall(param.firstChildElement(), errors);
            if (!errors.isEmpty())
                return false;
            m_values << value;
            param = param.nextSiblingElement("param");
        }
        return true;
    }
    else if( contents.tagName().toLower() == "fault")
    {
        const QDomElement errElement = contents.firstChildElement();
        const QVariant error = demarshall(errElement, errors);

        qWarning() << QString("XMLRPC Fault %1: %2")
                        .arg(error.toMap()["faultCode"].toString() )
                        .arg(error.toMap()["faultString"].toString() );
        return true;
    }
    else
    {
        qWarning("Bad XML response");
        return false;
    }
}

QVariantList XMLRPC::ResponseMessage::values() const
{
    return m_values;
}

void XMLRPC::ResponseMessage::setValues(const QVariantList &values)
{
    m_values = values;
}

void XMLRPC::ResponseMessage::writeXml( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("methodResponse");

    if( !m_values.isEmpty() )
    {
        writer->writeStartElement("params");
        foreach (const QVariant &arg, m_values)
        {
            writer->writeStartElement("param");
            marshall(writer, arg);
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

