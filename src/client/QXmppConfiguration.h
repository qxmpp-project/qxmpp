// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCONFIGURATION_H
#define QXMPPCONFIGURATION_H

#include "QXmppGlobal.h"

#include <optional>

#include <QSharedDataPointer>
#include <QString>

class QNetworkProxy;
class QSslCertificate;
class QXmppConfigurationPrivate;
class QXmppCredentials;
class QXmppSasl2UserAgent;

namespace QXmpp::Private {
struct Credentials;
}

///
/// \brief The QXmppConfiguration class holds configuration options.
///
/// It can be passed to QXmppClient to specify the options when connecting to
/// an XMPP server.
///
/// It is a container of all the settings, configuration required for
/// connecting to an XMPP server. E.g. server name, username, port, type
/// of authentication mechanism, type of security used by stream (encryption),
/// etc..
///
class QXMPP_EXPORT QXmppConfiguration
{
public:
    /// An enumeration for type of the Security Mode that is stream is encrypted or not.
    /// The server may or may not have TLS feature. Server may force the encryption.
    /// Depending upon all this user can specify following options.
    enum StreamSecurityMode {
        TLSEnabled = 0,  ///< Encryption is used if available (default).
        TLSDisabled,     ///< No encryption even if the server offers it.
        TLSRequired,     ///< Encryption must be available, otherwise the
                         ///< connection will not be established.
        LegacySSL        ///< Use only legacy SSL mode.
    };

    /// An enumeration for various Non-SASL authentication mechanisms available.
    /// The server may or may not allow QXmppConfiguration::Plain mechanism. So
    /// specifying the mechanism is just a hint to the library.
    enum NonSASLAuthMechanism {
        NonSASLPlain = 0,  ///< Plain
        NonSASLDigest      ///< Digest (default)
    };

    QXmppConfiguration();
    QXmppConfiguration(const QXmppConfiguration &other);
    ~QXmppConfiguration();
    QXmppConfiguration &operator=(const QXmppConfiguration &other);

    QString host() const;
    void setHost(const QString &);

    QString domain() const;
    void setDomain(const QString &);

    int port() const;
    void setPort(int);

    QString user() const;
    void setUser(const QString &);

    QString password() const;
    void setPassword(const QString &);

    QString resource() const;
    void setResource(const QString &);

    QString resourcePrefix() const;
    void setResourcePrefix(const QString &);

    QString jid() const;
    void setJid(const QString &jid);

    QString jidBare() const;

    QXmppCredentials credentials() const;
    void setCredentials(const QXmppCredentials &);

    QString facebookAccessToken() const;
    void setFacebookAccessToken(const QString &);

    QString facebookAppId() const;
    void setFacebookAppId(const QString &);

    QString googleAccessToken() const;
    void setGoogleAccessToken(const QString &accessToken);

    QString windowsLiveAccessToken() const;
    void setWindowsLiveAccessToken(const QString &accessToken);

    bool autoAcceptSubscriptions() const;
    void setAutoAcceptSubscriptions(bool);

    bool autoReconnectionEnabled() const;
    void setAutoReconnectionEnabled(bool);

    bool useSasl2Authentication() const;
    void setUseSasl2Authentication(bool);

    bool useFastTokenAuthentication() const;
    void setUseFastTokenAuthentication(bool);

    bool useSASLAuthentication() const;
    void setUseSASLAuthentication(bool);

    bool useNonSASLAuthentication() const;
    void setUseNonSASLAuthentication(bool);

    bool ignoreSslErrors() const;
    void setIgnoreSslErrors(bool);

    QXmppConfiguration::StreamSecurityMode streamSecurityMode() const;
    void setStreamSecurityMode(QXmppConfiguration::StreamSecurityMode mode);

    QXmppConfiguration::NonSASLAuthMechanism nonSASLAuthMechanism() const;
    void setNonSASLAuthMechanism(QXmppConfiguration::NonSASLAuthMechanism);

    QString saslAuthMechanism() const;
    void setSaslAuthMechanism(const QString &mechanism);

    QList<QString> disabledSaslMechanisms() const;
    void addDisabledSaslMechanism(const QString &);
    void setDisabledSaslMechanisms(const QList<QString> &);

    std::optional<QXmppSasl2UserAgent> sasl2UserAgent() const;
    void setSasl2UserAgent(const std::optional<QXmppSasl2UserAgent> &);

    QNetworkProxy networkProxy() const;
    void setNetworkProxy(const QNetworkProxy &proxy);

    int keepAliveInterval() const;
    void setKeepAliveInterval(int secs);

    int keepAliveTimeout() const;
    void setKeepAliveTimeout(int secs);

    QList<QSslCertificate> caCertificates() const;
    void setCaCertificates(const QList<QSslCertificate> &);

    /// \cond
    const QXmpp::Private::Credentials &credentialData() const;
    QXmpp::Private::Credentials &credentialData();
    /// \endcond

private:
    QSharedDataPointer<QXmppConfigurationPrivate> d;
};

#endif  // QXMPPCONFIGURATION_H
