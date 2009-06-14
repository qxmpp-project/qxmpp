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


#ifndef UTILS_H
#define UTILS_H


class QTextStream;
class QByteArray;
class QString;

QString jidToResource(const QString& jid);
QString jidToBareJid(const QString& jid);

void helperToXmlAddAttribute(QTextStream& stream, const QString& name, const QString& value);
void helperToXmlAddElement(QTextStream& stream, const QString& name, const QString& value);
void helperToXmlAddElement(QTextStream& stream, const QString& name, int value);

void log(const QString& str);
void log(const QByteArray& str);

#endif // UTILS_H
