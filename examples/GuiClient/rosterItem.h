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


#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QStandardItem>
#include <QItemDelegate>
#include <QPainter>
#include "QXmppPresence.h"

class rosterItem : public QStandardItem
{
public:
    enum userRoles
    {
        StatusText = Qt::UserRole + 2,
        StatusType,
        PresenceType,
        BareJid,
        Avatar
    };

    rosterItem(const QString& bareJid);

    void setName(const QString& name);
    QString getName();
    void setBareJid(const QString& bareJid);
    void setStatusText(const QString& text);
    void setStatusType(QXmppPresence::Status::Type type);
    void setPresenceType(QXmppPresence::Type type);
    void setAvatar(const QImage& image);
    QImage getAvatar();
    QString getBareJid();
    QString getStatusText();
    QXmppPresence::Status::Type getStatusType();
    QXmppPresence::Type getPresenceType();
};

class rosterItemDelegate : public QItemDelegate
{
public:
    rosterItemDelegate();
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // ROSTERITEM_H
