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

    QVariantList values() const;
    void setValues(const QVariantList &values);

    static bool isRpcResponseIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QVariantList m_values;
};

class QXmppRpcInvokeIq : public QXmppIq
{
public:
    QXmppRpcInvokeIq();

    QString interface() const;
    void setInterface( const QString &interface );

    QString method() const;
    void setMethod( const QString &method );

    QVariantList arguments() const;
    void setArguments(const QVariantList &arguments);

    static bool isRpcInvokeIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QVariantList m_arguments;
    QString m_method;
    QString m_interface;

    friend class QXmppRpcErrorIq;
};

class QXmppRpcErrorIq : public QXmppIq
{
public:
    QXmppRpcErrorIq();

    QXmppRpcInvokeIq query() const;
    void setQuery(const QXmppRpcInvokeIq &query);

    static bool isRpcErrorIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppRpcInvokeIq m_query;
};

#endif // QXMPPRPCIQ_H
