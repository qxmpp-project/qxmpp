/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#include <QNetworkProxy>
#include <QSslSocket>

#include "QXmppConfiguration.h"
#include "QXmppUtils.h"

class QXmppConfigurationPrivate : public QSharedData
{
public:
    QXmppConfigurationPrivate();

    QString host;
    int port;
    QString user;
    QString password;
    QString domain;
    QString resource;

    // Facebook
    QString facebookAccessToken;
    QString facebookAppId;

    // Google
    QString googleAccessToken;

    // Windows Live
    QString windowsLiveAccessToken;

    // default is false
    bool autoAcceptSubscriptions;
    // default is true
    bool sendIntialPresence;
    // default is true
    bool sendRosterRequest;
    // interval in seconds, if zero won't ping
    int keepAliveInterval;
    // interval in seconds, if zero won't timeout
    int keepAliveTimeout;
    // will keep reconnecting if disconnected, default is true
    bool autoReconnectionEnabled;
    // which authentication systems to use (if any)
    bool useSASLAuthentication;
    bool useNonSASLAuthentication;
    // default is true
    bool ignoreSslErrors;

    QXmppConfiguration::StreamSecurityMode streamSecurityMode;
    QXmppConfiguration::NonSASLAuthMechanism nonSASLAuthMechanism;
    QString saslAuthMechanism;

    QNetworkProxy networkProxy;

    QList<QSslCertificate> caCertificates;
};

QXmppConfigurationPrivate::QXmppConfigurationPrivate()
    : port(5222)
    , resource("QXmpp")
    , autoAcceptSubscriptions(false)
    , sendIntialPresence(true)
    , sendRosterRequest(true)
    , keepAliveInterval(60)
    , keepAliveTimeout(20)
    , autoReconnectionEnabled(true)
    , useSASLAuthentication(true)
    , useNonSASLAuthentication(true)
    , ignoreSslErrors(true)
    , streamSecurityMode(QXmppConfiguration::TLSEnabled)
    , nonSASLAuthMechanism(QXmppConfiguration::NonSASLDigest)
    , saslAuthMechanism("DIGEST-MD5")
{
}

/// Creates a QXmppConfiguration object.

QXmppConfiguration::QXmppConfiguration()
    : d(new QXmppConfigurationPrivate)
{
}

/// Creates a copy of \a other.

QXmppConfiguration::QXmppConfiguration(const QXmppConfiguration &other)
    : d(other.d)
{
}

/// Destructor, destroys the QXmppConfiguration object.
///

QXmppConfiguration::~QXmppConfiguration()
{
}

/// Assigns \a other to this QXmppConfiguration.

QXmppConfiguration& QXmppConfiguration::operator=(const QXmppConfiguration &other)
{
    d = other.d;
    return *this;
}

/// Sets the host name.
///
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
///

void QXmppConfiguration::setHost(const QString& host)
{
    d->host = host;
}

/// Sets the domain name.
///
/// \param domain Domain name e.g. "gmail.com" and "jabber.org".
/// \note host name and domain name can be different for google
/// domain name is gmail.com and host name is talk.google.com
///

void QXmppConfiguration::setDomain(const QString& domain)
{
    d->domain = domain;
}

/// Sets the port number.
///
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
///

void QXmppConfiguration::setPort(int port)
{
    d->port = port;
}

/// Sets the username.
///
/// \param user Username of the account at the specified XMPP server. It should
/// be the name without the domain name. E.g. "qxmpp.test1" and not
/// "qxmpp.test1@gmail.com"
///

void QXmppConfiguration::setUser(const QString& user)
{
    d->user = user;
}

/// Sets the password.
///
/// \param password Password for the specified username
///

void QXmppConfiguration::setPassword(const QString& password)
{
    d->password = password;
}

/// Sets the resource identifier.
///
/// Multiple resources (e.g., devices or locations) may connect simultaneously
/// to a server on behalf of each authorized client, with each resource
/// differentiated by the resource identifier of an XMPP address
/// (e.g. node\@domain/home vs. node\@domain/work)
///
/// The default value is "QXmpp".
///
/// \param resource Resource identifier of the client in connection.

void QXmppConfiguration::setResource(const QString& resource)
{
    d->resource = resource;
}

/// Sets the JID. If a full JID (i.e. one with a resource) is given, calling
/// this method will update the username, domain and resource. Otherwise, only
/// the username and the domain will be updated.
///
/// \param jid

void QXmppConfiguration::setJid(const QString& jid)
{
    d->user = QXmppUtils::jidToUser(jid);
    d->domain = QXmppUtils::jidToDomain(jid);
    const QString resource = QXmppUtils::jidToResource(jid);
    if (!resource.isEmpty())
        d->resource = resource;
}

/// Returns the host name.
///
/// \return host name
///

QString QXmppConfiguration::host() const
{
    return d->host;
}

/// Returns the domain name.
///
/// \return domain name
///

QString QXmppConfiguration::domain() const
{
    return d->domain;
}

/// Returns the port number.
///
/// \return port number
///

int QXmppConfiguration::port() const
{
    return d->port;
}

/// Returns the username.
///
/// \return username
///

QString QXmppConfiguration::user() const
{
    return d->user;
}

/// Returns the password.
///
/// \return password
///

QString QXmppConfiguration::password() const
{
    return d->password;
}

/// Returns the resource identifier.
///
/// \return resource identifier
///

QString QXmppConfiguration::resource() const
{
    return d->resource;
}

/// Returns the jabber id (jid).
///
/// \return jabber id (jid)
/// (e.g. "qxmpp.test1@gmail.com/resource" or qxmpptest@jabber.org/QXmpp156)
///

QString QXmppConfiguration::jid() const
{
    if (d->user.isEmpty())
        return d->domain;
    else
        return jidBare() + "/" + d->resource;
}

/// Returns the bare jabber id (jid), without the resource identifier.
///
/// \return bare jabber id (jid)
/// (e.g. "qxmpp.test1@gmail.com" or qxmpptest@jabber.org)
///

QString QXmppConfiguration::jidBare() const
{
    if (d->user.isEmpty())
        return d->domain;
    else
        return d->user+"@"+d->domain;
}

/// Returns the access token used for X-FACEBOOK-PLATFORM authentication.

QString QXmppConfiguration::facebookAccessToken() const
{
    return d->facebookAccessToken;
}

/// Sets the access token used for X-FACEBOOK-PLATFORM authentication.
///
/// This token is returned by Facebook at the end of the OAuth authentication
/// process.
///
/// \param accessToken

void QXmppConfiguration::setFacebookAccessToken(const QString& accessToken)
{
    d->facebookAccessToken = accessToken;
}

/// Returns the application ID used for X-FACEBOOK-PLATFORM authentication.

QString QXmppConfiguration::facebookAppId() const
{
    return d->facebookAppId;
}

/// Sets the application ID used for X-FACEBOOK-PLATFORM authentication.
///
/// \param appId

void QXmppConfiguration::setFacebookAppId(const QString& appId)
{
    d->facebookAppId = appId;
}

/// Returns the access token used for X-OAUTH2 authentication.

QString QXmppConfiguration::googleAccessToken() const
{
    return d->googleAccessToken;
}

/// Sets the access token used for X-OAUTH2 authentication.
///
/// This token is returned by Google at the end of the OAuth authentication
/// process.
///
/// \param accessToken

void QXmppConfiguration::setGoogleAccessToken(const QString& accessToken)
{
    d->googleAccessToken = accessToken;
}

/// Returns the access token used for X-MESSENGER-OAUTH2 authentication.

QString QXmppConfiguration::windowsLiveAccessToken() const
{
    return d->windowsLiveAccessToken;
}

/// Sets the access token used for X-MESSENGER-OAUTH2 authentication.
///
/// This token is returned by Windows Live at the end of the OAuth authentication
/// process.
///
/// \param accessToken

void QXmppConfiguration::setWindowsLiveAccessToken(const QString& accessToken)
{
    d->windowsLiveAccessToken = accessToken;
}

/// Returns the auto-accept-subscriptions-request configuration.
///
/// \return boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///

bool QXmppConfiguration::autoAcceptSubscriptions() const
{
    return d->autoAcceptSubscriptions;
}

/// Sets the auto-accept-subscriptions-request configuration.
///
/// \param value boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///

void QXmppConfiguration::setAutoAcceptSubscriptions(bool value)
{
    d->autoAcceptSubscriptions = value;
}

/// Returns the auto-reconnect-on-disconnection-on-error configuration.
///
/// \return boolean value
/// true means that auto-reconnect is enabled else disabled for false
///

bool QXmppConfiguration::autoReconnectionEnabled() const
{
    return d->autoReconnectionEnabled;
}

/// Sets the auto-reconnect-on-disconnection-on-error configuration.
///
/// \param value boolean value
/// true means that auto-reconnect is enabled else disabled for false
///

void QXmppConfiguration::setAutoReconnectionEnabled(bool value)
{
    d->autoReconnectionEnabled = value;
}

/// Returns whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to the XMPP server.

bool QXmppConfiguration::ignoreSslErrors() const
{
    return d->ignoreSslErrors;
}

/// Specifies whether SSL errors (such as certificate validation errors)
/// are to be ignored when connecting to an XMPP server.

void QXmppConfiguration::setIgnoreSslErrors(bool value)
{
    d->ignoreSslErrors = value;
}

/// Returns whether to make use of SASL authentication.

bool QXmppConfiguration::useSASLAuthentication() const
{
    return d->useSASLAuthentication;
}

/// Sets whether to make use of SASL authentication.

void QXmppConfiguration::setUseSASLAuthentication(bool useSASL)
{
    d->useSASLAuthentication = useSASL;
}

/// Returns whether to make use of non-SASL authentication.

bool QXmppConfiguration::useNonSASLAuthentication() const
{
    return d->useNonSASLAuthentication;
}

/// Sets whether to make use of non-SASL authentication.

void QXmppConfiguration::setUseNonSASLAuthentication(bool useNonSASL)
{
    d->useNonSASLAuthentication = useNonSASL;
}

/// Returns the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \return StreamSecurityMode

QXmppConfiguration::StreamSecurityMode QXmppConfiguration::streamSecurityMode() const
{
    return d->streamSecurityMode;
}

/// Specifies the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
/// \param mode StreamSecurityMode

void QXmppConfiguration::setStreamSecurityMode(
        QXmppConfiguration::StreamSecurityMode mode)
{
    d->streamSecurityMode = mode;
}

/// Returns the Non-SASL authentication mechanism configuration.
///
/// \return QXmppConfiguration::NonSASLAuthMechanism
///

QXmppConfiguration::NonSASLAuthMechanism QXmppConfiguration::nonSASLAuthMechanism() const
{
    return d->nonSASLAuthMechanism;
}

/// Hints the library the Non-SASL authentication mechanism to be used for authentication.
///
/// \param mech QXmppConfiguration::NonSASLAuthMechanism
///

void QXmppConfiguration::setNonSASLAuthMechanism(
        QXmppConfiguration::NonSASLAuthMechanism mech)
{
    d->nonSASLAuthMechanism = mech;
}

/// Returns the preferred SASL authentication mechanism.
///
/// Default value: "DIGEST-MD5"

QString QXmppConfiguration::saslAuthMechanism() const
{
    return d->saslAuthMechanism;
}

/// Sets the preferred SASL authentication \a mechanism.
///
/// Valid values: "PLAIN", "DIGEST-MD5", "ANONYMOUS", "X-FACEBOOK-PLATFORM"

void QXmppConfiguration::setSaslAuthMechanism(const QString &mechanism)
{
    d->saslAuthMechanism = mechanism;
}

/// Specifies the network proxy used for the connection made by QXmppClient.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \param proxy QNetworkProxy

void QXmppConfiguration::setNetworkProxy(const QNetworkProxy& proxy)
{
    d->networkProxy = proxy;
}

/// Returns the specified network proxy.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
/// \return QNetworkProxy

QNetworkProxy QXmppConfiguration::networkProxy() const
{
    return d->networkProxy;
}

/// Specifies the interval in seconds at which keep alive (ping) packets
/// will be sent to the server.
///
/// If set to zero, no keep alive packets will be sent.
///
/// The default value is 60 seconds.

void QXmppConfiguration::setKeepAliveInterval(int secs)
{
    d->keepAliveInterval = secs;
}

/// Returns the keep alive interval in seconds.
///
/// The default value is 60 seconds.

int QXmppConfiguration::keepAliveInterval() const
{
    return d->keepAliveInterval;
}

/// Specifies the maximum time in seconds to wait for a keep alive response
/// from the server before considering we are disconnected.
///
/// If set to zero or a value larger than the keep alive interval,
/// no timeout will occur.
///
/// The default value is 20 seconds.

void QXmppConfiguration::setKeepAliveTimeout(int secs)
{
    d->keepAliveTimeout = secs;
}

/// Returns the keep alive timeout in seconds.
///
/// The default value is 20 seconds.

int QXmppConfiguration::keepAliveTimeout() const
{
    return d->keepAliveTimeout;
}

/// Specifies a list of trusted CA certificates.

void QXmppConfiguration::setCaCertificates(const QList<QSslCertificate> &caCertificates)
{
    d->caCertificates = caCertificates;
}

/// Returns the a list of trusted CA certificates.

QList<QSslCertificate> QXmppConfiguration::caCertificates() const
{
    return d->caCertificates;
}
