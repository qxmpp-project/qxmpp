/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#ifndef QXMPPSASLAUTH_H
#define QXMPPSASLAUTH_H

#include <QByteArray>
#include <QMap>

#include "QXmppGlobal.h"

class QXmppSaslClientPrivate;

class QXMPP_EXPORT QXmppSaslDigestMd5
{
public:
    QByteArray authzid() const;
    void setAuthzid(const QByteArray &cnonce);

    QByteArray cnonce() const;
    void setCnonce(const QByteArray &cnonce);

    QByteArray digestUri() const;
    void setDigestUri(const QByteArray &digestUri);

    QByteArray nc() const;
    void setNc(const QByteArray &nc);

    QByteArray nonce() const;
    void setNonce(const QByteArray &nonce);

    QByteArray qop() const;
    void setQop(const QByteArray &qop);

    void setSecret(const QByteArray &secret);

    QByteArray calculateDigest(const QByteArray &A2) const;
    QByteArray calculateDigest(const QByteArray &A1, const QByteArray &A2) const;
    static QByteArray generateNonce();

    // message parsing and serialization
    static QMap<QByteArray, QByteArray> parseMessage(const QByteArray &ba);
    static QByteArray serializeMessage(const QMap<QByteArray, QByteArray> &map);

private:
    QByteArray m_authzid;
    QByteArray m_cnonce;
    QByteArray m_digestUri;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_qop;
    QByteArray m_secret;
};

/// The QXmppSaslClient class is the base class for all SASL client
//  authentication methods.

class QXMPP_EXPORT QXmppSaslClient
{
public:
    QXmppSaslClient();
    virtual ~QXmppSaslClient();

    QString host() const;
    void setHost(const QString &host);

    QString serviceType() const;
    void setServiceType(const QString &serviceType);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    virtual QString mechanism() const = 0;
    virtual bool respond(const QByteArray &challenge, QByteArray &response) = 0;

    static QXmppSaslClient* create(const QString &mechanism);

private:
    QXmppSaslClientPrivate *d;
};

#endif
