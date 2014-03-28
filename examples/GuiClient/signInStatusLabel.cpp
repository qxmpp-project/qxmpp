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


#include "signInStatusLabel.h"
#include <QFontMetrics>

signInStatusLabel::signInStatusLabel(QWidget* parent):QLabel(parent), m_timer(this),
        m_option(None)
{
    m_timer.setSingleShot(false);

    bool check = connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

void signInStatusLabel::setCustomText(const QString& text, signInStatusLabel::Option op,
                   int countDown)
{
    m_text = text;
    m_option = op;
    m_countDown = countDown;
    switch(op)
    {
    case None:
        m_timer.stop();
        m_postfix = "";
        break;
    case WithProgressEllipsis:
//        m_timer.start(400);
        m_postfix = "";
        break;
    case CountDown:
        m_timer.start(1000);
        m_postfix = "";
        break;
    default:
        m_timer.stop();
        m_postfix = "";
        break;
    }

    if(m_option == CountDown)
        setText(m_text.arg(m_countDown) + m_postfix);
    else
        setText(m_text + m_postfix);

    updateGeometry();
}

void signInStatusLabel::timeout()
{
    switch(m_option)
    {
    case None:
        break;
    case WithProgressEllipsis:
        if(m_postfix == "")
            m_postfix = ".";
        else if(m_postfix == ".")
            m_postfix = "..";
        else if(m_postfix == "..")
            m_postfix = "...";
        else if(m_postfix == "...")
            m_postfix = "";
        break;
    case CountDown:
        if(m_countDown == 0)
            m_timer.stop();
        --m_countDown;
        break;
    default:
        break;
    }

    if(m_option == CountDown)
        setText(m_text.arg(m_countDown) + m_postfix);
    else
        setText(m_text + m_postfix);
    updateGeometry();
}

//QSize signInStatusLabel::sizeHint() const
//{
//    QFont font;
//    QFontMetrics fm(font);
//    return QSize(fm.width(m_text) + 15, 20);
//}
