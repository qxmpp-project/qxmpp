#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>
#include <QTimer>

class customLabel : public QLabel
{
    Q_OBJECT

public:
    enum Option
    {
        None = 0,
        WithProgressEllipsis,
        CountDown
    };
    customLabel(QWidget* parent = 0);

    void setCustomText(const QString& text, customLabel::Option op = None,
                       int countDown = 0);

//    QSize sizeHint() const;

private slots:
    void timeout();

private:
    QTimer m_timer;
    customLabel::Option m_option;
    QString m_text;
    QString m_postfix;
    int m_countDown;
};

#endif // CUSTOMLABEL_H
