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


#ifndef CHATMSGGRAPHICSITEM_H
#define CHATMSGGRAPHICSITEM_H

#include <QGraphicsPathItem>

class chatMsgGraphicsItem : public QGraphicsPathItem
{
public:
    enum Alignment
    {
        LEFT = 0,
        RIGHT
    };

    chatMsgGraphicsItem(QGraphicsItem * parent = 0);
    void setText(const QString& text);
    void setName(const QString& name);
    QString getName() const;
    QString getText() const;

    void setMaxWidth(int width);
    int getMaxWidth() const;
    void setViewWidth(int viewWidth);

    void setAlignment(Alignment align);

    void setBoxStartLength(int length);
    int getBoxStartLength() const;

    void setColor(const QColor&);

    virtual QRectF boundingRect() const;

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath createPath();
    int getTextWidth() const;
    void calculateWidth();
    QString getTime() const;

    // max width of bubble including the spike
    int m_maxWidth;

    // actual width
    int m_width;

    // height of bubble
    int m_height;
    int m_spikeWidth;
    int m_spikeHeight;
    int m_cornerRadius;
    int m_textSpacing;
    int m_boxStartLength;
    int m_timeStampWidth;
    QColor m_color;

    QString m_text;
    QString m_name;
    int m_length;
    Alignment m_alignment;
};

#endif // CHATMSGGRAPHICSITEM_H
