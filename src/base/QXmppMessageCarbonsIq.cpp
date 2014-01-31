#include "QXmppMessageCarbonsIq.h"


#include <QDomElement>
#include <QXmlStreamWriter>

#include "QXmppConstants.h"


QXmppMessageCarbonsIq::QXmppMessageCarbonsIq()
    : QXmppIq(QXmppIq::Set)
{
}

void QXmppMessageCarbonsIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("enable");
    writer->writeAttribute("xmlns", ns_message_carbons);
    writer->writeEndElement();
}
