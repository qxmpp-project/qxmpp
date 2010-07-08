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

    QVariant getPayload() const;
    void setPayload( const QVariant &payload );

    static bool isRpcResponseIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QVariant m_payload;
};

class QXmppRpcInvokeIq : public QXmppIq
{
public:
    QXmppRpcInvokeIq();

    QVariantList getPayload() const;
    void setPayload( const QVariantList &payload );

    QString getMethod() const;
    void setMethod( const QString &method );

    QString getInterface() const;
    void setInterface( const QString &interface );

    static bool isRpcInvokeIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QVariantList m_payload;
    QString m_method;
    QString m_interface;

    friend class QXmppRpcErrorIq;
};

class QXmppRpcErrorIq : public QXmppIq
{
public:
    QXmppRpcErrorIq();

    void setQuery(const QXmppRpcInvokeIq &query);
    QXmppRpcInvokeIq getQuery() const;

    static bool isRpcErrorIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppRpcInvokeIq m_query;
};

#endif // QXMPPRPCIQ_H
