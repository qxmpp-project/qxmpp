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


#ifndef QXMPPUTILS_H
#define QXMPPUTILS_H

// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

class QByteArray;
class QDateTime;
class QDomElement;
class QString;
class QStringList;

// XEP-0082: XMPP Date and Time Profiles
QDateTime datetimeFromString(const QString &str);
QString datetimeToString(const QDateTime &dt);
int timezoneOffsetFromString(const QString &str);
QString timezoneOffsetToString(int secs);

QString jidToDomain(const QString& jid);
QString jidToResource(const QString& jid);
QString jidToUser(const QString& jid);
QString jidToBareJid(const QString& jid);

quint32 generateCrc32(const QByteArray &input);
QByteArray generateHmacMd5(const QByteArray &key, const QByteArray &text);
QByteArray generateHmacSha1(const QByteArray &key, const QByteArray &text);
int generateRandomInteger(int N);
QByteArray generateRandomBytes(int length);
QString generateStanzaHash(int length=32);

void helperToXmlAddAttribute(QXmlStreamWriter* stream, const QString& name,
                             const QString& value);
void helperToXmlAddDomElement(QXmlStreamWriter* stream,
    const QDomElement& element, const QStringList &omitNamespaces);
void helperToXmlAddTextElement(QXmlStreamWriter* stream, const QString& name,
                           const QString& value);

#endif // QXMPPUTILS_H
