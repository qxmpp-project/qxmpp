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


#include "avatarWidget.h"
#include <QtGui/QPainter>

avatarWidget::avatarWidget(QWidget* parent)
    : QPushButton(parent)
{
}

void avatarWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QRect r = rect();

    QPixmap pixmap = icon().pixmap(sizeHint(), QIcon::Normal, QIcon::On);
    if(pixmap.isNull())
        pixmap = QPixmap(":/icons/resource/avatar.png");
    QRect pixRect(0, 0, 32, 32);
    pixRect.moveCenter(r.center());
    painter.drawPixmap(pixRect, pixmap);

    if(underMouse() && !isDown())
    {
        painter.drawRect(pixRect.adjusted(0, 0, -1, -1));
        QColor col(Qt::white);
        col.setAlpha(80);
        painter.fillRect(pixRect.adjusted(0, 0, -1, -1), col);
    }
    if(isDown())
    {
        QColor col(Qt::white);
        col.setAlpha(50);
        painter.drawRect(pixRect.adjusted(1, 1, -2, -2));
    }
}

QSize avatarWidget::sizeHint() const
{
    return QSize(32, 32);
}
