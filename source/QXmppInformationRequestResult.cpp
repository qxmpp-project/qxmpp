#include "QXmppInformationRequestResult.h"
#include "QXmppConstants.h"

QXmppInformationRequestResult::QXmppInformationRequestResult() : QXmppIq(QXmppIq::Result)
{
}

QByteArray QXmppInformationRequestResult::toXmlElementFromChild() const
{
    QByteArray resultXml;

    resultXml += "<query xmlns='";
    resultXml += ns_disco_info;
    resultXml += "'>";
    resultXml += "<feature var='";
    resultXml += ns_disco_info;
    resultXml += "'/>";
    resultXml += "<feature var='";
    resultXml += ns_ibb;
    resultXml += "'/>";
    resultXml += "</query>";

    return resultXml;
}
