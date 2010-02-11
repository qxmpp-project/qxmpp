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

QXmppConfiguration::QXmppConfiguration() : m_port(5222),
                m_resource("QXmpp"),
                m_autoAcceptSubscriptions(true),
                m_sendIntialPresence(true),
                m_sendRosterRequest(true),
                m_keepAlivePingsInterval(100),
                m_autoReconnectionEnabled(true),
                m_useSASLAuthentication(true),
                m_ignoreSslErrors(true),
                m_streamSecurityMode(QXmppConfiguration::TLSEnabled),
                m_nonSASLAuthMechanism(QXmppConfiguration::NonSASLDigest),
                m_SASLAuthMechanism(QXmppConfiguration::SASLDigestMD5)
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

bool QXmppConfiguration::getAutoReconnectionEnabled() const
{
    return m_autoReconnectionEnabled;
}

void QXmppConfiguration::setAutoReconnectionEnabled(bool value)
{
    m_autoReconnectionEnabled = value;
}

/// Returns whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to the XMPP server.

bool QXmppConfiguration::getIgnoreSslErrors() const
{
    return m_ignoreSslErrors;
}

/// Specifies whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to an XMPP server.

void QXmppConfiguration::setIgnoreSslErrors(bool value)
{
    m_ignoreSslErrors = value;
}

/// Returns the type of authentication system specified by the user.
/// \return true if SASL was specified else false. If the specified
/// system is not available QXmpp will resort to the other one.

bool QXmppConfiguration::getUseSASLAuthentication() const
{
    return m_useSASLAuthentication;
}

/// Returns the type of authentication system specified by the user.
/// \param useSASL to hint to use SASL authentication system if available.
/// false will specify to use NonSASL XEP-0078: Non-SASL Authentication
/// If the specified one is not availbe, library will use the othe one

void QXmppConfiguration::setUseSASLAuthentication(bool useSASL)
{
    m_useSASLAuthentication = useSASL;
}

/// Returns the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \return StreamSecurityMode

QXmppConfiguration::StreamSecurityMode QXmppConfiguration::getStreamSecurityMode() const
{
    return m_streamSecurityMode;
}

/// Specifies the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \param mode StreamSecurityMode

void QXmppConfiguration::setStreamSecurityMode(
        QXmppConfiguration::StreamSecurityMode mode)
{
    m_streamSecurityMode = mode;
}

QXmppConfiguration::NonSASLAuthMechanism QXmppConfiguration::getNonSASLAuthMechanism() const
{
    return m_nonSASLAuthMechanism;
}

void QXmppConfiguration::setNonSASLAuthMechanism(
        QXmppConfiguration::NonSASLAuthMechanism mech)
{
    m_nonSASLAuthMechanism = mech;
}

QXmppConfiguration::SASLAuthMechanism QXmppConfiguration::getSASLAuthMechanism() const
{
    return m_SASLAuthMechanism;
}

void QXmppConfiguration::setSASLAuthMechanism(
        QXmppConfiguration::SASLAuthMechanism mech)
{
    m_SASLAuthMechanism = mech;
}

/// Specifies the network proxy used for the connection made by QXmppClient.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \param proxy QNetworkProxy

void QXmppConfiguration::setNetworkProxy(const QNetworkProxy& proxy)
{
    m_networkProxy = proxy;
}

/// Returns the specified network proxy.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \return QNetworkProxy

QNetworkProxy QXmppConfiguration::getNetworkProxy() const
{
    return m_networkProxy;
}

