#include "chatGraphicsView.h"
#include "chatGraphicsScene.h"
#include <QResizeEvent>

chatGraphicsView::chatGraphicsView(QWidget* parent) : QGraphicsView(parent)
{
    setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
}

void chatGraphicsView::setChatGraphicsScene(chatGraphicsScene* scene)
{
    m_scene = scene;
    setScene(m_scene);
}

void chatGraphicsView::addMessage(const QString& user, const QString& message)
{
    if(m_scene)
        m_scene->addMessage(user, message);

    QRectF rect = scene()->sceneRect();
    rect.adjust(-4, -4, 4, 4);
    setSceneRect(rect);

    rect = sceneRect();
    rect.setTop(sceneRect().height() - 20);
    rect.setWidth(20);
    ensureVisible(rect, 50, 50);
}

void chatGraphicsView::resizeEvent(QResizeEvent *event)
{
//  pass this to scene
    m_scene->setWidthResize(event->size().width(), event->oldSize().width());
    QGraphicsView::resizeEvent(event);

    QRectF rect = scene()->sceneRect();
    rect.adjust(-4, -4, 4, 4);
    setSceneRect(rect);
}
