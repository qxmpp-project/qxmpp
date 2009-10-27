#include "QXmppDataIq.h"
#include "QXmppConstants.h"

#include <QXmlStreamWriter>
#include <QDomElement>

QXmppDataIq::QXmppDataIq() : QXmppIq( QXmppIq::Set ), m_seq(0)
{
}

quint16 QXmppDataIq::getSequence() const
{
    return m_seq;
}

void QXmppDataIq::setSequence( quint16 seq )
{
    m_seq = seq;
}

QString QXmppDataIq::getSid() const
{
    return m_sid;
}

void QXmppDataIq::setSid( const QString &sid )
{
    m_sid = sid;
}

QByteArray QXmppDataIq::getPayload() const
{
    return m_payload;
}

void QXmppDataIq::setPayload( const QByteArray &data )
{
    m_payload = data;
}


void QXmppDataIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("data");
    writer->writeAttribute( "xmlns",ns_ibb);
    writer->writeAttribute( "sid",m_sid);
    writer->writeAttribute( "seq",QString::number(m_seq) );
    writer->writeCharacters( m_payload.toBase64() );
    writer->writeEndElement();
}

void QXmppDataIq::parse( QDomElement &element )
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

bool QXmppDataIq::isDataIq( QDomElement &element )
{
    QDomElement dataElement = element.firstChildElement("data");
    return dataElement.namespaceURI() == ns_ibb;
}
