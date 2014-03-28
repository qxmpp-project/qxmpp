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


#include "chatDialog.h"
#include "ui_chatDialog.h"

#include "chatGraphicsView.h"
#include "chatGraphicsScene.h"
#include "QXmppClient.h"
#include <QPainter>
#include <QPushButton>

chatDialog::chatDialog(QWidget *parent): QDialog(parent, Qt::Window),
    ui(new Ui::chatDialogClass), m_view(0), m_scene(0), m_pushButtonSend(0), m_client(0)
{
    ui->setupUi(this);
    m_view = new chatGraphicsView(this);
    m_scene = new chatGraphicsScene(this);
    m_view->setChatGraphicsScene(m_scene);
    m_pushButtonSend = new QPushButton("Send", this);
//    m_pushButtonSend->setFixedHeight();
//    m_pushButtonSend->setFixedWidth();
    QRect rect = ui->lineEdit->geometry();
    rect.setLeft(rect.right());
    rect.setWidth(60);
    m_pushButtonSend->setGeometry(rect);
    ui->lineEdit->setFocus();
    ui->verticalLayout->insertWidget(0, m_view);
    bool check = connect(m_pushButtonSend, SIGNAL(clicked(bool)), SLOT(sendMessage()));
    Q_ASSERT(check);
    Q_UNUSED(check);
    updateSendButtonGeomerty();
}

void chatDialog::show()
{
    QDialog::show();
}

QString chatDialog::getBareJid() const
{
    return m_bareJid;
}

QString chatDialog::getDisplayName() const
{
    return m_displayName;
}

void chatDialog::setBareJid(const QString& str)
{
    m_bareJid = str;
}

void chatDialog::setDisplayName(const QString& str)
{
    m_displayName = str;
    setWindowTitle(QString("Chat with %1").arg(m_bareJid));

    QFont font;
    font.setBold(true);
    QFontMetrics fontMetrics(font);
    QRect rect = fontMetrics.boundingRect(m_displayName);
    int width = rect.width();

    if(m_scene)
        m_scene->setBoxStartLength(width);
//    ui->horizontalSpacer_2->changeSize(width+20, 10);
    ui->lineEdit->setFixedWidth(350 - width - 25);
    updateSendButtonGeomerty();
}

void chatDialog::setQXmppClient(QXmppClient* client)
{
    m_client = client;
}

void chatDialog::sendMessage()
{
    if(m_client)
        m_client->sendMessage(getBareJid(), ui->lineEdit->text());

    m_view->addMessage("Me", ui->lineEdit->text());
    ui->lineEdit->clear();
}

void chatDialog::messageReceived(const QString& msg)
{
    m_view->addMessage(getDisplayName(), msg);
}

void chatDialog::keyPressEvent(QKeyEvent* event1)
{
    ui->lineEdit->setFocus();
    ui->lineEdit->event(event1);

    if(event1->key() == Qt::Key_Return)
    {
        m_pushButtonSend->click();
    }
    else if(event1->key() == Qt::Key_Escape)
    {
        hide();
    }
}

void chatDialog::paintEvent(QPaintEvent* event)
{
    QDialog::paintEvent(event);
    QPainter p(this);
    p.setPen(Qt::gray);
    p.drawRect(rect().adjusted(5, 5, -6, -6));
}

void chatDialog::resizeEvent(QResizeEvent *)
{
    updateSendButtonGeomerty();
}

void chatDialog::moveEvent(QMoveEvent *)
{
    updateSendButtonGeomerty();
}

void chatDialog::updateSendButtonGeomerty()
{
    QRect rect = ui->lineEdit->geometry();
    rect.setLeft(rect.right() + 6);
    rect.setWidth(60);
    QRect rect2 = rect;
    rect2.setHeight(25);
    rect2.moveCenter(rect.center());
    m_pushButtonSend->setGeometry(rect2);
}
