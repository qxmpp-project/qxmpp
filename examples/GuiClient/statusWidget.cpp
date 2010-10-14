/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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


#include "statusWidget.h"
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

statusWidget::statusWidget(QWidget* parent)
{
    setupUi(this);

    QMenu* menu = new QMenu(this);
    menu->addAction(actionAvailable);
    menu->addAction(actionBusy);
    menu->addAction(actionAway);
//    menu->addAction(actionInvisible);
    menu->addSeparator();
    menu->addAction(actionSign_out);
    toolButton_userName->setMenu(menu);

    bool check = connect(statusTextWidgetObject, SIGNAL(statusTextChanged(const QString&)), SIGNAL(statusTextChanged(const QString&)));
    Q_ASSERT(check);

    check = connect(actionAvailable, SIGNAL(triggered()), SLOT(presenceMenuTriggered()));
    Q_ASSERT(check);
    check = connect(actionBusy, SIGNAL(triggered()), SLOT(presenceMenuTriggered()));
    Q_ASSERT(check);
    check = connect(actionAway, SIGNAL(triggered()), SLOT(presenceMenuTriggered()));
    Q_ASSERT(check);
    check = connect(actionInvisible, SIGNAL(triggered()), SLOT(presenceMenuTriggered()));
    Q_ASSERT(check);
    check = connect(actionSign_out, SIGNAL(triggered()), SLOT(presenceMenuTriggered()));
    Q_ASSERT(check);

    check = connect(pushButton_avatar, SIGNAL(clicked()), SLOT(avatarSelection()));
    Q_ASSERT(check);
}

void statusWidget::setStatusText(const QString& statusText)
{
    statusTextWidgetObject->setStatusText(statusText);
}

void statusWidget::presenceMenuTriggered()
{
    QString icon = "green";
    QAction* action = qobject_cast<QAction*>(sender());
    if(action == actionAvailable)
    {
        emit presenceTypeChanged(QXmppPresence::Available);
        icon = "green";
    }
    else if(action == actionBusy)
    {
        emit presenceStatusTypeChanged(QXmppPresence::Status::DND);
        icon = "red";
    }
    else if(action == actionAway)
    {
        emit presenceStatusTypeChanged(QXmppPresence::Status::Away);
        icon = "orange";
    }
    else if(action == actionInvisible)
    {
        emit presenceStatusTypeChanged(QXmppPresence::Status::Invisible);
        icon = "gray";
    }
    else if(action == actionSign_out)
    {
        emit presenceTypeChanged(QXmppPresence::Unavailable);
        icon = "gray";
    }
    label->setPixmap(QPixmap(":/icons/resource/"+icon+".png"));
}

void statusWidget::setPresenceAndStatusType(QXmppPresence::Type presenceType,
                                  QXmppPresence::Status::Type statusType)
{
    if(presenceType == QXmppPresence::Available)
    {
        QString icon = "green";
        switch(statusType)
        {
        case QXmppPresence::Status::Online:
        case QXmppPresence::Status::Chat:
            icon = "green";
            break;
        case QXmppPresence::Status::Away:
        case QXmppPresence::Status::XA:
            icon = "orange";
            break;
        case QXmppPresence::Status::DND:
            icon = "red";
            break;
        case QXmppPresence::Status::Invisible:
        case QXmppPresence::Status::Offline:
            icon = "gray";
            break;
        }
        label->setPixmap(QPixmap(":/icons/resource/"+icon+".png"));
    }
    else if(presenceType == QXmppPresence::Unavailable)
    {
        label->setPixmap(QPixmap(":/icons/resource/gray.png"));
    }
}

void statusWidget::avatarSelection()
{
    QString fileFilters = QString("Images (*.png *.jpeg *.jpg *.gif *.bmp);;All Files (*.*)");
    QString file = QFileDialog::getOpenFileName(this, "Select your avatar",
                                                QString(), fileFilters);
    if(file.isEmpty())
        return;

    QImage image;
    if(image.load(file))
    {
        QImage scaled = image.scaled(QSize(96, 96), Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
        emit avatarChanged(scaled);
    }
    else
        QMessageBox::information(this, "Avatar selection", "Invalid image file");
}

void statusWidget::setDisplayName(const QString& name)
{
    toolButton_userName->setText(name);
}

void statusWidget::setAvatar(const QImage& image)
{
    pushButton_avatar->setIcon(QIcon(QPixmap::fromImage(image)));
}
