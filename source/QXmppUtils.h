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


#ifndef QXMPPUTILS_H
#define QXMPPUTILS_H


// forward declarations of QXmlStream* classes will not work on Mac, we need to
// include the whole header.
// See http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
// for an explanation.
#include <QXmlStreamWriter>

class QByteArray;
class QDateTime;
class QString;
class QImage;

// XEP-0082: XMPP Date and Time Profiles
QDateTime datetimeFromString(const QString &str);
QString datetimeToString(const QDateTime &dt);

QString jidToResource(const QString& jid);
QString jidToBareJid(const QString& jid);
QString generateStanzaHash();

void helperToXmlAddAttribute(QXmlStreamWriter* stream, const QString& name,
                             const QString& value);
void helperToXmlAddTextElement(QXmlStreamWriter* stream, const QString& name,
                           const QString& value);
void helperToXmlAddNumberElement(QXmlStreamWriter* stream, const QString& name,
                           int value);

void log(const QString& str);
void log(const QByteArray& str);

QString escapeString(const QString& str);
QString unescapeString(const QString& str);

QString getImageType(const QByteArray& image);
QString getImageHash(const QByteArray& image);
QImage getImageFromByteArray(const QByteArray& image);

#endif // QXMPPUTILS_H
