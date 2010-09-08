#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <QPushButton>

class avatarWidget : public QPushButton
{
public:
    avatarWidget(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};

#endif // AVATARWIDGET_H
