#include "QXmppEntityTimeIq.h"

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppUtils.h"

QString QXmppEntityTimeIq::tzo() const
{
    return m_tzo;
}

void QXmppEntityTimeIq::setTzo(const QString &tzo)
{
    m_tzo = tzo;
}

QString QXmppEntityTimeIq::utc() const
{
    return m_utc;
}

void QXmppEntityTimeIq::setUtc(const QString &utc)
{
    m_utc = utc;
}

bool QXmppEntityTimeIq::isEntityTimeIq(const QDomElement &element)
{
    QDomElement timeElement = element.firstChildElement("time");
    return timeElement.namespaceURI() == ns_entity_time;
}

void QXmppEntityTimeIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement timeElement = element.firstChildElement("time");
    m_tzo = timeElement.firstChildElement("tzo").text();
    m_utc = timeElement.firstChildElement("utc").text();
}

void QXmppEntityTimeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("time");
    helperToXmlAddAttribute(writer, "xmlns", ns_entity_time);

    if(!m_tzo.isEmpty())
        helperToXmlAddTextElement(writer, "tzo", m_tzo);

    if(!m_utc.isEmpty())
        helperToXmlAddTextElement(writer, "utc", m_utc);

    writer->writeEndElement();
}
