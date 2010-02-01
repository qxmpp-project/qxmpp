#include "xmlrpc.h"
#include <QMap>
#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <QTextStream>

XMLRPC::RequestMessage::RequestMessage( const QByteArray &method, const QList<QVariant> &args )
: MessageBase()
{
	m_method = method;
	m_args = args;
}

XMLRPC::RequestMessage::RequestMessage( const QByteArray &method, const QVariant &arg )
: MessageBase()
{
	m_method = method;
	m_args.append(arg);
}

void XMLRPC::RequestMessage::writeXml( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("methodCall");
    writer->writeTextElement("methodName", m_method );
    if( !m_args.isEmpty() )
    {
        writer->writeStartElement("params");
        foreach( QVariant arg, m_args)
        {
            writer->writeStartElement("param");
            marshall( writer, arg );
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();

}

QByteArray XMLRPC::RequestMessage::xml() const
{
	if( m_method.isEmpty() )
		return QByteArray();

        QByteArray returnXML;
        QXmlStreamWriter writer( &returnXML );
        writer.writeStartDocument();
        writeXml(&writer );
        writer.writeEndDocument();
	return returnXML;
}

void XMLRPC::MessageBase::marshall( QXmlStreamWriter *writer, const QVariant &value ) const
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
			foreach( QVariant item, value.toList() )
                                marshall( writer, item );
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
			else
			{

			}
			break;
		}
	}
        writer->writeEndElement();
    }
XMLRPC::ResponseMessage::ResponseMessage( const QDomElement &element )
{
    const QDomElement contents = element.firstChild().toElement();
    if( contents.tagName().toLower() == "params")
    {
            QDomNode param = contents.firstChild();
            while( !param.isNull() && isValid() )
            {
                    m_values.append( demarshall( param.firstChild().toElement() ) );
                    param = param.nextSibling();
            }
    }
    else if( contents.tagName().toLower() == "fault")
    {
            const QDomElement errElement = contents.firstChild().toElement();
            const QVariant error = demarshall( errElement );

            setError( QString("XMLRPC Fault %1: %2")
                            .arg(error.toMap()["faultCode"].toString() )
                            .arg(error.toMap()["faultString"].toString() ) );
    }
    else
    {
            setError("Bad XML response");
    }
}

XMLRPC::ResponseMessage::ResponseMessage( const QByteArray &xml )
: MessageBase()
{
	QDomDocument message;
	QString xmlErrMsg;
	int xmlErrLine;
	int xmlErrCol;
	if( message.setContent(xml, &xmlErrMsg, &xmlErrLine, &xmlErrCol) )
	{
		const QDomElement contents = message.documentElement().firstChild().toElement();
		if( contents.tagName().toLower() == "params")
		{
			QDomNode param = contents.firstChild();
			while( !param.isNull() && isValid() )
			{
				m_values.append( demarshall( param.firstChild().toElement() ) );
				param = param.nextSibling();
			}
		}
		else if( message.documentElement().firstChild().toElement().tagName().toLower() == "fault")
		{
			const QDomElement errElement = contents.firstChild().toElement();
			const QVariant error = demarshall( errElement );

			setError( QString("XMLRPC Fault %1: %2")
					.arg(error.toMap()["faultCode"].toString() )
					.arg(error.toMap()["faultString"].toString() ) );
		}
		else
		{
			setError("Bad XML response");
		}
	}
	else
	{
	setError(QString( "XML Error: %1 at row %2 and col %3")
	         .arg(xmlErrMsg).arg(xmlErrLine).arg(xmlErrCol));
	}
}

int XMLRPC::ResponseMessage::count() const
{
	return m_values.count();
}

QVariant XMLRPC::ResponseMessage::value( int index) const
{
	return m_values[index];
}

bool XMLRPC::MessageBase::isValid() const
{
	return m_valid;
}

QString XMLRPC::MessageBase::error() const
{
	return m_message;
}

QVariant XMLRPC::MessageBase::demarshall( const QDomElement &elem ) const
{
	if ( elem.tagName().toLower() != "value" )
	{
		m_valid = false;
		m_message = "bad param value";
		return QVariant();
	}

	if ( !elem.firstChild().isElement() )
	{
		return QVariant( elem.text() );
	}

	const QDomElement typeData = elem.firstChild().toElement();
	const QString typeName = typeData.tagName().toLower();

	if ( typeName == "string" )
                return QVariant( typeData.text() );
	else if (typeName == "int" || typeName == "i4" )
	{
		bool ok = false;
		QVariant val( typeData.text().toInt( &ok ) );
		if( ok )
			return val;
		m_message = "I was looking for an integer but data was courupt";
	}
	else if( typeName == "double" )
	{
		bool ok = false;
		QVariant val( typeData.text().toDouble( &ok ) );
		if( ok )
			return val;
		m_message = "I was looking for an double but data was courupt";
	}
	else if( typeName == "boolean" )
		return QVariant( ( typeData.text().toLower() == "true" || typeData.text() == "1")?true:false );
	else if( typeName == "datetime" || typeName == "dateTime.iso8601" )
		return QVariant( QDateTime::fromString( typeData.text(), Qt::ISODate ) );
	else if( typeName == "array" )
	{
		QList<QVariant> arr;
		QDomNode valueNode = typeData.firstChild().firstChild();
		while( !valueNode.isNull() && m_valid )
		{
			arr.append( demarshall( valueNode.toElement() ) );
			valueNode = valueNode.nextSibling();
		}
		return QVariant( arr );
	}
	else if( typeName == "struct" )
	{
		QMap<QString,QVariant> stct;
		QDomNode valueNode = typeData.firstChild();
		while( !valueNode.isNull() && m_valid )
		{
			const QDomElement memberNode = valueNode.toElement().elementsByTagName("name").item(0).toElement();
			const QDomElement dataNode = valueNode.toElement().elementsByTagName("value").item(0).toElement();
			stct[ memberNode.text() ] = demarshall( dataNode );
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
		if( returnVariant.isValid() )
			return returnVariant;
		else
			return QVariant( dest );
	}
	setError(QString( "Cannot handle type %1").arg(typeName));
	return QVariant();
}

void XMLRPC::MessageBase::setError( const QString & message ) const
{
	m_valid = false;
	m_message = message;
}

XMLRPC::MessageBase::MessageBase( ) : m_valid(true)
{
}

XMLRPC::MessageBase::~MessageBase( )
{
}


QList< QVariant > XMLRPC::ResponseMessage::values() const
{
	return m_values;
}

QList< QVariant > XMLRPC::RequestMessage::args() const
{
	return m_args;
}

QByteArray XMLRPC::RequestMessage::method() const
{
	return m_method;
}

XMLRPC::RequestMessage::RequestMessage( const QDomElement  &element )
{
    m_args.clear();
    m_method.clear();

    const QDomElement methodName = element.firstChildElement("methodName");
    if( !methodName.isNull() )
    {
            m_method = methodName.text().toLatin1();
    }
    else
    {
            setError("Missing methodName property.");
            return;
    }

    const QDomElement methodParams = element.firstChildElement("params");
    if( !methodParams.isNull() )
    {
            QDomNode param = methodParams.firstChild();
            while( !param.isNull() && isValid() )
            {
                    m_args.append( demarshall( param.firstChild().toElement() ) );
                    param = param.nextSibling();
            }
    }
}

XMLRPC::RequestMessage::RequestMessage( const QByteArray & xml )
{
	QDomDocument message;
	QString xmlErrMsg;
	int xmlErrLine;
	int xmlErrCol;
	m_args.clear();
	m_method.clear();

	if( message.setContent(xml, &xmlErrMsg, &xmlErrLine, &xmlErrCol) )
	{
		const QDomElement methodCall = message.firstChildElement("methodCall");
		if( !methodCall.isNull() )
		{
			const QDomElement methodName = methodCall.firstChildElement("methodName");
			if( !methodName.isNull() )
			{
				m_method = methodName.text().toLatin1();
			}
			else
			{
				setError("Missing methodName property.");
				return;
			}

			const QDomElement methodParams = methodCall.firstChildElement("params");
			if( !methodParams.isNull() )
			{
				QDomNode param = methodParams.firstChild();
				while( !param.isNull() && isValid() )
				{
					m_args.append( demarshall( param.firstChild().toElement() ) );
					param = param.nextSibling();
				}
			}
		}
		else
			setError("Not a valid methodCall message.");
	}
	else
	{
		setError(QString( "XML Error: %1 at row %2 and col %3")
	         .arg(xmlErrMsg).arg(xmlErrLine).arg(xmlErrCol));
	}
}

void XMLRPC::ResponseMessage::writeXml( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("methodResponse");

    if( !m_values.isEmpty() )
    {
        writer->writeStartElement("params");
        foreach( QVariant arg, m_values)
        {
            writer->writeStartElement("params");
            marshall( writer, arg );
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

QByteArray XMLRPC::ResponseMessage::xml( ) const
{
        QByteArray returnXML ;
        QXmlStreamWriter writer(&returnXML);
        writer.writeStartDocument();
        writeXml( &writer );
        writer.writeEndElement();
        writer.writeEndDocument();
	return returnXML;
}

void XMLRPC::ResponseMessage::setValues( const QList< QVariant > vals )
{
	m_values = vals;
}

XMLRPC::ResponseMessage::ResponseMessage( const QList< QVariant > & theValue  )
: MessageBase(), m_values(theValue)
{
}

XMLRPC::FaultMessage::FaultMessage( int code, const QString & message ) :
ResponseMessage(QList<QVariant>() )
{
	QList<QVariant> args;
	QMap<QString,QVariant> fault;
	fault["faultCode"] = code;
	fault["faultString"] = message;
	args << fault;
	setValues(args);
}

void XMLRPC::FaultMessage::writeXml( QXmlStreamWriter *writer ) const
{
    writer->writeStartElement("fault");
    marshall( writer, values().first() );
    writer->writeEndElement();
}

QByteArray XMLRPC::FaultMessage::xml( ) const
{
    QByteArray returnXML;
    QXmlStreamWriter writer( &returnXML );
    writer.writeStartDocument();
    writeXml(&writer);
    writer.writeEndDocument();
    return returnXML;
}

XMLRPC::ResponseMessage::ResponseMessage( const QVariant & theValue )
: MessageBase()
{
	m_values << theValue;
}
