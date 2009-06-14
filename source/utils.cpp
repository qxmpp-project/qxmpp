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


#include "utils.h"
#include "QXmppLogger.h"
#include <QString>
#include <QTextStream>
#include <QByteArray>

QString jidToResource(const QString& jid)
{
    return jid.mid(jid.indexOf(QChar('/'))+1);
}

QString jidToBareJid(const QString& jid)
{
    return jid.left(jid.indexOf(QChar('/')));
}

void helperToXmlAddAttribute(QTextStream& stream, const QString& name, const QString& value)
{
    if(!value.isEmpty())
        stream << " " << name <<"='" << value << "'";
}

void helperToXmlAddElement(QTextStream& stream, const QString& name, int value)
{
    stream << "<" << name << ">" << value << "</" << name << ">";
}

void helperToXmlAddElement(QTextStream& stream, const QString& name, const QString& value)
{
    if(!value.isEmpty())
        stream << "<" << name << ">" << value << "</" << name << ">";
}

void log(const QString& str)
{
    QXmppLogger::getLogger()->log(str);
}

void log(const QByteArray& str)
{
    QXmppLogger::getLogger()->log(str);
}
