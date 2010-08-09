#ifndef QXMPPXMLRPC_H
#define QXMPPXMLRPC_H

#include <QDomElement>
#include <QVariant>
#include <QXmlStreamWriter>

/**
* Packets are serialized QVariants that map to XMLRPC types.
* @li int -> int
* @li double -> double,float
* @li string -> QString, QCString, char *, QColor, QFont
* @li datetime.iso8601 -> QDateTime, QTime, QDate
* @li array -> QList<QVariant>, QStringList
* @li struct -> QMap<QVariant>
* @li base64 -> QByteArray
*
* An example of how to use the XML Packets is as follows:
* @code
*    QList<QVariant> args;
*    args << m_db << m_username << m_password << dbQuery;
*    RequestMessage msg;
*    msg.setMethod("data.query");
*    msg.setArguments(args);
*
*    ResponseMessage resp;
*    if (resp.parse(someDomElement))
*    {
*        int rows = resp.values().first().toMap()["widgets"].toInt();
*    }
*    else
*        qWarning("Error: %s", resp.error().latin1() );
* @endcode
*
* This example will construct invoke the data.query() method on the XMLRPC
* interface with the args.  It will then check for the response to see if
* it was valid.  If its valid the message contains a struct of values, one of
* which is "widgets" that is an integer.  The struct is converted to a QVariant
* map and we can convert it as such from the QVariant.  We can then get the
* QVariant for the "widgets" value and convert that to an integer.  If there was
* an error, the packet is marked invalid and will have an error message in it.  The
* error() message will return this message.  The struct in the value will be a valid
* error structure so it can be dealt with accordingly.
*/

namespace XMLRPC
{

void marshall( QXmlStreamWriter *writer, const QVariant &value);
QVariant demarshall(const QDomElement &elem, QStringList &errors);

/**
* Creates an XMLRPC message that will call a method with a series of
* QVariants that will be converted to XMLRPC types.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class RequestMessage
{
public:
    /**
    * Parse an xml packet.
    */
    bool parse(const QDomElement &element);

    /**
    * Return the xml representation of the packet.
    */
    void writeXml( QXmlStreamWriter *writer ) const;

    QByteArray method() const;
    void setMethod(const QByteArray &method);

    QVariantList arguments() const;
    void setArguments(const QVariantList &args);

private:
    QByteArray m_method;
    QVariantList m_args;
};

/**
* Decodes an XMLRPC message from a server into a set of QVariants.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class ResponseMessage
{
public:
    /**
    * Parse an xml packet.
    */
    bool parse(const QDomElement &element);

    /**
    * Return the xml representation of the packet.
    */
    void writeXml( QXmlStreamWriter *writer ) const;

    QVariantList values() const;
    void setValues(const QVariantList &values);

private:
    QList<QVariant> m_values;
};

}
#endif
