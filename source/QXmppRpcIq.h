#ifndef QXMPPRPCIQ_H
#define QXMPPRPCIQ_H

#include "QXmppIq.h"
#include <QVariant>

class QXmlStreamWriter;
class QDomElement;

class QXmppRpcResponseIq : public QXmppIq
{
public:
    QXmppRpcResponseIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isRpcResponseIq( QDomElement &element );

    QVariant getPayload() const;
    void setPayload( const QVariant &payload );

private:
    QVariant m_payload;

};

class QXmppRpcInvokeIq : public QXmppIq
{
public:
    QXmppRpcInvokeIq();

    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isRpcInvokeIq( QDomElement &element );

    QVariantList getPayload() const;
    void setPayload( const QVariantList &payload );

    QString getMethod() const;
    void setMethod( const QString &method );

    QString getInterface() const;
    void setInterface( const QString &interface );


private:
    QVariantList m_payload;
    QString m_method;
    QString m_interface;

};

class QXmppRpcErrorIq : public QXmppIq
{
public:
    QXmppRpcErrorIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isRpcErrorIq( QDomElement &element );

    void setQuery(const QXmppRpcInvokeIq &query );
    QXmppRpcInvokeIq getQuery() const;

private:
    QXmppRpcInvokeIq m_query;
};

#endif // QXMPPRPCIQ_H
