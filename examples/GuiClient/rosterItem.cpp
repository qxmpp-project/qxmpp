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


#include "rosterItem.h"
#include <QImage>

rosterItem::rosterItem(const QString& bareJid) //: QStandardItem(bareJid)
{
    setStatusType(QXmppPresence::Status::Offline);
    setStatusText("Offline");
}

void rosterItem::setName(const QString& name)
{
    setText(name);
}

QString rosterItem::getName()
{
    return text();
}

void rosterItem::setBareJid(const QString& bareJid)
{
    setData(bareJid, rosterItem::BareJid);
}

void rosterItem::setStatusText(const QString& text)
{
    setData(text, rosterItem::StatusText);
}

QString rosterItem::getBareJid()
{
    return data(rosterItem::BareJid).toString();
}

QString rosterItem::getStatusText()
{
    return data(rosterItem::StatusText).toString();
}

void rosterItem::setStatusType(QXmppPresence::Status::Type type)
{
    setData(static_cast<int>(type), StatusType);
    QString icon;
    switch(type)
    {
    case QXmppPresence::Status::Online:
    case QXmppPresence::Status::Chat:
        icon = "green";
        break;
    case QXmppPresence::Status::Away:
    case QXmppPresence::Status::XA:
        icon = "orange";
        break;
    case QXmppPresence::Status::DND:
        icon = "red";
        break;
    case QXmppPresence::Status::Invisible:
    case QXmppPresence::Status::Offline:
        icon = "gray";
        break;
    }
    if(!icon.isEmpty())
        setIcon(QIcon(":/icons/resource/"+icon+".png"));
}

QXmppPresence::Status::Type rosterItem::getStatusType()
{
    return static_cast<QXmppPresence::Status::Type>(data(StatusType).toInt());
}

void rosterItem::setPresenceType(QXmppPresence::Type type)
{
    setData(static_cast<int>(type), PresenceType);
    QString icon;
    switch(type)
    {
    case QXmppPresence::Available:
        break;
    case QXmppPresence::Unavailable:
        icon = "gray";
        break;
    case QXmppPresence::Error:
    case QXmppPresence::Subscribe:
    case QXmppPresence::Subscribed:
    case QXmppPresence::Unsubscribe:
    case QXmppPresence::Unsubscribed:
    case QXmppPresence::Probe:
        break;
    }
    if(!icon.isEmpty())
        setIcon(QIcon(":/icons/resource/"+icon+".png"));
}

QXmppPresence::Type rosterItem::getPresenceType()
{
    return static_cast<QXmppPresence::Type>(data(PresenceType).toInt());
}

void rosterItem::setAvatar(const QImage& image)
{
    setData(QVariant(image), rosterItem::Avatar);
}

QImage rosterItem::getAvatar()
{
    return qvariant_cast<QImage>(data(rosterItem::Avatar));
}

ItemDelegate::ItemDelegate()
{
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex & index) const
{
    return QSize(44, 36);
}

void ItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QVariant value = index.data(Qt::DecorationRole);

    QColor selectedBg(60, 140, 222);
    QColor alternateBg(239, 245, 254);
    QColor selectedText(Qt::white);

    QColor nameTextColor(Qt::black);
    QColor statusTextColor(Qt::darkGray);

    QPixmap pixmap;
    if(value.type() == QVariant::Icon)
    {
        QIcon icon = qvariant_cast<QIcon>(value);
        pixmap = icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On);
    }

    QPen penDivision;
//        if(index.row() % 2)
//            painter->fillRect(option.rect, alternateBg);

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, selectedBg);
//            painter->fillRect(option.rect, option.palette.highlight());
//            penDivision.setColor(option.palette.highlight().color());
        penDivision.setColor(selectedBg);
        nameTextColor = selectedText;
        statusTextColor = selectedText;
    }
    else
    {
        penDivision.setColor(QColor(244, 244, 244));
    }

    QRect rect = option.rect;
    rect.setWidth(pixmap.width());
    rect.setHeight(pixmap.height());
    rect.moveTop(rect.y() + (option.rect.height() - pixmap.height())/2);
    rect.moveLeft(rect.left() + 2);
    painter->drawPixmap(rect, pixmap);

    rect = option.rect;
    rect.setLeft(rect.x() + pixmap.width() + 8);
    rect.moveTop(rect.y() + 3);
    QFont font;
    painter->setFont(font);
    painter->setPen(nameTextColor);
    if(!index.data(Qt::DisplayRole).toString().isEmpty())
        painter->drawText(rect, index.data(Qt::DisplayRole).toString());
    else
        painter->drawText(rect, index.data(rosterItem::BareJid).toString());

    painter->setPen(statusTextColor);
    rect.setTop(rect.y() + rect.height()/2);
    rect.moveTop(rect.y() - 3);
    QString statusText = index.data(rosterItem::StatusText).toString();
    QFontMetrics fontMetrics(font);
    statusText = fontMetrics.elidedText(statusText, Qt::ElideRight, rect.width() - 34);
    painter->drawText(rect, statusText);

    penDivision.setWidth(0);
    painter->setPen(penDivision);

    rect = option.rect;
    QPoint left = rect.bottomLeft();
    left.setX(left.x() + 4);
    QPoint right = rect.bottomRight();
    right.setX(right.x() - 4);
    painter->drawLine(left, right);

    QImage image;
    value = index.data(rosterItem::Avatar);
    if(value.type() == QVariant::Image)
    {
        image = qvariant_cast<QImage>(value);
    }

    pixmap = QPixmap(":/icons/resource/avatar.png");
    rect = option.rect;
    rect.setWidth(pixmap.width());
    rect.setHeight(pixmap.height());
    rect.moveTop(rect.y() + (option.rect.height() - pixmap.height())/2);
    rect.moveLeft(option.rect.x() + option.rect.width() - pixmap.width() - 2);

//    if(image.isNull())
//        painter->drawPixmap(rect, pixmap);
//    else
        painter->drawImage(rect, image);

    painter->restore();
}
