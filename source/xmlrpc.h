#ifndef PACKET_H
#define PACKET_H

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
*    RequestMessage msg( "data.query", args );
*    ResponseMessage resp( SomeHttpDispatchObject( msg.xml() ) );
*    if( resp.isValid() )
*    {
*        int rows = resp.values().first().toMap()["widgets"].toInt();
*    }
*    else
*        qWarning("Error: %s", resp.error().latin1() );
* @endcode
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
/**
* Creates an XMLRPC message that will call a method with a series of
* QVariants that will be converted to XMLRPC types.
@author Ian Reinhart Geiser <geiseri@kde.org>
*/
class RequestMessage
{
public:
    /**
    * Creates a method packet that will call method with a list of args.
    */
    RequestMessage(const QByteArray &method = QByteArray(), const QVariantList &args = QVariantList());

    /**
    * Parse an xml packet.
    */
    bool parse(const QDomElement &element);

    /**
    * Return the xml representation of the packet.
    */
    void writeXml( QXmlStreamWriter *writer ) const;

    QByteArray method() const;
    QVariantList args() const;

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
     * Create a new response message with data.
     */
    ResponseMessage(const QVariantList &values = QVariantList());

    /**
    * Parse an xml packet.
    */
    bool parse(const QDomElement &element);

    /**
    * Return the xml representation of the packet.
    */
    void writeXml( QXmlStreamWriter *writer ) const;

    QVariantList values() const;

private:
    QList<QVariant> m_values;
};

}
#endif
