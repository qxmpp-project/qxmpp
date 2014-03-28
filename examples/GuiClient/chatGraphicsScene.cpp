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


#include "chatGraphicsScene.h"
#include "chatMsgGraphicsItem.h"
#include "chatGraphicsView.h"

chatGraphicsScene::chatGraphicsScene(QObject* parent) : QGraphicsScene(parent),
        m_verticalPosForNewMessage(0), m_verticalSpacing(5)
{
}

void chatGraphicsScene::addMessage(const QString& user, const QString& message)
{
    chatMsgGraphicsItem* item = new chatMsgGraphicsItem();
    m_items.append(item);
    item->setName(user);
    item->setBoxStartLength(m_boxStartLength);
    item->setText(message);
    item->setViewWidth(350);
//    item->setViewWidth(views().at(0)->size().width());
    item->setPos(0, m_verticalPosForNewMessage);
    int height = item->boundingRect().height();
    m_verticalPosForNewMessage = m_verticalPosForNewMessage + height + m_verticalSpacing;
    addItem(item);

    QRectF rect = sceneRect();
    rect.setHeight(m_verticalPosForNewMessage);
    setSceneRect(rect);
}

void chatGraphicsScene::setWidthResize(int newWidth, int oldWidth)
{
    Q_UNUSED(newWidth);
    Q_UNUSED(oldWidth);
//    verticalReposition();
}

void chatGraphicsScene::verticalReposition()
{
    m_verticalPosForNewMessage = 0;

    chatMsgGraphicsItem* item = 0;
    for(int i = 0; i < m_items.size(); ++i)
    {
        item = m_items.at(i);
        item->setViewWidth(views().at(0)->size().width());
        item->setPos(0, m_verticalPosForNewMessage);
        int height = item->boundingRect().height();
        m_verticalPosForNewMessage = m_verticalPosForNewMessage + height + m_verticalSpacing;
    }

    QRectF rect = sceneRect();
    if(item)
    {
        rect.setHeight(m_verticalPosForNewMessage);
        rect.setWidth(item->getMaxWidth() + item->getBoxStartLength() - 4);
        setSceneRect(rect);
    }
}

void chatGraphicsScene::setBoxStartLength(int length)
{
    m_boxStartLength = length;
}

