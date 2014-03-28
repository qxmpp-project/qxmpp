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


#ifndef CHATGRAPHICSVIEW_H
#define CHATGRAPHICSVIEW_H

#include <QGraphicsView>
class chatGraphicsScene;

class chatGraphicsView : public QGraphicsView
{
public:
    chatGraphicsView(QWidget* parent = 0);
    void setChatGraphicsScene(chatGraphicsScene* scene);
    void addMessage(const QString& user, const QString& message);

private:
    void resizeEvent(QResizeEvent *event);

    chatGraphicsScene* m_scene;
};

#endif // CHATGRAPHICSVIEW_H
