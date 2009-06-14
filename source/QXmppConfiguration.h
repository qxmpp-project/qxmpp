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


#ifndef QXMPPCONFIGURATION_H
#define QXMPPCONFIGURATION_H

#include <QString>

class QXmppConfiguration
{
public:
    QXmppConfiguration();
    ~QXmppConfiguration();

    void setHost(const QString&);
    void setDomain(const QString&);
    void setPort(int);
    void setUser(const QString&);
    void setPasswd(const QString&);
    void setStatus(const QString&);
    void setResource(const QString&);

    QString getHost() const;
    QString getDomain() const;
    int getPort() const;
    QString getUser() const;
    QString getPasswd() const;
    QString getStatus() const;
    QString getResource() const;
    QString getJid() const;
    QString getJidBare() const;

    bool getAutoAcceptSubscriptions() const;
    void setAutoAcceptSubscriptions(bool);

private:
    QString m_host;
    int m_port;
    QString m_user;
    QString m_passwd;
    QString m_domain;
    QString m_status;
    QString m_resource;

    bool m_autoAcceptSubscriptions; // default is true
    bool m_sendIntialPresence;      // default is true
    bool m_sendRosterRequest;       // default is true
    int m_keepAlivePingsInterval;   // interval in seconds, if negative it won't ping
};

#endif // QXMPPCONFIGURATION_H
