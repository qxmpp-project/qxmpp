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


#ifndef VCARDCACHE_H
#define VCARDCACHE_H

#include <QObject>
#include <QMap>
#include "QXmppVCardIq.h"

class QImage;
class QXmppClient;

class vCardCache : public QObject
{
    Q_OBJECT

public:
    vCardCache(QXmppClient* client);

    bool isVCardAvailable(const QString& bareJid) const;
    void requestVCard(const QString& bareJid);
    QXmppVCardIq& getVCard(const QString& bareJid);
    QImage getAvatar(const QString& bareJid) const;

    void loadFromFile();

    QByteArray getPhotoHash(const QString& bareJid) const;

signals:
    void vCardReadyToUse(const QString& bareJid);

public slots:
    void vCardReceived(const QXmppVCardIq&);

private:
    void saveToFile(const QString& bareJid);

    QXmppClient* m_client;
    QMap<QString, QXmppVCardIq> m_mapBareJidVcard;
};

#endif // VCARDCACHE_H
