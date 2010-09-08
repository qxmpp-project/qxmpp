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

void rosterItemModel::addRosterItemIfDontExist(const QString& bareJid)
{
    if(!m_jidRosterItemMap.contains(bareJid))
    {
        rosterItem* item = new rosterItem(bareJid);
        m_jidRosterItemMap[bareJid] = item;
        appendRow(item);
        item->setStatusText("Offline");
        item->setBareJid(bareJid);
    }
}

void rosterItemModel::updatePresence(const QString& bareJid, const QMap<QString, QXmppPresence>& presences)
{
    addRosterItemIfDontExist(bareJid);

    if(presences.count() > 0)
    {
        QString statusText = presences.begin().value().status().statusText();
        QXmppPresence::Status::Type statusType = presences.begin().value().status().type();
        QXmppPresence::Type presenceType = presences.begin().value().type();

        if(statusText.isEmpty())
        {
            if(presenceType == QXmppPresence::Available)
                statusText = "Available";
            else if(presenceType == QXmppPresence::Unavailable)
                statusText = "Offline";
        }
        getRosterItemFromBareJid(bareJid)->setStatusText(statusText);
        getRosterItemFromBareJid(bareJid)->setStatusType(statusType);
        getRosterItemFromBareJid(bareJid)->setPresenceType(presenceType);
    }
}

void rosterItemModel::updateRosterEntry(const QString& bareJid, const QXmppRosterIq::Item& rosterEntry)
{
    addRosterItemIfDontExist(bareJid);

    QString name = rosterEntry.name();
    if(name.isEmpty())
    {
        name = bareJid;
    }

    if(getRosterItemFromBareJid(bareJid))
        getRosterItemFromBareJid(bareJid)->setName(name);
}

void rosterItemModel::updateAvatar(const QString& bareJid, const QImage& image)
{
    addRosterItemIfDontExist(bareJid);

    if(image.isNull())
        return;

    getRosterItemFromBareJid(bareJid)->setAvatar(image);
}

void rosterItemModel::clear()
{
    QStandardItemModel::clear();
    m_jidRosterItemMap.clear();
}
