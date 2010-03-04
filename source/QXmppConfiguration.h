/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
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


/// \class QXmppConfiguration
///
/// \brief The QXmppConfiguration is a configuration class. Its object can be passed
/// to the QXmppClient for connecting the an XMPP server.
///
/// It is a container of all the settings, configuration required for connecting to
/// an XMPP server. E.g. server name, username, port, type of authentication mechanism,
/// type of security used by stream (encryption) etc.
///

#ifndef QXMPPCONFIGURATION_H
#define QXMPPCONFIGURATION_H

#include <QString>
#include <QNetworkProxy>

class QXmppConfiguration
{
public:
    /// An enumeration for type of the Security Mode that is stream is encrypted or not.
    /// The server may or may not have TLS feature. Server may force the encryption.
    /// Depending upon all this user can specify following options.
    enum StreamSecurityMode
    {
        TLSEnabled = 0, ///< Default, encryption is used if available
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
       NonSASLDigest    ///< Default,
    };

    /// An enumeration for various SASL authentication mechanisms available.
    /// The server may or may not allow any particular mechanism. So depending
    /// upon the availability of mechanisms on the server the library will choose
    /// a mechanism.
    enum SASLAuthMechanism
    {
       SASLPlain = 0,
       SASLDigestMD5    ///< Default
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
// obsolete start
    QString getHost() const;
    QString getDomain() const;
    int getPort() const;
    QString getUser() const;
    QString getPasswd() const;
    QString getResource() const;
    QString getJid() const;
    QString getJidBare() const;

    bool getAutoAcceptSubscriptions() const;
    bool getAutoReconnectionEnabled() const;
    bool getUseSASLAuthentication() const;
    bool getIgnoreSslErrors() const;
    QXmppConfiguration::StreamSecurityMode getStreamSecurityMode() const;
    QXmppConfiguration::NonSASLAuthMechanism getNonSASLAuthMechanism() const;
    QXmppConfiguration::SASLAuthMechanism getSASLAuthMechanism() const;
    QNetworkProxy getNetworkProxy() const;
// obsolete end

private:
    QString m_host;
    int m_port;
    QString m_user;
    QString m_passwd;
    QString m_domain;
    QString m_resource;

    // default is true
    bool m_autoAcceptSubscriptions;
    // default is true
    bool m_sendIntialPresence;
    // default is true
    bool m_sendRosterRequest;
    // interval in seconds, if negative it won't ping
    int m_keepAlivePingsInterval;
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
