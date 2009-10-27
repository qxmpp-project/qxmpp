#ifndef QXMPPDATAIQ_H
#define QXMPPDATAIQ_H

#include "QXmppIq.h"

class QXmlStreamWriter;
class QDomElement;

class QXmppDataIq : public QXmppIq
{
public:
    QXmppDataIq();

    quint16 getSequence() const;
    void setSequence( quint16 seq );
    QString getSid() const;
    void setSid( const QString &sid );
    QByteArray getPayload() const;
    void setPayload( const QByteArray &data );

    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isDataIq( QDomElement &element );
private:
    quint16 m_seq;
    QString m_sid;
    QByteArray m_payload;
};

#endif // QXMPPDATAIQ_H
