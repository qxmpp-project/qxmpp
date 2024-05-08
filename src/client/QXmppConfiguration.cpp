// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConfiguration.h"

#include "QXmppConstants_p.h"
#include "QXmppSasl2UserAgent.h"
#include "QXmppUtils.h"

#include <QCoreApplication>
#include <QNetworkProxy>
#include <QSslSocket>

using namespace QXmpp::Private;

class QXmppConfigurationPrivate : public QSharedData
{
public:
    QString host;
    int port = XMPP_DEFAULT_PORT;
    QString user;
    QString password;
    QString domain;
    QString resource = QStringLiteral("QXmpp");
    QString resourcePrefix;

    // Facebook
    QString facebookAccessToken;
    QString facebookAppId;

    // Google
    QString googleAccessToken;

    // Windows Live
    QString windowsLiveAccessToken;

    bool autoAcceptSubscriptions = false;
    bool sendIntialPresence = true;
    bool sendRosterRequest = true;
    // interval in seconds, if zero won't ping
    int keepAliveInterval = 60;
    // interval in seconds, if zero won't timeout
    int keepAliveTimeout = 20;
    // will keep reconnecting if disconnected, default is true
    bool autoReconnectionEnabled = true;
    // which authentication systems to use (if any)
    bool useSasl2Authentication = true;
    bool useSASLAuthentication = true;
    bool useNonSASLAuthentication = true;
    bool ignoreSslErrors = false;

    QXmppConfiguration::StreamSecurityMode streamSecurityMode = QXmppConfiguration::TLSEnabled;
    QXmppConfiguration::NonSASLAuthMechanism nonSASLAuthMechanism = QXmppConfiguration::NonSASLDigest;
    QString saslAuthMechanism;
    QList<QString> disabledSaslMechanisms = { QStringLiteral("PLAIN") };
    std::optional<QXmppSasl2UserAgent> sasl2UserAgent;

    QNetworkProxy networkProxy;

    QList<QSslCertificate> caCertificates;
};

/// Creates a QXmppConfiguration object.
QXmppConfiguration::QXmppConfiguration()
    : d(new QXmppConfigurationPrivate)
{
}

/// Creates a copy of \a other.
QXmppConfiguration::QXmppConfiguration(const QXmppConfiguration &other) = default;

QXmppConfiguration::~QXmppConfiguration() = default;

/// Assigns \a other to this QXmppConfiguration.
QXmppConfiguration &QXmppConfiguration::operator=(const QXmppConfiguration &other) = default;

///
/// Sets the host name.
///
/// \param host host name of the XMPP server where connection has to be made
/// (e.g. "jabber.org" and "talk.google.com"). It can also be an IP address in
/// the form of a string (e.g. "192.168.1.25").
///
void QXmppConfiguration::setHost(const QString &host)
{
    d->host = host;
}

///
/// Sets the domain name.
///
/// \param domain Domain name e.g. "gmail.com" and "jabber.org".
/// \note host name and domain name can be different for google
/// domain name is gmail.com and host name is talk.google.com
///
void QXmppConfiguration::setDomain(const QString &domain)
{
    d->domain = domain;
}

///
/// Sets the port number.
///
/// \param port Port number at which the XMPP server is listening. The default
/// value is 5222.
///
void QXmppConfiguration::setPort(int port)
{
    d->port = port;
}

///
/// Sets the username.
///
/// \param user Username of the account at the specified XMPP server. It should
/// be the name without the domain name. E.g. "qxmpp.test1" and not
/// "qxmpp.test1@gmail.com"
///
void QXmppConfiguration::setUser(const QString &user)
{
    d->user = user;
}

///
/// Sets the password.
///
/// \param password Password for the specified username
///
void QXmppConfiguration::setPassword(const QString &password)
{
    d->password = password;
}

///
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
///
void QXmppConfiguration::setResource(const QString &resource)
{
    d->resource = resource;
}

///
/// Returns the resource prefix ('tag' for this client) used when \xep{0386, Bind 2} is available.
///
/// \since QXmpp 1.8
///
QString QXmppConfiguration::resourcePrefix() const
{
    return d->resourcePrefix;
}

///
/// Sets the resource prefix ('tag' for this client) used when \xep{0386, Bind 2} is available.
///
/// \since QXmpp 1.8
///
void QXmppConfiguration::setResourcePrefix(const QString &resourcePrefix)
{
    d->resourcePrefix = resourcePrefix;
}

///
/// Sets the JID. If a full JID (i.e. one with a resource) is given, calling
/// this method will update the username, domain and resource. Otherwise, only
/// the username and the domain will be updated.
///
void QXmppConfiguration::setJid(const QString &jid)
{
    d->user = QXmppUtils::jidToUser(jid);
    d->domain = QXmppUtils::jidToDomain(jid);
    const QString resource = QXmppUtils::jidToResource(jid);
    if (!resource.isEmpty()) {
        d->resource = resource;
    }
}

/// Returns the custom hostname to connect to.
QString QXmppConfiguration::host() const
{
    return d->host;
}

/// Returns the domain part of the JID.
QString QXmppConfiguration::domain() const
{
    return d->domain;
}

/// Returns the port number.
int QXmppConfiguration::port() const
{
    return d->port;
}

/// Returns the localpart of the JID.
QString QXmppConfiguration::user() const
{
    return d->user;
}

/// Returns the password.
QString QXmppConfiguration::password() const
{
    return d->password;
}

/// Returns the resource identifier.
QString QXmppConfiguration::resource() const
{
    return d->resource;
}

///
/// Returns the Jabber-ID (JID).
///
/// \return Jabber-ID (JID)
/// (e.g. "qxmpp.test1@gmail.com/resource" or qxmpptest@jabber.org/QXmpp156)
///
QString QXmppConfiguration::jid() const
{
    if (d->user.isEmpty()) {
        return d->domain;
    } else {
        return jidBare() + u'/' + d->resource;
    }
}

///
/// Returns the bare jabber id (jid), without the resource identifier.
///
/// \return bare jabber id (jid)
/// (e.g. "qxmpp.test1@gmail.com" or qxmpptest@jabber.org)
///
QString QXmppConfiguration::jidBare() const
{
    if (d->user.isEmpty()) {
        return d->domain;
    } else {
        return d->user + u'@' + d->domain;
    }
}

/// Returns the access token used for X-FACEBOOK-PLATFORM authentication.
QString QXmppConfiguration::facebookAccessToken() const
{
    return d->facebookAccessToken;
}

///
/// Sets the access token used for X-FACEBOOK-PLATFORM authentication.
///
/// This token is returned by Facebook at the end of the OAuth authentication
/// process.
///
void QXmppConfiguration::setFacebookAccessToken(const QString &accessToken)
{
    d->facebookAccessToken = accessToken;
}

/// Returns the application ID used for X-FACEBOOK-PLATFORM authentication.
QString QXmppConfiguration::facebookAppId() const
{
    return d->facebookAppId;
}

/// Sets the application ID used for X-FACEBOOK-PLATFORM authentication.
void QXmppConfiguration::setFacebookAppId(const QString &appId)
{
    d->facebookAppId = appId;
}

/// Returns the access token used for X-OAUTH2 authentication.
QString QXmppConfiguration::googleAccessToken() const
{
    return d->googleAccessToken;
}

///
/// Sets the access token used for X-OAUTH2 authentication.
///
/// This token is returned by Google at the end of the OAuth authentication
/// process.
///
void QXmppConfiguration::setGoogleAccessToken(const QString &accessToken)
{
    d->googleAccessToken = accessToken;
}

/// Returns the access token used for X-MESSENGER-OAUTH2 authentication.
QString QXmppConfiguration::windowsLiveAccessToken() const
{
    return d->windowsLiveAccessToken;
}

///
/// Sets the access token used for X-MESSENGER-OAUTH2 authentication.
///
/// This token is returned by Windows Live at the end of the OAuth authentication
/// process.
///
void QXmppConfiguration::setWindowsLiveAccessToken(const QString &accessToken)
{
    d->windowsLiveAccessToken = accessToken;
}

///
/// Returns the auto-accept-subscriptions-request configuration.
///
/// \return boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///
bool QXmppConfiguration::autoAcceptSubscriptions() const
{
    return d->autoAcceptSubscriptions;
}

///
/// Sets the auto-accept-subscriptions-request configuration.
///
/// \param value boolean value
/// true means that auto-accept-subscriptions-request is enabled else disabled for false
///
void QXmppConfiguration::setAutoAcceptSubscriptions(bool value)
{
    d->autoAcceptSubscriptions = value;
}

///
/// Returns the auto-reconnect-on-disconnection-on-error configuration.
///
/// \return boolean value
/// true means that auto-reconnect is enabled else disabled for false
///
bool QXmppConfiguration::autoReconnectionEnabled() const
{
    return d->autoReconnectionEnabled;
}

///
/// Sets the auto-reconnect-on-disconnection-on-error configuration.
///
/// \param value boolean value
/// true means that auto-reconnect is enabled else disabled for false
///
void QXmppConfiguration::setAutoReconnectionEnabled(bool value)
{
    d->autoReconnectionEnabled = value;
}

///
/// Returns whether SASL 2 (\xep{0388, Extensible SASL Profile}) authentication is used if
/// available.
///
bool QXmppConfiguration::useSasl2Authentication() const
{
    return d->useSasl2Authentication;
}

///
/// Sets whether to use SASL 2 (\xep{0388, Extensible SASL Profile}) authentication if available.
///
/// \since QXmpp 1.7
///
void QXmppConfiguration::setUseSasl2Authentication(bool enabled)
{
    d->useSasl2Authentication = enabled;
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
QXmppConfiguration::StreamSecurityMode QXmppConfiguration::streamSecurityMode() const
{
    return d->streamSecurityMode;
}

/// Specifies the specified security mode for the stream. The default value is
/// QXmppConfiguration::TLSEnabled.
void QXmppConfiguration::setStreamSecurityMode(
    QXmppConfiguration::StreamSecurityMode mode)
{
    d->streamSecurityMode = mode;
}

/// Returns the Non-SASL authentication mechanism configuration.
QXmppConfiguration::NonSASLAuthMechanism QXmppConfiguration::nonSASLAuthMechanism() const
{
    return d->nonSASLAuthMechanism;
}

/// Hints the library the Non-SASL authentication mechanism to be used for authentication.
void QXmppConfiguration::setNonSASLAuthMechanism(
    QXmppConfiguration::NonSASLAuthMechanism mech)
{
    d->nonSASLAuthMechanism = mech;
}

/// Returns the preferred SASL authentication mechanism.
QString QXmppConfiguration::saslAuthMechanism() const
{
    return d->saslAuthMechanism;
}

///
/// Sets the preferred SASL authentication \a mechanism.
///
/// Valid values: "SCRAM-SHA-256", "SCRAM-SHA-1", "DIGEST-MD5", "PLAIN", "ANONYMOUS",
//                "X-FACEBOOK-PLATFORM", "X-MESSENGER-OAUTH2", "X-OAUTH2"
///
void QXmppConfiguration::setSaslAuthMechanism(const QString &mechanism)
{
    d->saslAuthMechanism = mechanism;
}

///
/// Returns the list of disabled SASL mechanisms.
///
/// Those mechanisms are not used by the client, even if no other mechanism is available.
///
/// \since QXmpp 1.7
///
QList<QString> QXmppConfiguration::disabledSaslMechanisms() const
{
    return d->disabledSaslMechanisms;
}

///
/// Adds to the list of disabled SASL mechanisms.
///
/// Those mechanisms are not used by the client, even if no other mechanism is available.
///
/// \since QXmpp 1.7
///
void QXmppConfiguration::addDisabledSaslMechanism(const QString &m)
{
    if (!d->disabledSaslMechanisms.contains(m)) {
        d->disabledSaslMechanisms.push_back(m);
    }
}

///
/// Sets the list of disabled SASL mechanisms.
///
/// Those mechanisms are not used by the client, even if no other mechanism is available.
///
/// \since QXmpp 1.7
///
void QXmppConfiguration::setDisabledSaslMechanisms(const QList<QString> &disabled)
{
    d->disabledSaslMechanisms = disabled;
}

///
/// Returns the user-agent used for \xep{0388, Extensible SASL Profile}.
///
/// If this is empty, no user-agent will be sent.
///
/// \since QXmpp 1.7
///
std::optional<QXmppSasl2UserAgent> QXmppConfiguration::sasl2UserAgent() const
{
    return d->sasl2UserAgent;
}

///
/// Sets the user-agent used for \xep{0388, Extensible SASL Profile}.
///
/// If this is empty, no user-agent will be sent.
///
/// \since QXmpp 1.7
///
void QXmppConfiguration::setSasl2UserAgent(const std::optional<QXmppSasl2UserAgent> &userAgent)
{
    d->sasl2UserAgent = userAgent;
}

///
/// Specifies the network proxy used for the connection made by QXmppClient.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
///
void QXmppConfiguration::setNetworkProxy(const QNetworkProxy &proxy)
{
    d->networkProxy = proxy;
}

///
/// Returns the specified network proxy.
/// The default value is QNetworkProxy::DefaultProxy that is the proxy is
/// determined based on the application proxy set using
/// QNetworkProxy::setApplicationProxy().
///
QNetworkProxy QXmppConfiguration::networkProxy() const
{
    return d->networkProxy;
}

///
/// Specifies the interval in seconds at which keep alive (ping) packets
/// will be sent to the server.
///
/// If set to zero, no keep alive packets will be sent.
///
/// The default value is 60 seconds.
///
void QXmppConfiguration::setKeepAliveInterval(int secs)
{
    d->keepAliveInterval = secs;
}

///
/// Returns the keep alive interval in seconds.
///
/// The default value is 60 seconds.
///
int QXmppConfiguration::keepAliveInterval() const
{
    return d->keepAliveInterval;
}

///
/// Specifies the maximum time in seconds to wait for a keep alive response
/// from the server before considering we are disconnected.
///
/// If set to zero or a value larger than the keep alive interval,
/// no timeout will occur.
///
/// The default value is 20 seconds.
///
void QXmppConfiguration::setKeepAliveTimeout(int secs)
{
    d->keepAliveTimeout = secs;
}

///
/// Returns the keep alive timeout in seconds.
///
/// The default value is 20 seconds.
///
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
