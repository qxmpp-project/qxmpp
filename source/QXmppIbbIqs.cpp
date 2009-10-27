#include "QXmppIbbIqs.h"
#include "QXmppConstants.h"

#include <QDomElement>
#include <QXmlStreamWriter>

QXmppIbbOpenIq::QXmppIbbOpenIq() : QXmppIq(QXmppIq::Set), m_block_size(1024)
{

}

void QXmppIbbOpenIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("open");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeAttribute( "block-size",QString::number(m_block_size) );
    writer->writeEndElement();
}

void QXmppIbbOpenIq::parse( QDomElement &element )
{
    QDomElement openElement = element.firstChildElement("open");
    setId( element.attribute("id"));
    setTo( element.attribute("to"));
    setFrom( element.attribute("from"));
    setTypeFromStr( element.attribute("type"));
    m_sid = openElement.attribute( "sid" );
    m_block_size = openElement.attribute( "block-size" ).toLong();
}

bool QXmppIbbOpenIq::isIbbOpenIq( QDomElement &element )
{
    QDomElement openElement = element.firstChildElement("open");
    return openElement.namespaceURI() == ns_ibb;
}

long QXmppIbbOpenIq::getBlockSize() const
{
    return m_block_size;
}

void QXmppIbbOpenIq::setBlockSize( long block_size )
{
    m_block_size = block_size;
}

QString QXmppIbbOpenIq::getSid() const
{
   return  m_sid;
}

void QXmppIbbOpenIq::setSid( const QString &sid )
{
    m_sid = sid;
}

QXmppIbbAckIq::QXmppIbbAckIq() : QXmppIq(QXmppIq::Result)
{

}

void QXmppIbbAckIq::parse( QDomElement &element )
{
    setId( element.attribute("id"));
    setTo( element.attribute("to"));
    setFrom( element.attribute("from"));
    setTypeFromStr( element.attribute("type"));
}
bool QXmppIbbAckIq::isIbbAckIq( QDomElement &element )
{
    return element.attribute("type") == "result";
}

QXmppIbbErrorIq::QXmppIbbErrorIq()  : QXmppIq(QXmppIq::Error), m_type(Unknown)
{

}

void QXmppIbbErrorIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("error");
    switch( m_type ) {
        case Unknown:
            break;
        case Cancel:
            writer->writeAttribute("type", "cancel");
            writer->writeStartElement("not-acceptable");
            break;
        case NoSupport:
            writer->writeAttribute("type", "cancel");
            writer->writeStartElement("service-unavailable");
            break;
        case Modify:
            writer->writeAttribute("type", "modify");
            writer->writeStartElement("resource-constrainte");
            break;
        case NotFound:
            writer->writeAttribute("type", "cancel");
            writer->writeStartElement("item-not-found");
            break;
    }

    writer->writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
    writer->writeEndElement();
    writer->writeEndElement();
}

void QXmppIbbErrorIq::parse( QDomElement &element )
{
    QDomElement errorElement = element.firstChildElement("error");
    setId( element.attribute("id"));
    setTo( element.attribute("to"));
    setFrom( element.attribute("from"));
    setTypeFromStr( element.attribute("type"));
    if ( errorElement.attribute( "type" ) == "cancel" )
    {
       if( !errorElement.firstChildElement("service-unavailable").isNull() )
            m_type = NoSupport;
       else if( !errorElement.firstChildElement("not-acceptable").isNull() )
            m_type = Cancel;
       else if( !errorElement.firstChildElement("item-not-found").isNull() )
            m_type = NotFound;
       else
           m_type = Unknown;
    }
    else if ( errorElement.attribute( "type" ) == "modify" )
    {
        if( !errorElement.firstChildElement("resource-constraint").isNull() )
            m_type = Modify;
        else
            m_type = Unknown;
    }
    else
        m_type = Unknown;
    m_errorString = errorElement.text();

}

bool QXmppIbbErrorIq::isIbbErrorIq( QDomElement &element )
{
    return element.attribute("type") == "error";
}

QXmppIbbErrorIq::Type QXmppIbbErrorIq::getErrorType() const
{
    return m_type;
}

void QXmppIbbErrorIq::setErrorType( Type err )
{
    m_type = err;
}

QString QXmppIbbErrorIq::getErrorString() const
{
    return m_errorString;
}

void QXmppIbbErrorIq::setErrorString( const QString &err )
{
    m_errorString = err;
}

QXmppIbbCloseIq::QXmppIbbCloseIq() : QXmppIq(QXmppIq::Set)
{

}

void QXmppIbbCloseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("close");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeEndElement();
}

void QXmppIbbCloseIq::parse( QDomElement &element )
{
    QDomElement openElement = element.firstChildElement("close");
    setId( element.attribute("id"));
    setTo( element.attribute("to"));
    setFrom( element.attribute("from"));
    setTypeFromStr( element.attribute("type"));
    m_sid = openElement.attribute( "sid" );
}

bool QXmppIbbCloseIq::isIbbCloseIq( QDomElement &element )
{
    QDomElement openElement = element.firstChildElement("close");
    return openElement.namespaceURI() == ns_ibb;
}

QString QXmppIbbCloseIq::getSid() const
{
   return  m_sid;
}

void QXmppIbbCloseIq::setSid( const QString &sid )
{
    m_sid = sid;
}
