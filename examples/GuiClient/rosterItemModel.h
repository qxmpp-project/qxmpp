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


#ifndef ROSTERITEMMODEL_H
#define ROSTERITEMMODEL_H

#include <QStandardItemModel>
#include "rosterItem.h"
#include "QXmppRosterManager.h"
#include "QXmppPresence.h"

class rosterItemModel : public QStandardItemModel
{
public:
    rosterItemModel(QObject* parent);
    rosterItem* getRosterItemFromBareJid(const QString& bareJid);

    void updatePresence(const QString& bareJid, const QMap<QString, QXmppPresence>& presences);
    void updateRosterEntry(const QString& bareJid, const QXmppRosterIq::Item& rosterEntry);
    void updateAvatar(const QString& bareJid, const QImage& image);

    void clear();
private:
    QMap<QString, rosterItem*> m_jidRosterItemMap;
    void addRosterItemIfDontExist(const QString& bareJid);
};

#endif // ROSTERITEMMODEL_H
