/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppNonSASLAuth.h"
#include "QXmppUtils.h"
#include <QCryptographicHash>
#include <QXmlStreamWriter>

QXmppNonSASLAuthTypesRequestIq::QXmppNonSASLAuthTypesRequestIq() : QXmppIq(QXmppIq::Get)
{

}

void QXmppNonSASLAuthTypesRequestIq::setUsername( const QString &username )
{
    m_username = username;
}

void QXmppNonSASLAuthTypesRequestIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute( "xmlns","jabber:iq:auth");
    writer->writeTextElement("username", m_username );
    writer->writeEndElement();
}

QXmppNonSASLAuthIq::QXmppNonSASLAuthIq() : QXmppIq(QXmppIq::Set), m_useplaintext(false)
{

}

void QXmppNonSASLAuthIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute( "xmlns","jabber:iq:auth");
    writer->writeTextElement("username", m_username );
    if ( m_useplaintext )
        writer->writeTextElement("password", m_password );
    else
    {//SHA1(concat(sid, password)).
        QByteArray textSid = m_sid.toUtf8();
        QByteArray encodedPassword = m_password.toUtf8();
        QByteArray digest = QCryptographicHash::hash(textSid + encodedPassword, QCryptographicHash::Sha1 ).toHex();
        writer->writeTextElement("digest", digest );
    }
    writer->writeTextElement("resource", m_resource );
    writer->writeEndElement();
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
