#include "QXmppReachAddress.h"

#include "QXmppConstants.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomElement>


QXmppReachAddress::QXmppReachAddress()
{
}

bool QXmppReachAddress::isNull() const
{
    return m_addressList.isEmpty();
}


const QList<QXmppAddress> QXmppReachAddress::getAddresses() const
{
    return m_addressList;
}

void QXmppReachAddress::addAddress(const QXmppAddress& addr)
{
    m_addressList.append(addr);
}


/// \cond
void QXmppReachAddress::parse(const QDomElement& element)
{
    QDomElement reachElement = (element.tagName() == "reach") ? element : element.firstChildElement("reach");
    if (reachElement.namespaceURI() == ns_reach) {

        QDomElement addrElement = reachElement.firstChildElement("addr");
        for (; !addrElement.isNull(); addrElement = addrElement.nextSiblingElement("addr")) {
            QXmppAddress addr;
            addr.setAddress(addrElement.attribute("uri"));
            QDomElement descElelement = addrElement.firstChildElement("desc");
            if(!descElelement.isNull())
            {
                addr.setLanguage(descElelement.attribute("lang"));
                addr.setDescription(descElelement.text());
            }
            m_addressList.append(addr);
        }
    }
}

void QXmppReachAddress::toXml(QXmlStreamWriter* writer) const
{
    if (isNull())
        return;
    writer->writeStartElement("reach");
    writer->writeAttribute("xmlns", ns_reach);
    foreach(const QXmppAddress& addr, m_addressList)
    {
        if(!addr.getAddress().isEmpty()) {
            writer->writeStartElement("addr");
            helperToXmlAddAttribute(writer, "uri", addr.getAddress());

            if(!addr.getDescription().isEmpty()) {
                writer->writeStartElement("desc");
                helperToXmlAddAttribute(writer, "xml:lang", addr.getLanguage());
                writer->writeCharacters(addr.getDescription());
                writer->writeEndElement();
            }
            writer->writeEndElement();
        }
    }
    writer->writeEndElement();
}

QXmppElement QXmppReachAddress::toQXmppElement() const
{
    QXmppElement reachElement;
    reachElement.setTagName("reach");
    reachElement.setAttribute("xmlns",ns_reach);

    foreach(const QXmppAddress& addr, m_addressList)
    {
        if(!addr.getAddress().isEmpty()) {
            QXmppElement addrElement;
            addrElement.setTagName("addr");
            if(!addr.getAddress().isEmpty())
                addrElement.setAttribute("uri", addr.getAddress());

            if(!addr.getDescription().isEmpty()) {
                QXmppElement descElement;
                descElement.setTagName("desc");
                descElement.setAttribute("xml:lang", addr.getLanguage());
                descElement.setValue(addr.getDescription());

                addrElement.appendChild(descElement);
            }
            reachElement.appendChild(addrElement);
        }
    }

    return reachElement;
}


QXmppAddress::QXmppAddress()
    : m_address("")
    , m_description("")
    , m_language("")
{

}
QXmppAddress::QXmppAddress(const QString& addr, const QString& desc, const QString& lang)
    : m_address(addr)
    , m_description(desc)
    , m_language(lang)
{

}

void QXmppAddress::setAddress(const QString& addr)
{
    m_address = addr;
}

void QXmppAddress::setDescription(const QString& desc)
{
    m_description = desc;
}

void QXmppAddress::setLanguage(const QString& lang)
{
    m_language = lang;
}

const QString& QXmppAddress::getAddress() const
{
    return m_address;
}

const QString& QXmppAddress::getDescription() const
{
    return m_description;
}

const QString& QXmppAddress::getLanguage() const
{
    return m_language;
}



