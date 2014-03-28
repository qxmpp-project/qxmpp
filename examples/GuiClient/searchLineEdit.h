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


#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QPushButton>
#include <QLineEdit>
#include <QPainter>

class searchClearButton : public QPushButton
{
    Q_OBJECT

public:
    searchClearButton(QWidget *w)
        : QPushButton(w)
    {
        setMinimumSize(24, 24);
        setFixedSize(24, 24);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        int height = parentWidget()->geometry().height();
        int width = height; //parentWidget()->geometry().width();

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::white);

        float penwidth = isDown() ? 1.2 :
                         underMouse() ? 1.6 : 1.2;
        painter.setBrush(Qt::red);
        //painter.drawEllipse(4, 4, width - 8, height - 8);
        QPen pen;
        pen.setWidthF(penwidth);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        int border = 7;
        painter.drawLine(border, border, width - border, height - border);
        painter.drawLine(border, height - border, width - border, border);
    }
};

class searchLineEdit : public QLineEdit
{
public:
    searchLineEdit(QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);

private:
    QPushButton *clearButton;
};

#endif // SEARCHLINEEDIT_H
