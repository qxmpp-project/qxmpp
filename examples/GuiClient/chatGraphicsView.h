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
