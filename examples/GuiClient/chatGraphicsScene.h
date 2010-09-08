#ifndef CHATGRAPHICSSCENE_H
#define CHATGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QList>

class messageGraphicsItem;

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
    QList <messageGraphicsItem*> m_items;
};

#endif // CHATGRAPHICSSCENE_H
