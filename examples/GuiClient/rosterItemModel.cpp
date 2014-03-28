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


#include "rosterItemModel.h"

rosterItemModel::rosterItemModel(QObject* parent) : QStandardItemModel(parent)
{
//    addRosterItemIfDontExist("jkhjkhkhkhk");
//    addRosterItemIfDontExist("uuuu");
//    addRosterItemIfDontExist("kkkkkkk");
//    addRosterItemIfDontExist("jjjjjjjj");
}

rosterItem* rosterItemModel::getRosterItemFromBareJid(const QString& bareJid)
{
    if(m_jidRosterItemMap.contains(bareJid))
        return m_jidRosterItemMap[bareJid];
    else
        return 0;
}

rosterItem* rosterItemModel::getOrCreateItem(const QString& bareJid)
{
    if(m_jidRosterItemMap.contains(bareJid)) {
        return m_jidRosterItemMap[bareJid];
    } else {
        rosterItem* item = new rosterItem(bareJid);
        m_jidRosterItemMap[bareJid] = item;
        appendRow(item);
        return item;
    }
}

void rosterItemModel::updatePresence(const QString& bareJid, const QMap<QString, QXmppPresence>& presences)
{
    rosterItem *item = getOrCreateItem(bareJid);
    if (!presences.isEmpty())
        item->setPresence(*presences.begin());
    else
        item->setPresence(QXmppPresence(QXmppPresence::Unavailable));
}

void rosterItemModel::updateRosterEntry(const QString& bareJid, const QXmppRosterIq::Item& rosterEntry)
{
    getOrCreateItem(bareJid)->setName(rosterEntry.name());
}

void rosterItemModel::updateAvatar(const QString& bareJid, const QImage& image)
{
    getOrCreateItem(bareJid)->setAvatar(image);
}

void rosterItemModel::updateName(const QString& bareJid, const QString& name)
{
    if (!name.isEmpty())
        getOrCreateItem(bareJid)->setName(name);
}

void rosterItemModel::clear()
{
    QStandardItemModel::clear();
    m_jidRosterItemMap.clear();
}

void rosterItemModel::removeRosterEntry(const QString& bareJid)
{
    rosterItem* item = getRosterItemFromBareJid(bareJid);
    if(item)
    {
        removeRow(item->row());
    }
}
