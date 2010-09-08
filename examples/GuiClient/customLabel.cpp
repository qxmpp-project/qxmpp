#include "customLabel.h"
#include <QFontMetrics>

customLabel::customLabel(QWidget* parent):QLabel(parent), m_timer(this),
        m_option(None)
{
    m_timer.setSingleShot(false);

    bool check = connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    Q_ASSERT(check);
}

void customLabel::setCustomText(const QString& text, customLabel::Option op,
                   int countDown)
{
    m_text = text;
    m_option = op;
    m_countDown = countDown;
    switch(op)
    {
    case None:
        m_timer.stop();
        m_postfix = "";
        break;
    case WithProgressEllipsis:
//        m_timer.start(400);
        m_postfix = "";
        break;
    case CountDown:
        m_timer.start(1000);
        m_postfix = "";
        break;
    default:
        m_timer.stop();
        m_postfix = "";
        break;
    }

    if(m_option == CountDown)
        setText(m_text.arg(m_countDown) + m_postfix);
    else
        setText(m_text + m_postfix);

    updateGeometry();
}

void customLabel::timeout()
{
    switch(m_option)
    {
    case None:
        break;
    case WithProgressEllipsis:
        if(m_postfix == "")
            m_postfix = ".";
        else if(m_postfix == ".")
            m_postfix = "..";
        else if(m_postfix == "..")
            m_postfix = "...";
        else if(m_postfix == "...")
            m_postfix = "";
        break;
    case CountDown:
        if(m_countDown == 0)
            m_timer.stop();
        --m_countDown;
        break;
    default:
        break;
    }

    if(m_option == CountDown)
        setText(m_text.arg(m_countDown) + m_postfix);
    else
        setText(m_text + m_postfix);
    updateGeometry();
}

//QSize customLabel::sizeHint() const
//{
//    QFont font;
//    QFontMetrics fm(font);
//    return QSize(fm.width(m_text) + 15, 20);
//}
