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

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_rpc);
    writer->writeEndElement();
    writer->writeStartElement("identity");
    writer->writeAttribute("category", "automation" );
    writer->writeAttribute("type", "rpc" );
    writer->writeEndElement();

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_ping);
    writer->writeEndElement();

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_chat_states);
    writer->writeEndElement();

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_stream_initiation);
    writer->writeEndElement();

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_stream_initiation_file_transfer);
    writer->writeEndElement();

    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_bytestreams);
    writer->writeEndElement();

    // XEP-0092: Software Version
    writer->writeStartElement("feature");
    writer->writeAttribute("var", ns_version);
    writer->writeEndElement();

    writer->writeEndElement();
}
