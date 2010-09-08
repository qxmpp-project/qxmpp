#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>
#include <QAction>

class customListView : public QListView
{
    Q_OBJECT

public:
    customListView(QWidget* parent = 0);
    bool event(QEvent* e);

public slots:
    void mousePressed(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void clicked(const QModelIndex& index);

private slots:
    void showChatDialog_helper();

protected:
    void keyPressEvent(QKeyEvent*);

signals:
    void showChatDialog(const QString& bareJid);

private:
    QAction m_chat;
    QAction m_profile;
};

#endif // CUSTOMLISTVIEW_H
