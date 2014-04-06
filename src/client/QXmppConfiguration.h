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


#ifndef QXMPPCONFIGURATION_H
#define QXMPPCONFIGURATION_H

#include <QString>
#include <QSharedDataPointer>

#include "QXmppGlobal.h"

class QNetworkProxy;
class QSslCertificate;
class QXmppConfigurationPrivate;

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
    enum StreamSecurityMode
    {
        TLSEnabled = 0, ///< Encryption is used if available (default).
        TLSDisabled,    ///< No encryption even if the server offers it.
        TLSRequired,    ///< Encryption must be available, otherwise the
                        ///< connection will not be established.
        LegacySSL       ///< Use only legacy SSL mode.
    };

    /// An enumeration for various Non-SASL authentication mechanisms available.
    /// The server may or may not allow QXmppConfiguration::Plain mechanism. So
    /// specifying the mechanism is just a hint to the library.
    enum NonSASLAuthMechanism
    {
        NonSASLPlain = 0,///< Plain
        NonSASLDigest    ///< Digest (default)
    };

    /// An enumeration for various SASL authentication mechanisms available.
    /// The server may or may not allow any particular mechanism. So depending
    /// upon the availability of mechanisms on the server the library will choose
    /// a mechanism.

    QXmppConfiguration();
    QXmppConfiguration(const QXmppConfiguration &other);
    ~QXmppConfiguration();
    QXmppConfiguration& operator=(const QXmppConfiguration &other);

    QString host() const;
    void setHost(const QString&);

    QString domain() const;
    void setDomain(const QString&);

    int port() const;
    void setPort(int);

    QString user() const;
    void setUser(const QString&);

    QString password() const;
    void setPassword(const QString&);

    QString resource() const;
    void setResource(const QString&);

    QString jid() const;
    void setJid(const QString &jid);

    QString jidBare() const;

    QString facebookAccessToken() const;
    void setFacebookAccessToken(const QString&);

    QString facebookAppId() const;
    void setFacebookAppId(const QString&);

    QString googleAccessToken() const;
    void setGoogleAccessToken(const QString &accessToken);

    QString windowsLiveAccessToken() const;
    void setWindowsLiveAccessToken(const QString &accessToken);

    bool autoAcceptSubscriptions() const;
    void setAutoAcceptSubscriptions(bool);

    bool autoReconnectionEnabled() const;
    void setAutoReconnectionEnabled(bool);

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

    QNetworkProxy networkProxy() const;
    void setNetworkProxy(const QNetworkProxy& proxy);

    int keepAliveInterval() const;
    void setKeepAliveInterval(int secs);

    int keepAliveTimeout() const;
    void setKeepAliveTimeout(int secs);

    QList<QSslCertificate> caCertificates() const;
    void setCaCertificates(const QList<QSslCertificate> &);

private:
    QSharedDataPointer<QXmppConfigurationPrivate> d;
};

#endif // QXMPPCONFIGURATION_H
