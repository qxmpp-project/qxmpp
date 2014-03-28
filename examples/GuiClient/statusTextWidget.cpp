/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#include "statusTextWidget.h"
#include <QHBoxLayout>
#include <QMenu>

#include <QStyle>
#include <QStyleOptionFrameV2>

QSize statusLineEdit::sizeHint() const
{
    QFont font;
    QFontMetrics fm(font);
    int width = fm.width(text());
    if(width <= (160 - 8))
        return QSize(width+8, 18);
    else
        return QSize(160, 18);
}

void statusLineEditButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QStyleOptionButton panel;
    initStyleOption(&panel);
    QRect r = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &panel, this);

    QImage image(":/icons/resource/downArrow.png");
    QRect rectDelta(0, 0, 7, 4);
    rectDelta.moveCenter(r.center());
    painter.drawImage(rectDelta, image);
}

void statusLineEdit::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    QLineEdit::selectAll();
}

void statusLineEdit::mousePressEvent(QMouseEvent* event)
{
    QLineEdit::mousePressEvent(event);
    QLineEdit::selectAll();
}

statusTextWidget::statusTextWidget(QWidget* parent)
    : QWidget(parent)
    , m_statusLineEdit(0)
    , m_statusButton(0)
    , m_clearStatusTextHistory("Clear Status Message", this)
{
    bool check;
    Q_UNUSED(check);

    m_statusLineEdit = new statusLineEdit(this);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(m_statusLineEdit);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    m_statusButton = new statusLineEditButton(this);
    layout->addWidget(m_statusButton);
    setLayout(layout);

    check = connect(m_statusButton, SIGNAL(clicked(bool)), SLOT(showMenu()));
    Q_ASSERT(check);

    check = connect(m_statusLineEdit, SIGNAL(textChanged(QString)), SLOT(textChanged()));
    Q_ASSERT(check);

    check = connect(m_statusLineEdit, SIGNAL(editingFinished()), SLOT(statusTextChanged_helper()));
    Q_ASSERT(check);

    check = connect(&m_clearStatusTextHistory, SIGNAL(triggered()), SLOT(clearStatusTextHistory()));
    Q_ASSERT(check);
}

void statusTextWidget::showMenu()
{
    QMenu menu(this);

    int size = m_statusTextActionList.size();
    for(int i = 0; i < size; ++i)
    {
        menu.addAction(m_statusTextActionList.at(size - 1 - i));
    }

    menu.addSeparator();
    menu.addAction(&m_clearStatusTextHistory);
    m_clearStatusTextHistory.setDisabled(size == 0);
    menu.exec(m_statusLineEdit->mapToGlobal(QPoint(0, m_statusLineEdit->height())));
}

void statusTextWidget::textChanged()
{
    m_statusLineEdit->updateGeometry();
}

void statusTextWidget::statusTextChanged_helper()
{
    addStatusTextToList(m_statusLineEdit->text());
    emit statusTextChanged(m_statusLineEdit->text());
    parentWidget()->setFocus();
}

void statusTextWidget::setStatusText(const QString& statusText)
{
    m_statusLineEdit->setText(statusText);
}

void statusTextWidget::addStatusTextToList(const QString& status)
{
    for(int i = 0; i < m_statusTextActionList.size(); ++i)
    {
        if(m_statusTextActionList.at(i)->data().toString() == status)
        {
            QAction* action = m_statusTextActionList.takeAt(i);
            m_statusTextActionList.append(action);
            return;
        }
    }

    QAction* action = new QAction(status, this);
    action->setData(status);
    bool check = connect(action, SIGNAL(triggered()), SLOT(statusTextChanged_menuClick()));
    Q_ASSERT(check);
    Q_UNUSED(check);
    m_statusTextActionList.append(action);
}

void statusTextWidget::statusTextChanged_menuClick()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(action)
    {
        int i = 0;
        while(i < m_statusTextActionList.size() && action != m_statusTextActionList.at(i))
        {
            ++i;
        }

        if(action == m_statusTextActionList.at(i))
        {
            m_statusTextActionList.removeAt(i);
            m_statusTextActionList.append(action);
        }
        m_statusLineEdit->setText(action->data().toString());
        emit statusTextChanged(action->data().toString());
    }
}

void statusTextWidget::clearStatusTextHistory()
{
    emit statusTextChanged("");
}
