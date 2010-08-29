/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

class QXmppSaslDigestMd5
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

    QByteArray realm() const;
    void setRealm(const QByteArray &realm);

    QByteArray username() const;
    void setUsername(const QByteArray &username);

    void setPassword(const QByteArray &password);

    QByteArray calculateDigest(const QByteArray &a2) const;
    static QByteArray generateNonce();

private:
    QByteArray m_authzid;
    QByteArray m_cnonce;
    QByteArray m_digestUri;
    QByteArray m_nc;
    QByteArray m_nonce;
    QByteArray m_realm;
    QByteArray m_username;
    QByteArray m_password;
};

#endif
