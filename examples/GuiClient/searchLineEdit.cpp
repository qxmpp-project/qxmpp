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


#include "searchLineEdit.h"

searchLineEdit::searchLineEdit(QWidget* parent):QLineEdit(parent)
{
    setMinimumSize(QSize(20, 24));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setStyleSheet(":enabled { padding-right: 20px; padding-left: 20px }");
    clearButton = new searchClearButton(this);
    clearButton->setVisible(true);

    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setToolTip("Clear");
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
}

void searchLineEdit::paintEvent(QPaintEvent *e) {
    QLineEdit::paintEvent(e);
    QPainter painter(this);

    QImage image(":/icons/resource/searchIcon.png");

    QRectF target(image.rect());
    target.moveCenter(QPointF(target.center().x()+2, target.center().y()+3));
    painter.drawImage(target, image, image.rect());

    if (text().length() == 0 && (!hasFocus()) )
    {
        painter.setPen(Qt::gray);
        QRect r = rect();
        painter.drawText(24, r.height()/2+4, "Search Contacts");
    }

    if(text().isEmpty())
        clearButton->setVisible(false);
    else
        clearButton->setVisible(true);
}

void searchLineEdit::resizeEvent(QResizeEvent*)
{
    clearButton->setParent(this);
    clearButton->setGeometry(QRect(width()-23,
                                   0,
                                   24, 24));
}

void searchLineEdit::moveEvent(QMoveEvent*)
{
    clearButton->setParent(this);
    clearButton->setGeometry(QRect(width()-23, 1,
                                   24, 24));
}
