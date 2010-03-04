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

    long blockSize() const;
    void setBlockSize( long block_size );

    QString sid() const;
    void setSid( const QString &sid );

private:
    long m_block_size;
    QString m_sid;
};

class QXmppIbbCloseIq: public QXmppIq
{
public:
    QXmppIbbCloseIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isIbbCloseIq( QDomElement &element );

    QString sid() const;
    void setSid( const QString &sid );

private:
    QString m_sid;
};

class QXmppIbbDataIq : public QXmppIq
{
public:
    QXmppIbbDataIq();

    quint16 sequence() const;
    void setSequence( quint16 seq );
    QString sid() const;
    void setSid( const QString &sid );
    QByteArray payload() const;
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
