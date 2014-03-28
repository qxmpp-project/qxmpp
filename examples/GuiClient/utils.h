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


#ifndef CLIENTUTILS_H
#define CLIENTUTILS_H

#include "QXmppPresence.h"

#include <QCryptographicHash>
#include <QBuffer>
#include <QImageReader>

int comparisonWeightsPresenceStatusType(QXmppPresence::AvailableStatusType);
int comparisonWeightsPresenceType(QXmppPresence::Type);

QString presenceToStatusText(const QXmppPresence& presence);

QString getSettingsDir(const QString& bareJid = "");

QString getSha1HashAsHex(const QByteArray& image);
QImage getImageFromByteArray(const QByteArray& image);
QString getImageType1(const QByteArray& image);

bool isValidBareJid(const QString& bareJid);

QByteArray calculateXor(const QByteArray& data, const QByteArray& key);

#endif // CLIENTUTILS_H
