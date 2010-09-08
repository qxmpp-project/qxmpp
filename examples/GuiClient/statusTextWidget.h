#ifndef STATUSTEXTWIDGET_H
#define STATUSTEXTWIDGET_H

#include <QWidget>
#include <QAction>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStyleOptionFrameV2>
#include <QFontMetrics>


class statusLineEditButton : public QPushButton
{
    Q_OBJECT

public:
    statusLineEditButton(QWidget* parent = 0): QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
    }
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const
    {
        return QSize(14, 14);
    }
};

class statusLineEdit : public QLineEdit
{
public:
    statusLineEdit(QWidget* parent = 0) : QLineEdit(parent)
    {
        setAttribute(Qt::WA_Hover, true);
        setText("Available");
        setMinimumSize(QSize(20, 18));
    }
    void focusInEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);

    void paintEvent(QPaintEvent* event)
    {
        if(hasFocus())
        {
            QLineEdit::paintEvent(event);
        }
        else
        {
            QPainter p(this);
            QRect r = rect();
            QPalette pal = palette();

            QStyleOptionFrameV2 panel;
            initStyleOption(&panel);
            r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
            r.adjust(-1, -1, 0, 0);
            r.setLeft(r.left() + 4);
            p.setPen(Qt::darkGray);
            p.drawText(r, Qt::AlignVCenter, text());
        }

        if(underMouse() && !hasFocus())
        {
            QPainter p(this);
            QRect r = rect();
            QPalette pal = palette();

            QStyleOptionFrameV2 panel;
            initStyleOption(&panel);
            r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
            r.adjust(-1, -1, 0, 0);
            p.setPen(Qt::gray);
            p.drawRect(r);
            r.setLeft(r.left() + 4);
            p.setPen(Qt::darkGray);
            p.drawText(r, Qt::AlignVCenter, text());
        }
    }
    QSize sizeHint() const;
};

class statusTextWidget : public QWidget
{
    Q_OBJECT

public:
    statusTextWidget(QWidget* parent = 0);
    void setStatusText(const QString& statusText);

public slots:
    void showMenu();
    void textChanged();

private slots:
    void statusTextChanged_helper();
    void statusTextChanged_menuClick();
    void clearStatusTextHistory();

signals:
    void statusTextChanged(const QString&);

private:
    void addStatusTextToList(const QString& status);
    statusLineEdit* m_statusLineEdit;
    statusLineEditButton* m_statusButton;

    QList<QAction*> m_statusTextActionList;
    QAction m_clearStatusTextHistory;
};

#endif // STATUSTEXTWIDGET_H
