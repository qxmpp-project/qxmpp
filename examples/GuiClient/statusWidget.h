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


#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include "ui_statusWidget.h"

#include "QXmppPresence.h"

/// Main Widget for the client's status/status text/avatar management

class statusWidget : public QWidget, public Ui::statusWidgetClass
{
    Q_OBJECT

public:
    statusWidget(QWidget* parent = 0);

    void setDisplayName(const QString& name);
    void setStatusText(const QString& statusText);
    void setPresenceAndStatusType(QXmppPresence::Type presenceType,
                                  QXmppPresence::AvailableStatusType statusType);
    void setAvatar(const QImage&);

private slots:
    void presenceMenuTriggered();
    void avatarSelection();

signals:
    void statusTextChanged(const QString&);
    void presenceTypeChanged(QXmppPresence::Type);
    void presenceStatusTypeChanged(QXmppPresence::AvailableStatusType);
    void avatarChanged(const QImage&);
};

#endif // STATUSWIDGET_H
