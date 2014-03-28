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


#ifndef ROSTERLISTVIEW_H
#define ROSTERLISTVIEW_H

#include <QListView>
#include <QAction>

class rosterListView : public QListView
{
    Q_OBJECT

public:
    rosterListView(QWidget* parent = 0);
    bool event(QEvent* e);

public slots:
    void mousePressed(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void clicked(const QModelIndex& index);

private slots:
    void showChatDialog_helper();
    void showProfile_helper();
    void removeContact_helper();

protected:
    void keyPressEvent(QKeyEvent*);

signals:
    void showChatDialog(const QString& bareJid);
    void showProfile(const QString& bareJid);
    void removeContact(const QString& bareJid);

private:
    QString selectedBareJid();

private:
    QAction m_chat;
    QAction m_profile;
    QAction m_removeContact;
};

#endif // ROSTERLISTVIEW_H
