#include "chatGraphicsScene.h"
#include "messageGraphicsItem.h"
#include "chatGraphicsView.h"

chatGraphicsScene::chatGraphicsScene(QObject* parent) : QGraphicsScene(parent),
        m_verticalPosForNewMessage(0), m_verticalSpacing(5)
{
}

void chatGraphicsScene::addMessage(const QString& user, const QString& message)
{
    messageGraphicsItem* item = new messageGraphicsItem();
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
//    verticalReposition();
}

void chatGraphicsScene::verticalReposition()
{
    m_verticalPosForNewMessage = 0;

    messageGraphicsItem* item = 0;
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

