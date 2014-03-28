/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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


#ifndef QXMPPUTILS_H
#define QXMPPUTILS_H

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

#include "QXmppGlobal.h"

class QByteArray;
class QDateTime;
class QDomElement;
class QString;
class QStringList;

/// \brief The QXmppUtils class contains static utility functions.
///
class QXMPP_EXPORT QXmppUtils
{
public:
    // XEP-0082: XMPP Date and Time Profiles
    static QDateTime datetimeFromString(const QString &str);
    static QString datetimeToString(const QDateTime &dt);
    static int timezoneOffsetFromString(const QString &str);
    static QString timezoneOffsetToString(int secs);

    static QString jidToDomain(const QString& jid);
    static QString jidToResource(const QString& jid);
    static QString jidToUser(const QString& jid);
    static QString jidToBareJid(const QString& jid);

    static quint32 generateCrc32(const QByteArray &input);
    static QByteArray generateHmacMd5(const QByteArray &key, const QByteArray &text);
    static QByteArray generateHmacSha1(const QByteArray &key, const QByteArray &text);
    static int generateRandomInteger(int N);
    static QByteArray generateRandomBytes(int length);
    static QString generateStanzaHash(int length=32);
};

void helperToXmlAddAttribute(QXmlStreamWriter* stream, const QString& name,
                             const QString& value);
void helperToXmlAddTextElement(QXmlStreamWriter* stream, const QString& name,
                           const QString& value);

#endif // QXMPPUTILS_H
