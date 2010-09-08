#ifndef CUSTOMPUSHBUTTON_H
#define CUSTOMPUSHBUTTON_H

#include <QPushButton>
#include <QPainter>

class customPushButton : public QPushButton
{
    Q_OBJECT

public:
    customPushButton(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
};

#endif // CUSTOMPUSHBUTTON_H
