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

long QXmppIbbOpenIq::blockSize() const
{
    return m_block_size;
}

void QXmppIbbOpenIq::setBlockSize( long block_size )
{
    m_block_size = block_size;
}

QString QXmppIbbOpenIq::sid() const
{
   return  m_sid;
}

void QXmppIbbOpenIq::setSid( const QString &sid )
{
    m_sid = sid;
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

QString QXmppIbbCloseIq::sid() const
{
   return  m_sid;
}

void QXmppIbbCloseIq::setSid( const QString &sid )
{
    m_sid = sid;
}

QXmppIbbDataIq::QXmppIbbDataIq() : QXmppIq( QXmppIq::Set ), m_seq(0)
{
}

quint16 QXmppIbbDataIq::sequence() const
{
    return m_seq;
}

void QXmppIbbDataIq::setSequence( quint16 seq )
{
    m_seq = seq;
}

QString QXmppIbbDataIq::sid() const
{
    return m_sid;
}

void QXmppIbbDataIq::setSid( const QString &sid )
{
    m_sid = sid;
}

QByteArray QXmppIbbDataIq::payload() const
{
    return m_payload;
}

void QXmppIbbDataIq::setPayload( const QByteArray &data )
{
    m_payload = data;
}


void QXmppIbbDataIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("data");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeAttribute( "seq",QString::number(m_seq) );
    writer->writeCharacters( m_payload.toBase64() );
    writer->writeEndElement();
}

void QXmppIbbDataIq::parse( QDomElement &element )
{
    QDomElement dataElement = element.firstChildElement("data");
    setId( element.attribute("id"));
    setTo( element.attribute("to"));
    setFrom( element.attribute("from"));
    setTypeFromStr( element.attribute("type"));

    m_sid = dataElement.attribute( "sid" );
    m_seq = dataElement.attribute( "seq" ).toLong();
    m_payload = QByteArray::fromBase64( dataElement.text().toLatin1() );
}

bool QXmppIbbDataIq::isIbbDataIq( QDomElement &element )
{
    QDomElement dataElement = element.firstChildElement("data");
    return dataElement.namespaceURI() == ns_ibb;
}
