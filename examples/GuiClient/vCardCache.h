/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#ifndef VCARDMANAGER_H
#define VCARDMANAGER_H

#include <QObject>
#include <QMap>
#include <QImage>
#include "QXmppVCardIq.h"

// use sqlite

class QXmppClient;

class vCardCache : public QObject
{
    Q_OBJECT

public:
    vCardCache(QXmppClient* client);
    void requestVCard(const QString& bareJid);
    bool isVCardAvailable(const QString& bareJid);
    QImage getAvatar(const QString& bareJid) const;
    QXmppVCardIq& getVCard(const QString& bareJid);
    void loadAllFromCache();
    void saveToCache(const QString& bareJid);
    QString getSelfFullName();

signals:
    void vCardReadyToUse(const QString& bareJid);

public slots:
    void vCardReceived(const QXmppVCardIq&);

private:
    QString m_selfFullName;
    QXmppClient* m_client;

    QMap<QString, QXmppVCardIq> m_mapBareJidVcard;
};

#endif // VCARDMANAGER_H
