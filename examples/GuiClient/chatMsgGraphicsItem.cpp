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


#include "chatMsgGraphicsItem.h"

#include <QPainter>
#include <QTextDocument>
#include <QTime>

QLinearGradient getGradient(const QColor &col, const QRectF &rect)
{
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());

    qreal hue = col.hueF();
    qreal value = col.valueF();
    qreal saturation = col.saturationF();

    QColor c = col;
    c.setHsvF(hue, 0.42 * saturation, 0.98 * value);
    g.setColorAt(0, c);
    c.setHsvF(hue, 0.58 * saturation, 0.95 * value);
    g.setColorAt(0.25, c);
    c.setHsvF(hue, 0.70 * saturation, 0.93 * value);
    g.setColorAt(0.5, c);

    c.setHsvF(hue, 0.95 * saturation, 0.9 * value);
    g.setColorAt(0.501, c);
    c.setHsvF(hue * 0.95, 0.95 * saturation, 0.95 * value);
    g.setColorAt(0.75, c);
    c.setHsvF(hue * 0.90, 0.95 * saturation, 1 * value);
    g.setColorAt(1.0, c);

    return g;
}

QLinearGradient darken(const QLinearGradient &gradient)
{
    QGradientStops stops = gradient.stops();
    for (int i = 0; i < stops.size(); ++i) {
        QColor color = stops.at(i).second;
        stops[i].second = color.darker(160);
    }

    QLinearGradient g = gradient;
    g.setStops(stops);
    return g;
}

static void drawPath(QPainter *p, const QPainterPath &path, const QColor &col, bool dark = false)
{
    const QRectF pathRect = path.boundingRect();

    const QLinearGradient baseGradient = getGradient(col, pathRect);
    const QLinearGradient darkGradient = darken(baseGradient);

    p->save();

   // p->setOpacity(0.25);

    //glow
//    if (dark)
//        p->strokePath(path, QPen(darkGradient, 6));
//    else
//        p->strokePath(path, QPen(baseGradient, 6));

    p->setOpacity(1.0);

    //fill
    if (dark)
        p->fillPath(path, darkGradient);
    else
        p->fillPath(path, baseGradient);

    QLinearGradient g(pathRect.topLeft(), pathRect.topRight());
    g.setCoordinateMode(QGradient::ObjectBoundingMode);

    p->setOpacity(0.2);
    p->fillPath(path, g);

    p->setOpacity(0.5);

    // highlight
//    if (dark)
//        p->strokePath(path, QPen(col.lighter(160).darker(160), 2));
//    else
//        p->strokePath(path, QPen(col.lighter(160), 2));

    p->setOpacity(1.0);

    p->restore();
}

chatMsgGraphicsItem::chatMsgGraphicsItem(QGraphicsItem * parent):QGraphicsPathItem(parent),
            m_spikeWidth(9),
            m_spikeHeight(6),
            m_cornerRadius(10),
            m_textSpacing(4), m_color(Qt::yellow)
{
    setPath(createPath());
//    setFlags(QGraphicsItem::ItemIsMovable);

    QFont font;
    QFontMetrics fm(font);
    m_timeStampWidth = fm.width(getTime()) + 4;
}

void chatMsgGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    drawPath(painter, path(), m_color);

//    int spike_x = m_spikeWidth;
//    int spike_y = m_spikeHeight;
//    int corner = m_cornerRadius;
//    int length = m_width - spike_x;
//    int offset = spike_x;
    QFont font;
    font.setBold(true);
    QTextDocument textDoc(getText());
    QTextOption textOp;
    textOp.setWrapMode(QTextOption::WrapAnywhere);
    textOp.setAlignment(Qt::AlignLeft);
    textDoc.setDefaultTextOption(textOp);
    textDoc.setTextWidth(getTextWidth());
    textDoc.setDefaultFont(font);

    painter->setPen(Qt::white);
    painter->setFont(font);
    int height = (int) textDoc.size().height();
    painter->drawText(m_spikeWidth + m_cornerRadius, 4, getTextWidth(), height,
                      Qt::AlignLeft|Qt::TextWrapAnywhere, getText());

//    painter->setPen(Qt::gray);
    painter->setPen(Qt::black);

//    font.setBold(false);
    painter->setFont(font);
    painter->drawText(-m_boxStartLength, 0, m_boxStartLength, m_height,
                      Qt::AlignRight|Qt::AlignBottom, getName());

    font.setBold(false);
    painter->setPen(Qt::gray);
    painter->setFont(font);

    int timeWidth;
    if(m_timeStampWidth > m_boxStartLength)
        timeWidth = m_timeStampWidth;
    else
        timeWidth = m_boxStartLength;

    painter->drawText(getMaxWidth() + 6, 0, timeWidth - 6, m_height,
                      Qt::AlignBottom|Qt::AlignLeft, getTime());
}

void chatMsgGraphicsItem::setText(const QString& text)
{
    m_text = text;
    calculateWidth();
    setPath(createPath());
}

void chatMsgGraphicsItem::setMaxWidth(int width)
{
    m_maxWidth = width;
    setPath(createPath());
}

void chatMsgGraphicsItem::setViewWidth(int width)
{
    //25 for scrollbar
    setMaxWidth(width - getBoxStartLength() - 25);
}

int chatMsgGraphicsItem::getMaxWidth() const
{
    return m_maxWidth;
}

void chatMsgGraphicsItem::setAlignment(Alignment align)
{
    m_alignment = align;
    setPath(createPath());
}

QPainterPath chatMsgGraphicsItem::createPath()
{
    calculateWidth();
    int spike_x = m_spikeWidth;
    int spike_y = m_spikeHeight;
    int corner = m_cornerRadius;
    int length = m_width - spike_x;
    int offset = spike_x;

    QPainterPath messageBoxPath;
    messageBoxPath.moveTo(0 + offset, m_height - corner);
    QRectF rect(offset - 2*spike_x, m_height - corner - spike_y, 2*spike_x, 2*spike_y);
    messageBoxPath.arcMoveTo(rect, -90.0);
    messageBoxPath.arcTo(rect, 270, 90.0);
    messageBoxPath.lineTo(0 + offset, corner);
    messageBoxPath.arcTo(0 + offset, 0, 2*corner, 2*corner, 180, -90.0);
    messageBoxPath.lineTo(length - corner, 0);
    messageBoxPath.arcTo(length + offset - corner*2, 0, 2*corner, 2*corner, 90, -90.0);
    messageBoxPath.lineTo(length + offset, m_height - corner);
    messageBoxPath.arcTo(length + offset - corner*2, m_height - 2*corner, 2*corner, 2*corner, 0, -90.0);
    messageBoxPath.lineTo(offset + corner, m_height);
    messageBoxPath.arcTo(offset, m_height - 2*corner, 2*corner, 2*corner, 270, -45.0);
    messageBoxPath.closeSubpath();

    return messageBoxPath;
}

QString chatMsgGraphicsItem::getText() const
{
    return m_text;
}

int chatMsgGraphicsItem::getTextWidth() const
{
    return getMaxWidth() - m_spikeWidth - m_cornerRadius*2;
}

void chatMsgGraphicsItem::calculateWidth()
{
    QFont font;
    font.setBold(true);
    QTextDocument textDoc(m_text);
    textDoc.setDefaultFont(font);
    int idealWidth = (int)textDoc.size().width();
    textDoc.setTextWidth(getTextWidth());
    m_height = (int)textDoc.size().height();

    if(idealWidth < getTextWidth())
    {
        m_width = idealWidth + m_spikeWidth + m_cornerRadius;
    }
    else
        m_width = getMaxWidth();
}

void chatMsgGraphicsItem::setName(const QString& name)
{
    m_name = name;
    if(name != "Me")
        m_color = QColor(0, 210, 250);
    else
        m_color = QColor(250, 188, 239);
}

QString chatMsgGraphicsItem::getName() const
{
    return m_name;
}

QString chatMsgGraphicsItem::getTime() const
{
    return QTime::currentTime().toString("hh:mm");
}

void chatMsgGraphicsItem::setBoxStartLength(int length)
{
    m_boxStartLength = length;
}

int chatMsgGraphicsItem::getBoxStartLength() const
{
    return m_boxStartLength;
}

void chatMsgGraphicsItem::setColor(const QColor& color)
{
    m_color = color;
}

QRectF chatMsgGraphicsItem::boundingRect() const
{
    QRectF rect = QGraphicsPathItem::boundingRect();
    rect.setLeft(-getBoxStartLength());

    int timeWidth;
    if(m_timeStampWidth > m_boxStartLength)
        timeWidth = m_timeStampWidth;
    else
        timeWidth = m_boxStartLength;
    rect.setRight(getMaxWidth() + timeWidth);
    return rect;
}
