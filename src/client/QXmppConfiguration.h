/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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
#include <QNetworkProxy>
#include <QSslCertificate>

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

class QXmppConfiguration
{
public:
    /// An enumeration for type of the Security Mode that is stream is encrypted or not.
    /// The server may or may not have TLS feature. Server may force the encryption.
    /// Depending upon all this user can specify following options.
    enum StreamSecurityMode
    {
        TLSEnabled = 0, ///< Encryption is used if available (default)
        TLSDisabled,    ///< No encryption is server allows
        TLSRequired     ///< Encryption is a must otherwise connection would not
                        ///< be established
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
    enum SASLAuthMechanism
    {
        SASLPlain = 0,         ///< Plain
        SASLDigestMD5,         ///< Digest MD5 (default)
        SASLAnonymous,         ///< Anonymous
        SASLXFacebookPlatform, ///< Facebook Platform
    };

    /// An enumeration for stream compression methods.
    enum CompressionMethod
    {
        ZlibCompression = 0 ///< zlib compression
    };

    QXmppConfiguration();
    ~QXmppConfiguration();

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

    bool autoAcceptSubscriptions() const;
    void setAutoAcceptSubscriptions(bool);

    bool autoReconnectionEnabled() const;
    void setAutoReconnectionEnabled(bool);

    bool useSASLAuthentication() const;
    void setUseSASLAuthentication(bool);

    bool ignoreSslErrors() const;
    void setIgnoreSslErrors(bool);

    QXmppConfiguration::StreamSecurityMode streamSecurityMode() const;
    void setStreamSecurityMode(QXmppConfiguration::StreamSecurityMode mode);

    QXmppConfiguration::NonSASLAuthMechanism nonSASLAuthMechanism() const;
    void setNonSASLAuthMechanism(QXmppConfiguration::NonSASLAuthMechanism);

    QXmppConfiguration::SASLAuthMechanism sASLAuthMechanism() const;
    void setSASLAuthMechanism(QXmppConfiguration::SASLAuthMechanism);

    QNetworkProxy networkProxy() const;
    void setNetworkProxy(const QNetworkProxy& proxy);

    int keepAliveInterval() const;
    void setKeepAliveInterval(int secs);

    int keepAliveTimeout() const;
    void setKeepAliveTimeout(int secs);

    QList<QSslCertificate> caCertificates() const;
    void setCaCertificates(const QList<QSslCertificate> &);

private:
    QString m_host;
    int m_port;
    QString m_user;
    QString m_password;
    QString m_domain;
    QString m_resource;

    // Facebook
    QString m_facebookAccessToken;
    QString m_facebookAppId;

    // default is false
    bool m_autoAcceptSubscriptions;
    // default is true
    bool m_sendIntialPresence;
    // default is true
    bool m_sendRosterRequest;
    // interval in seconds, if zero won't ping
    int m_keepAliveInterval;
    // interval in seconds, if zero won't timeout
    int m_keepAliveTimeout;
    // will keep reconnecting if disconnected, default is true
    bool m_autoReconnectionEnabled;
    bool m_useSASLAuthentication; ///< flag to specify what authentication system
                                  ///< to be used
                                ///< defualt is true and use SASL
                                ///< false would use NonSASL if available
    // default is true
    bool m_ignoreSslErrors;

    StreamSecurityMode m_streamSecurityMode;
    NonSASLAuthMechanism m_nonSASLAuthMechanism;
    SASLAuthMechanism m_SASLAuthMechanism;

    QNetworkProxy m_networkProxy;

    QList<QSslCertificate> m_caCertificates;
};

#endif // QXMPPCONFIGURATION_H
