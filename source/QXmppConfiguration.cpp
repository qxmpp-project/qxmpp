/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
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


#include "QXmppConfiguration.h"

QXmppConfiguration::QXmppConfiguration():m_resource("QXmpp"), m_autoAcceptSubscriptions(true),
                m_sendIntialPresence(true), m_sendRosterRequest(true), m_port(5222),
                m_keepAlivePingsInterval(100)
{

}

QXmppConfiguration::~QXmppConfiguration()
{

}

void QXmppConfiguration::setHost(const QString& str)
{
    m_host = str;
}

void QXmppConfiguration::setDomain(const QString& str)
{
    m_domain = str;
}

void QXmppConfiguration::setPort(int port)
{
    m_port = port;
}

void QXmppConfiguration::setUser(const QString& str)
{
    m_user = str;
}

void QXmppConfiguration::setPasswd(const QString& str)
{
    m_passwd = str;
}

void QXmppConfiguration::setStatus(const QString& str)
{
    m_status = str;
}

void QXmppConfiguration::setResource(const QString& str)
{
    m_resource = str;
}

QString QXmppConfiguration::getHost() const
{
    return m_host;
}

QString QXmppConfiguration::getDomain() const
{
    return m_domain;
}

int QXmppConfiguration::getPort() const
{
    return m_port;
}

QString QXmppConfiguration::getUser() const
{
    return m_user;
}
QString QXmppConfiguration::getPasswd() const
{
    return m_passwd;
}

QString QXmppConfiguration::getStatus() const
{
    return m_status;
}

QString QXmppConfiguration::getResource() const
{
    return m_resource;
}

QString QXmppConfiguration::getJid() const
{
    return getJidBare() + "/" + m_resource;
}

QString QXmppConfiguration::getJidBare() const
{
    return m_user+"@"+m_domain;
}

bool QXmppConfiguration::getAutoAcceptSubscriptions() const
{
    return m_autoAcceptSubscriptions;
}

void QXmppConfiguration::setAutoAcceptSubscriptions(bool check)
{
    m_autoAcceptSubscriptions = check;
}

