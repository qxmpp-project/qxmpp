#ifndef CUSTOMTOOLBUTTON_H
#define CUSTOMTOOLBUTTON_H

#include <QToolButton>

class customToolButton : public QToolButton
{
public:
    customToolButton(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};

#endif // CUSTOMTOOLBUTTON_H
