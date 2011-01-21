/*
 * Copyright (C) 2008-2011 The QXmpp developers
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

#include <cstdlib>

#include <QCryptographicHash>

#include "QXmppSaslAuth.h"
#include "QXmppUtils.h"

QByteArray QXmppSaslDigestMd5::authzid() const
{
    return m_authzid;
}

void QXmppSaslDigestMd5::setAuthzid(const QByteArray &authzid)
{
    m_authzid = authzid;
}

QByteArray QXmppSaslDigestMd5::cnonce() const
{
    return m_cnonce;
}

void QXmppSaslDigestMd5::setCnonce(const QByteArray &cnonce)
{
    m_cnonce = cnonce;
}

QByteArray QXmppSaslDigestMd5::digestUri() const
{
    return m_digestUri;
}

void QXmppSaslDigestMd5::setDigestUri(const QByteArray &digestUri)
{
    m_digestUri = digestUri;
}

QByteArray QXmppSaslDigestMd5::nc() const
{
    return m_nc;
}

void QXmppSaslDigestMd5::setNc(const QByteArray &nc)
{
    m_nc = nc;
}

QByteArray QXmppSaslDigestMd5::nonce() const
{
    return m_nonce;
}

void QXmppSaslDigestMd5::setNonce(const QByteArray &nonce)
{
    m_nonce = nonce;
}

QByteArray QXmppSaslDigestMd5::qop() const
{
    return m_qop;
}

void QXmppSaslDigestMd5::setQop(const QByteArray &qop)
{
    m_qop = qop;
}

QByteArray QXmppSaslDigestMd5::realm() const
{
    return m_realm;
}

void QXmppSaslDigestMd5::setRealm(const QByteArray &realm)
{
    m_realm = realm;
}

QByteArray QXmppSaslDigestMd5::username() const
{
    return m_username;
}

void QXmppSaslDigestMd5::setUsername(const QByteArray &username)
{
    m_username = username;
}

void QXmppSaslDigestMd5::setPassword(const QByteArray &password)
{
    m_password = password;
}

QByteArray QXmppSaslDigestMd5::generateNonce()
{
    QByteArray nonce = generateRandomBytes(32);

    // The random data can the '=' char is not valid as it is a delimiter,
    // so to be safe, base64 the nonce
    return nonce.toBase64();
}

/// Calculate digest response for use with XMPP/SASL.
///
/// \param A2
///

QByteArray QXmppSaslDigestMd5::calculateDigest(const QByteArray &A2) const
{
    const QByteArray a1 = m_username + ':' + m_realm + ':' + m_password;
    QByteArray ha1 = QCryptographicHash::hash(a1, QCryptographicHash::Md5);
    ha1 += ':' + m_nonce + ':' + m_cnonce;

    if (!m_authzid.isEmpty())
        ha1 += ':' + m_authzid;

    return calculateDigest(ha1, A2);
}

/// Calculate generic digest response.
///
/// \param A1
/// \param A2
///

QByteArray QXmppSaslDigestMd5::calculateDigest(const QByteArray &A1, const QByteArray &A2) const
{
    QByteArray HA1 = QCryptographicHash::hash(A1, QCryptographicHash::Md5).toHex();
    QByteArray HA2 = QCryptographicHash::hash(A2, QCryptographicHash::Md5).toHex();
    QByteArray KD;
    if (m_qop == "auth" || m_qop == "auth-int")
        KD = HA1 + ':' + m_nonce + ':' + m_nc + ':' + m_cnonce + ':' + m_qop + ':' + HA2;
    else
        KD = HA1 + ':' + m_nonce + ':' + HA2;
    return QCryptographicHash::hash(KD, QCryptographicHash::Md5).toHex();
}

QMap<QByteArray, QByteArray> QXmppSaslDigestMd5::parseMessage(const QByteArray &ba)
{
    QMap<QByteArray, QByteArray> map;
    int startIndex = 0;
    int pos = 0;
    while ((pos = ba.indexOf("=", startIndex)) >= 0)
    {
        // key get name and skip equals
        const QByteArray key = ba.mid(startIndex, pos - startIndex).trimmed();
        pos++;

        // check whether string is quoted
        if (ba.at(pos) == '"')
        {
            // skip opening quote
            pos++;
            int endPos = ba.indexOf('"', pos);
            // skip quoted quotes
            while (endPos >= 0 && ba.at(endPos - 1) == '\\')
                endPos = ba.indexOf('"', endPos + 1);
            if (endPos < 0)
            {
                qWarning("Unfinished quoted string");
                return map;
            }
            // unquote
            QByteArray value = ba.mid(pos, endPos - pos);
            value.replace("\\\"", "\"");
            value.replace("\\\\", "\\"); 
            map[key] = value;
            // skip closing quote and comma
            startIndex = endPos + 2;
        } else {
            // non-quoted string
            int endPos = ba.indexOf(',', pos);
            if (endPos < 0)
                endPos = ba.size();
            map[key] = ba.mid(pos, endPos - pos);
            // skip comma
            startIndex = endPos + 1;
        }
    }
    return map;
}

QByteArray QXmppSaslDigestMd5::serializeMessage(const QMap<QByteArray, QByteArray> &map)
{
    QByteArray ba;
    foreach (const QByteArray &key, map.keys())
    {
        if (!ba.isEmpty())
            ba.append(',');
        ba.append(key + "=");
        QByteArray value = map[key];
        const char *separators = "()<>@,;:\\\"/[]?={} \t";
        bool quote = false;
        for (const char *c = separators; *c; c++)
        {
            if (value.contains(*c))
            {
                quote = true;
                break;
            }
        }
        if (quote)
        {
            value.replace("\\", "\\\\");
            value.replace("\"", "\\\"");
            ba.append("\"" + value + "\"");
        }
        else
            ba.append(value);
    }
    return ba;
}

