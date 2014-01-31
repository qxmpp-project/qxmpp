#include "QXmppMessageCarbonsIq.h"


#include <QDomElement>
#include <QXmlStreamWriter>

#include "QXmppConstants.h"


QXmppMessageCarbonsIq::QXmppMessageCarbonsIq()
    : QXmppIq(QXmppIq::Set)
{
}

void QXmppMessageCarbonsIq::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("iq");
    writer->writeAttribute("xmlns", ns_client);
    writer->writeAttribute("type", "set");
    writer->writeAttribute("id ", "enableCarbonsBB10");

    writer->writeStartElement("enable");
    writer->writeAttribute("xmlns", ns_message_carbons);
    writer->writeEndElement();

    writer->writeEndElement();

}
