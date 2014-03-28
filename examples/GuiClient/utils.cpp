/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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
#include <QDir>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

int comparisonWeightsPresenceStatusType(QXmppPresence::AvailableStatusType statusType)
{
    switch(statusType)
    {
        case QXmppPresence::Online:
        case QXmppPresence::Chat:
            return 0;
        case QXmppPresence::DND:
            return 1;
        case QXmppPresence::Away:
        case QXmppPresence::XA:
            return 2;
        case QXmppPresence::Invisible:
            return 3;
        default:
            return 5;
    }
}

int comparisonWeightsPresenceType(QXmppPresence::Type type)
{
    switch(type)
    {
        case QXmppPresence::Available:
            return 0;
        case QXmppPresence::Unavailable:
            return 1;
        case QXmppPresence::Error:
        case QXmppPresence::Subscribe:
        case QXmppPresence::Subscribed:
        case QXmppPresence::Unsubscribe:
        case QXmppPresence::Unsubscribed:
        case QXmppPresence::Probe:
            return 3;
        default:
            return 5;
    }
}

QString presenceToStatusText(const QXmppPresence& presence)
{
    QString statusText = presence.statusText();
    if(statusText.isEmpty())
    {
        if(presence.type() == QXmppPresence::Available)
        {
            switch(presence.availableStatusType())
            {
            case QXmppPresence::Invisible:
                statusText = "Offline";
                break;
            case QXmppPresence::Online:
            case QXmppPresence::Chat:
                statusText = "Available";
                break;
            case QXmppPresence::Away:
            case QXmppPresence::XA:
                statusText = "Idle";
                break;
            case QXmppPresence::DND:
                statusText = "Busy";
                break;
            }
        }
        else
            statusText = "Offline";
    }

    return statusText;
}

QString getSettingsDir(const QString& bareJid)
{
#if QT_VERSION >= 0x050000
    QStringList dirList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    QString dir = dirList.size() > 0 ? dirList.at(0) : "";
#else
    QString dir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    if(bareJid.isEmpty())
        return dir + "/";
    else
        return QString(dir + "/%1/").arg(bareJid);
}

QString getSha1HashAsHex(const QByteArray& image)
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

QString getImageType1(const QByteArray& image)
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

bool isValidBareJid(const QString& bareJid)
{
    QRegExp re("^[^@]+@[^@]+$");
    return re.exactMatch(bareJid);
}

QByteArray calculateXor(const QByteArray& data, const QByteArray& key)
{
    if(key.isEmpty())
        return data;

    QByteArray result;
    for(int i = 0 , j = 0; i < data.length(); ++i , ++j)
    {
        if(j == key.length())
            j = 0;
        result.append(data.at(i) ^ key.at(j));
    }
    return result;
}
