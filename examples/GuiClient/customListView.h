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


#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>
#include <QAction>

class customListView : public QListView
{
    Q_OBJECT

public:
    customListView(QWidget* parent = 0);
    bool event(QEvent* e);

public slots:
    void mousePressed(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void clicked(const QModelIndex& index);

private slots:
    void showChatDialog_helper();
    void showProfile_helper();

protected:
    void keyPressEvent(QKeyEvent*);

signals:
    void showChatDialog(const QString& bareJid);
    void showProfile(const QString& bareJid);

private:
    QString selectedBareJid();

private:
    QAction m_chat;
    QAction m_profile;
};

#endif // CUSTOMLISTVIEW_H
