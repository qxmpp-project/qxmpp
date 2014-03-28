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


#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui
{
    class chatDialogClass;
}

class chatGraphicsView;
class chatGraphicsScene;
class QXmppClient;
class QPushButton;

class chatDialog : public QDialog
{
    Q_OBJECT

public:
    chatDialog(QWidget *parent = 0);
    void show();

    QString getBareJid() const;
    QString getDisplayName() const;
    void setBareJid(const QString&);
    void setDisplayName(const QString&);
    void setQXmppClient(QXmppClient* client);
    void messageReceived(const QString& msg);

private slots:
    void sendMessage();

protected:
    void keyPressEvent(QKeyEvent*);
    void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);

private:
    void updateSendButtonGeomerty();

    Ui::chatDialogClass *ui;
    chatGraphicsView* m_view;
    chatGraphicsScene* m_scene;
    QPushButton* m_pushButtonSend;

    // holds a reference to the the connected client
    QXmppClient* m_client;

    QString m_bareJid;
    QString m_displayName;
};

#endif // CHATDIALOG_H
