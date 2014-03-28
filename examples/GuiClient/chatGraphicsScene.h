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


#ifndef CHATGRAPHICSSCENE_H
#define CHATGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QList>

class chatMsgGraphicsItem;

class chatGraphicsScene : public QGraphicsScene
{
public:
    chatGraphicsScene(QObject* parent = 0);
    void addMessage(const QString& user, const QString& message);
    void setWidthResize(int newWidth, int oldWidth);
    void verticalReposition();
    void setBoxStartLength(int length);

private:
    int m_verticalPosForNewMessage;
    int m_verticalSpacing;
    int m_boxStartLength;
    QList <chatMsgGraphicsItem*> m_items;
};

#endif // CHATGRAPHICSSCENE_H
