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


#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSystemTrayIcon>
#include <QMenu>

#include "rosterItemModel.h"
#include "rosterItemSortFilterProxyModel.h"
#include "statusWidget.h"
#include "vCardCache.h"
#include "capabilitiesCache.h"
#include "accountsCache.h"
#include "xmlConsoleDialog.h"

#include "QXmppClient.h"

class chatDialog;

class QKeyEvent;

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
    void closeEvent(QCloseEvent* event);

private slots:
    void rosterChanged(const QString& bareJid);
    void rosterReceived();
    void presenceChanged(const QString&, const QString&);
    void filterChanged(const QString& filter);
    void showChatDialog(const QString& bareJid);
    void messageReceived(const QXmppMessage& msg);
    void statusTextChanged(const QString&);
    void presenceTypeChanged(QXmppPresence::Type);
    void presenceStatusTypeChanged(QXmppPresence::AvailableStatusType);
    void signIn();
    void cancelSignIn();
    void showSignInPage();
    void showSignInPageAfterUserDisconnection();
    void showRosterPage();
    void startConnection();
    void updateStatusWidget();
    void showLoginStatusWithProgress(const QString& msg);
    void showLoginStatus(const QString& msg);
    void showLoginStatusWithCounter(const QString& msg, int time);
    void updateVCard(const QString& bareJid);
    void avatarChanged(const QImage&);
    void showProfile(const QString& bareJid);
    void userNameCompleter_activated(const QString&);
    void addAccountToCache();
    void presenceReceived(const QXmppPresence&);
    void errorClient(QXmppClient::Error);

    void action_addContact();
    void action_removeContact(const QString& bareJid);
    void action_signOut();
    void action_quit();
    void action_trayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void action_showXml();
    void action_aboutDlg();
    void action_settingsPressed();

private:
    void loadAccounts();
    void createTrayIconAndMenu();
    void createSettingsMenu();

    void addPhotoHash(QXmppPresence&);

    chatDialog* getChatDialog(const QString& bareJid);

    Ui::mainDialogClass* ui;
    QXmppClient m_xmppClient;
    rosterItemModel m_rosterItemModel;
    rosterItemSortFilterProxyModel m_rosterItemSortFilterModel;
    statusWidget m_statusWidget;
    vCardCache m_vCardCache;
    capabilitiesCache m_capabilitiesCache;
    accountsCache m_accountsCache;

    // map of bare jids and respective chatdlg
    QMap<QString, chatDialog*> m_chatDlgsList;

#ifndef QT_NO_SYSTEMTRAYICON
    QSystemTrayIcon m_trayIcon;
    QMenu m_trayIconMenu;
#endif

    QAction m_quitAction;
    QAction m_signOutAction;

    xmlConsoleDialog m_consoleDlg;

    QMenu* m_settingsMenu;
};

#endif // MAINDIALOG_H
