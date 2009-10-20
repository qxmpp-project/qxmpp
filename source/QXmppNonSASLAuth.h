#ifndef QXmppNonSASLAuth_H
#define QXmppNonSASLAuth_H

#include "QXmppIq.h"

class QXmppNonSASLAuthTypesRequestIq : public QXmppIq
{
public:
    QXmppNonSASLAuthTypesRequestIq();
    void setUsername( const QString &username );
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
private:
    QString m_username;
};

class QXmppNonSASLAuthIq : public QXmppIq
{
public:
    QXmppNonSASLAuthIq();
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void setUsername( const QString &username );
    void setPassword( const QString &password );
    void setResource( const QString &resource );
    void setStreamId( const QString &sid );
    void setUsePlainText( bool useplaintext );

private:
    QString m_username;
    QString m_password;
    QString m_resource;
    QString m_sid;
    bool m_useplaintext;
};

#endif // QXmppNonSASLAuth_H
