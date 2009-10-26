#include "QXmppInformationRequestResult.h"
#include "QXmppConstants.h"
#include <QXmlStreamWriter>

QXmppInformationRequestResult::QXmppInformationRequestResult() : QXmppIq(QXmppIq::Result)
{
}

void QXmppInformationRequestResult::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_disco_info );
    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_disco_info );
    writer->writeEndElement();
    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_ibb );
    writer->writeEndElement();
    writer->writeEndElement();
}
