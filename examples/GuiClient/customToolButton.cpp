#include "customToolButton.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOptionToolButton>

customToolButton::customToolButton(QWidget* parent) : QToolButton(parent)
{
    setMinimumSize(QSize(20, 18));
}

void customToolButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QStyleOptionToolButton panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &panel, &painter, this);
    QRect r = rect();
    QFont font;
    painter.setFont(font);
    painter.setPen(Qt::gray);

    QRect rectSize(0, 0, sizeHint().width(), sizeHint().height());
    rectSize.moveCenter(r.center());
    r = rectSize;
    r.adjust(0, 0, -1, -1);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    r.moveLeft(r.left() + 3);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(r, Qt::AlignVCenter|Qt::TextSingleLine, text());

    QImage image(":/icons/resource/downArrow.png");
    QRect rectDelta(0, 0, 7, 4);
    rectDelta.moveRight(r.right() - 4);
    rectDelta.moveCenter(QPoint(rectDelta.center().x(), r.center().y()));
    painter.drawImage(rectDelta, image);
}

QSize customToolButton::sizeHint() const
{
    QFont font;
    font.setBold(true);
    QFontMetrics fm(font);
    int width = fm.width(text());
    if(width <= (160 - 8 - 9))
        return QSize(width+8+9, 18);
    else
        return QSize(160, 18);
}
