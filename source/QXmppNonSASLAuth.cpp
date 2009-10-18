#include "QXmppNonSASLAuth.h"
#include "QXmppUtils.h"
#include <QCryptographicHash>

QXmppNonSASLAuthTypesRequestIq::QXmppNonSASLAuthTypesRequestIq() : QXmppIq(QXmppIq::Get)
{

}

void QXmppNonSASLAuthTypesRequestIq::setUsername( const QString &username )
{
    m_username = username;
}

QByteArray QXmppNonSASLAuthTypesRequestIq::toXmlElementFromChild() const
{
    QByteArray resultingXml;
    resultingXml += "<query xmlns=\"jabber:iq:auth\">";
    resultingXml += "<username>" + escapeString(m_username).toUtf8() + "</username>";
    resultingXml += "</query>";
    return resultingXml;
}

QXmppNonSASLAuthIq::QXmppNonSASLAuthIq() : QXmppIq(QXmppIq::Set), m_useplaintext(false)
{

}

QByteArray QXmppNonSASLAuthIq::toXmlElementFromChild() const
{
    QByteArray resultingXml;
    resultingXml += "<query xmlns=\"jabber:iq:auth\">";
    resultingXml += "<username>" + escapeString(m_username).toUtf8() + "</username>";
    if ( m_useplaintext )
        resultingXml += "<password>" + escapeString(m_password).toUtf8() + "</password>";
    else
    {//SHA1(concat(sid, password)).
        QByteArray textSid = m_sid.toUtf8();
        QByteArray encodedPassword = m_password.toUtf8();
        QByteArray digest = QCryptographicHash::hash(textSid + encodedPassword, QCryptographicHash::Sha1 ).toHex();
        resultingXml += "<digest>" + digest + "</digest>";
    }
    resultingXml += "<resource>" + escapeString(m_resource).toUtf8() + "</resource>";
    resultingXml += "</query>";
    return resultingXml;
}

void QXmppNonSASLAuthIq::setUsername( const QString &username )
{
    m_username = username;
}

void QXmppNonSASLAuthIq::setPassword( const QString &password )
{
    m_password = password;
}

void QXmppNonSASLAuthIq::setResource( const QString &resource )
{
    m_resource = resource;
}

void QXmppNonSASLAuthIq::setStreamId( const QString &sid )
{
    m_sid = sid;
}

void QXmppNonSASLAuthIq::setUsePlainText( bool use )
{
    m_useplaintext = use;
}
