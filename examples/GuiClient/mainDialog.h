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


#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "QXmppClient.h"
#include "rosterItemModel.h"
#include "rosterItemSortFilterProxyModel.h"
#include <QKeyEvent>
#include <QMap>
#include "statusWidget.h"
#include "chatDialog.h"
#include "vCardManager.h"
#include "capabilitiesCollection.h"
#include "accountsCache.h"

namespace Ui
{
    class mainDialogClass;
}

class mainDialog : public QDialog
{
    Q_OBJECT

public:
    mainDialog(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent*);

private slots:
    void rosterChanged(const QString& bareJid);
    void rosterReceived();
    void presenceChanged(const QString&, const QString&);
    void sort();
    void filterChanged(const QString& filter);
    void showChatDialog(const QString& bareJid);
    void messageReceived(const QXmppMessage& msg);
    void statusTextChanged(const QString&);
    void presenceTypeChanged(QXmppPresence::Type);
    void presenceStatusTypeChanged(QXmppPresence::Status::Type);
    void signIn();
    void cancelSignIn();
    void showSignInPage();
    void showSignInPageAfterUserDisconnection();
    void showSignInPageForAutoReconnection(int);
    void showSignInPageForAutoReconnectionNow();
    void showRosterPage();
    void startConnection();
    void updateStatusWidget();
    void showLoginStatusWithProgress(const QString& msg);
    void showLoginStatus(const QString& msg);
    void showLoginStatusWithCounter(const QString& msg, int time);
    void updateVCard(const QString& bareJid);
    void avatarChanged(const QImage&);
    void showProfile(const QString& bareJid);
    void userNameLineEdit_editingFinished();

private:
    void loadAccounts();

    chatDialog* getChatDialog(const QString& bareJid);

    Ui::mainDialogClass* ui;
    QXmppClient m_xmppClient;
    rosterItemModel m_rosterItemModel;
    rosterItemSortFilterProxyModel m_rosterItemSortFilterModel;
    statusWidget m_statusWidget;
    vCardManager m_vCardManager;
    capabilitiesCollection m_capabilitiesCollection;
    accountsCache m_accountsCache;

    // map of bare jids and respective chatdlg
    QMap<QString, chatDialog*> m_chatDlgsList;
};

#endif // MAINDIALOG_H
