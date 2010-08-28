/*
 * Copyright (C) 2008-2010 The QXmpp developers
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
        SASLPlain = 0,  ///< Plain
        SASLDigestMD5,  ///< Digest MD5 (default)
        SASLAnonymous   ///< Anonymous
    };

    /// An enumeration for stream compression methods.
    enum CompressionMethod
    {
        ZlibCompression = 0 ///< zlib compression
    };

    QXmppConfiguration();
    ~QXmppConfiguration();

    QString host() const;
    QString domain() const;
    int port() const;
    QString user() const;
    QString passwd() const;
    QString resource() const;
    QString jid() const;
    QString jidBare() const;

    bool autoAcceptSubscriptions() const;
    bool autoReconnectionEnabled() const;
    bool useSASLAuthentication() const;
    bool ignoreSslErrors() const;
    QXmppConfiguration::StreamSecurityMode streamSecurityMode() const;
    QXmppConfiguration::NonSASLAuthMechanism nonSASLAuthMechanism() const;
    QXmppConfiguration::SASLAuthMechanism sASLAuthMechanism() const;
    QNetworkProxy networkProxy() const;

    int keepAliveInterval() const;
    void setKeepAliveInterval(int secs);
    int keepAliveTimeout() const;
    void setKeepAliveTimeout(int secs);

    void setHost(const QString&);
    void setDomain(const QString&);
    void setPort(int);
    void setUser(const QString&);
    void setPasswd(const QString&);
    void setResource(const QString&);

    void setAutoAcceptSubscriptions(bool);
    void setAutoReconnectionEnabled(bool);
    void setUseSASLAuthentication(bool);
    void setIgnoreSslErrors(bool);

    void setStreamSecurityMode(QXmppConfiguration::StreamSecurityMode mode);
    void setNonSASLAuthMechanism(QXmppConfiguration::NonSASLAuthMechanism);
    void setSASLAuthMechanism(QXmppConfiguration::SASLAuthMechanism);

    void setNetworkProxy(const QNetworkProxy& proxy);

    // deprecated accessors, use the form without "get" instead
    /// \cond
    QString Q_DECL_DEPRECATED getHost() const;
    QString Q_DECL_DEPRECATED getDomain() const;
    int Q_DECL_DEPRECATED getPort() const;
    QString Q_DECL_DEPRECATED getUser() const;
    QString Q_DECL_DEPRECATED getPasswd() const;
    QString Q_DECL_DEPRECATED getResource() const;
    QString Q_DECL_DEPRECATED getJid() const;
    QString Q_DECL_DEPRECATED getJidBare() const;

    bool Q_DECL_DEPRECATED getAutoAcceptSubscriptions() const;
    bool Q_DECL_DEPRECATED getAutoReconnectionEnabled() const;
    bool Q_DECL_DEPRECATED getUseSASLAuthentication() const;
    bool Q_DECL_DEPRECATED getIgnoreSslErrors() const;
    QXmppConfiguration::StreamSecurityMode Q_DECL_DEPRECATED getStreamSecurityMode() const;
    QXmppConfiguration::NonSASLAuthMechanism Q_DECL_DEPRECATED getNonSASLAuthMechanism() const;
    QXmppConfiguration::SASLAuthMechanism Q_DECL_DEPRECATED getSASLAuthMechanism() const;
    QNetworkProxy Q_DECL_DEPRECATED getNetworkProxy() const;
    /// \endcond

private:
    QString m_host;
    int m_port;
    QString m_user;
    QString m_passwd;
    QString m_domain;
    QString m_resource;

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
};

#endif // QXMPPCONFIGURATION_H
