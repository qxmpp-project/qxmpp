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


#ifndef CAPABILITIESCACHE_H
#define CAPABILITIESCACHE_H

#include <QObject>
#include <QDomElement>
#include <QMap>

class QXmppClient;

#include "QXmppDiscoveryIq.h"

class capabilitiesCache : public QObject
{
    Q_OBJECT

public:
    capabilitiesCache(QXmppClient* client);
    bool isCapabilityAvailable(const QString& nodeVer);
    void requestInfo(const QString& jid, const QString& nodeVer);

    void loadFromFile();

    QStringList getFeatures(const QString& nodeVer);
    QStringList getIdentities(const QString& nodeVer);

signals:

private slots:
    void infoReceived(const QXmppDiscoveryIq&);

private:
    void saveToFile(const QString& nodeVer);

    QXmppClient* m_client;

    QMap<QString, QXmppDiscoveryIq> m_mapCapabilities;
    QMap<QString, QString> m_mapIdNodeVer;
};

#endif // CAPABILITIESCACHE_H
