#ifndef QXMPPIBBIQ_H
#define QXMPPIBBIQ_H

#include "QXmppIq.h"

class QDomElement;
class QXmlStreamWriter;

class QXmppIbbOpenIq: public QXmppIq
{
public:
    QXmppIbbOpenIq();

    long blockSize() const;
    void setBlockSize( long block_size );

    QString sid() const;
    void setSid( const QString &sid );

    static bool isIbbOpenIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    long m_block_size;
    QString m_sid;
};

class QXmppIbbCloseIq: public QXmppIq
{
public:
    QXmppIbbCloseIq();

    QString sid() const;
    void setSid( const QString &sid );

    static bool isIbbCloseIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

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

    static bool isIbbDataIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    quint16 m_seq;
    QString m_sid;
    QByteArray m_payload;
};

#endif // QXMPPIBBIQS_H
