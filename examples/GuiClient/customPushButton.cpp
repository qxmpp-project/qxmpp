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


#include "customPushButton.h"
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionFrameV2>
#include <QFontMetrics>

customPushButton::customPushButton(QWidget* parent)
    : QPushButton(parent)
{
    setMinimumSize(QSize(20, 18));
}

void customPushButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);

    QFont font;
    painter.setFont(font);
    QStyleOptionButton panel;
    initStyleOption(&panel);
    QRect r = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &panel, this);
    painter.setPen(Qt::gray);

    QRect rectSize(0, 0, sizeHint().width(), sizeHint().height());
    rectSize.moveCenter(r.center());
    r = rectSize;
    r.adjust(0, 0, -1, -1);
    if(underMouse() && !isDown())
    {
        QRectF rr = r;
        painter.drawRoundedRect(r, 3, 3);
        QColor col(Qt::white);
        col.setAlpha(80);
        rr.adjust(1, 1, -1, -1);
        painter.fillRect(rr, col);
    }
    if(isDown())
    {
        QRectF rr = r;
//        rr.adjust(1, 1, -1, -1);
        painter.drawRoundedRect(rr, 3, 3);
        QColor col(Qt::white);
        col.setAlpha(80);
        rr.adjust(1, 1, -1, -1);
        painter.fillRect(rr, col);
    }
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    r.moveLeft(r.left() + 3);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(r, Qt::AlignVCenter|Qt::TextSingleLine, text());

    QImage image(":/icons/resource/downArrow.png");
    QRect rectDelta(0, 0, 7, 4);
    rectDelta.moveRight(r.right() - 4);
    rectDelta.moveCenter(QPoint(rectDelta.center().x(), r.center().y()));
    painter.drawImage(rectDelta, image);
}

QSize customPushButton::sizeHint() const
{
    QFont font;
    font.setBold(true);
    QFontMetrics fm(font);
    int width = fm.width(text());
    if(width <= (160 - 8 - 9))
        return QSize(width+8+9, 18);
    else
        return QSize(160, 18);
}
