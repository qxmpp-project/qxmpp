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


#include "QXmppUtils.h"
#include "QXmppLogger.h"
#include <QString>
#include <QXmlStreamWriter>
#include <QByteArray>
#include <QBuffer>
#include <QImageReader>
#include <QCryptographicHash>
#include <QDateTime>

QString jidToResource(const QString& jid)
{
    return jid.mid(jid.indexOf(QChar('/'))+1);
}

QString jidToBareJid(const QString& jid)
{
    return jid.left(jid.indexOf(QChar('/')));
}

QString generateStanzaHash()
{

    QString somechars = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString hashResult;
    for ( int idx = 0; idx < 10; ++idx )
    {
        hashResult += somechars[(qrand() % 61)];
    }
    return hashResult;
}

void helperToXmlAddAttribute(QXmlStreamWriter* stream, const QString& name,
                             const QString& value)
{
    if(!value.isEmpty())
        stream->writeAttribute(name,value);
}

void helperToXmlAddNumberElement(QXmlStreamWriter* stream, const QString& name, int value)
{
    stream->writeTextElement( name, QString::number(value));
}

void helperToXmlAddTextElement(QXmlStreamWriter* stream, const QString& name,
                           const QString& value)
{
    if(!value.isEmpty())
        stream->writeTextElement( name, value);
    else
        stream->writeEmptyElement(name);
}

void log(const QString& str)
{
    QXmppLogger::getLogger()->log(str);
}

void log(const QByteArray& str)
{
    QXmppLogger::getLogger()->log(str);
}

QString escapeString(const QString& str)
{
    QString strOut = str;
    strOut.replace(QChar('&'), "&amp;");
    strOut.replace(QChar('<'), "&lt;");
    strOut.replace(QChar('>'), "&gt;");
    strOut.replace(QChar('"'), "&quot;");
    return strOut;
}

QString unescapeString(const QString& str)
{
    QString strOut = str;
    strOut.replace("&lt;", QChar('<'));
    strOut.replace("&gt;", QChar('>'));
    strOut.replace("&quot;", QChar('"'));
    strOut.replace("&amp;", QChar('&'));
    return strOut;
}

QString getImageType(const QByteArray& image)
{
    QBuffer buffer;
    buffer.setData(image);
    buffer.open(QIODevice::ReadOnly);
    QString format = QImageReader::imageFormat(&buffer);

    if(format.toUpper() == "PNG")
        return "image/png";
    else if(format.toUpper() == "MNG")
        return "video/x-mng";
    else if(format.toUpper() == "GIF")
        return "image/gif";
    else if(format.toUpper() == "BMP")
        return "image/bmp";
    else if(format.toUpper() == "XPM")
        return "image/x-xpm";
    else if(format.toUpper() == "SVG")
        return "image/svg+xml";
    else if(format.toUpper() == "JPEG")
        return "image/jpeg";

    return "image/unknown";
}

QString getImageHash(const QByteArray& image)
{
    if(image.isEmpty())
        return "";
    else
        return QString(QCryptographicHash::hash(image,
            QCryptographicHash::Sha1).toHex());
}

QImage getImageFromByteArray(const QByteArray& image)
{
    QBuffer buffer;
    buffer.setData(image);
    buffer.open(QIODevice::ReadOnly);
    QImageReader imageReader(&buffer);
    return imageReader.read();
}
