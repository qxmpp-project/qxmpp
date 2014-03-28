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


#ifndef STATUSAVATARWIDGET_H
#define STATUSAVATARWIDGET_H

#include <QPushButton>

class statusAvatarWidget : public QPushButton
{
public:
    statusAvatarWidget(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};

#endif // STATUSAVATARWIDGET_H
