#ifndef QXMPPIBBIQS_H
#define QXMPPIBBIQS_H

#include "QXmppIq.h"

class QDomElement;
class QXmlStreamWriter;

class QXmppIbbOpenIq: public QXmppIq
{
public:
    QXmppIbbOpenIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isIbbOpenIq( QDomElement &element );

    long getBlockSize() const;
    void setBlockSize( long block_size );

    QString getSid() const;
    void setSid( const QString &sid );

private:
    long m_block_size;
    QString m_sid;
};

class QXmppIbbAckIq: public QXmppIq
{
public:
    QXmppIbbAckIq();
    void parse( QDomElement &element );
    static bool isIbbAckIq( QDomElement &element );
};

class QXmppIbbErrorIq: public QXmppIq
{
public:
    enum Type { Unknown, Cancel, NoSupport, Modify, NotFound };
    QXmppIbbErrorIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isIbbErrorIq( QDomElement &element );

    Type getErrorType() const;
    void setErrorType( Type err );

    QString getErrorString() const;
    void setErrorString( const QString &err );

private:
    Type m_type;
    QString m_errorString;
};

class QXmppIbbCloseIq: public QXmppIq
{
public:
    QXmppIbbCloseIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isIbbCloseIq( QDomElement &element );

    QString getSid() const;
    void setSid( const QString &sid );

private:
    QString m_sid;
};

class QXmppIbbDataIq : public QXmppIq
{
public:
    QXmppIbbDataIq();

    quint16 getSequence() const;
    void setSequence( quint16 seq );
    QString getSid() const;
    void setSid( const QString &sid );
    QByteArray getPayload() const;
    void setPayload( const QByteArray &data );

    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isIbbDataIq( QDomElement &element );

private:
    quint16 m_seq;
    QString m_sid;
    QByteArray m_payload;
};

#endif // QXMPPIBBIQS_H
