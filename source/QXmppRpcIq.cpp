#include "QXmppRpcIq.h"
#include "QXmppConstants.h"
#include "xmlrpc.h"

#include <QDomElement>

QXmppRpcErrorIq::QXmppRpcErrorIq() : QXmppIq( QXmppIq::Error )
{

}

void QXmppRpcErrorIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    m_query.toXmlElementFromChild(writer);
}

void QXmppRpcErrorIq::setQuery(const QXmppRpcInvokeIq &query )
{
    m_query = query;
}

QXmppRpcInvokeIq QXmppRpcErrorIq::getQuery() const
{
    return m_query;
}

bool QXmppRpcErrorIq::isRpcErrorIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement errorElement = element.firstChildElement("error");
    QDomElement queryElement = element.firstChildElement("query");
    return (type == "error") &&
            !errorElement.isNull() &&
            queryElement.namespaceURI() == ns_rpc;
}

void QXmppRpcErrorIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));
}

QXmppRpcResponseIq::QXmppRpcResponseIq() : QXmppIq( QXmppIq::Result )
{
}

QVariant QXmppRpcResponseIq::getPayload() const
{
    return m_payload;
}

void QXmppRpcResponseIq::setPayload( const QVariant &payload )
{
    m_payload = payload;
}

bool QXmppRpcResponseIq::isRpcResponseIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement dataElement = element.firstChildElement("query");
    return dataElement.namespaceURI() == ns_rpc &&
           type == "result";
}

void QXmppRpcResponseIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    QDomElement methodElement = queryElement.firstChildElement("methodResponse");

    XMLRPC::ResponseMessage message( methodElement );
    m_payload = message.value();

}

void QXmppRpcResponseIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    XMLRPC::ResponseMessage message( m_payload );
    writer->writeStartElement(ns_rpc, "query");
    message.writeXml(writer);
    writer->writeEndElement();
}

QXmppRpcInvokeIq::QXmppRpcInvokeIq() : QXmppIq( QXmppIq::Set )
{
}

QVariantList QXmppRpcInvokeIq::getPayload() const
{
    return m_payload;
}

void QXmppRpcInvokeIq::setPayload( const QVariantList &payload )
{
    m_payload = payload;
}

QString QXmppRpcInvokeIq::getMethod() const
{
    return m_method;
}
void QXmppRpcInvokeIq::setMethod( const QString &method )
{
    m_method = method;
}

QString QXmppRpcInvokeIq::getInterface() const
{
    return m_interface;
}

void QXmppRpcInvokeIq::setInterface( const QString &interface )
{
    m_interface = interface;
}

bool QXmppRpcInvokeIq::isRpcInvokeIq(const QDomElement &element)
{
    QString type = element.attribute("type");
    QDomElement dataElement = element.firstChildElement("query");
    return dataElement.namespaceURI() == ns_rpc &&
           type == "set";
}

void QXmppRpcInvokeIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    QDomElement methodElement = queryElement.firstChildElement("methodCall");

    XMLRPC::RequestMessage message( methodElement );

    m_interface = message.method().split('.').value(0);
    m_method = message.method().split('.').value(1);
    m_payload = message.args();

}

void QXmppRpcInvokeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    QString methodName = m_interface + "." + m_method;
    XMLRPC::RequestMessage message( methodName.toLatin1() ,m_payload );
    writer->writeStartElement(ns_rpc, "query");
    message.writeXml(writer);
    writer->writeEndElement();
}

