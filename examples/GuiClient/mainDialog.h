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

private:
    chatDialog* getChatDialog(const QString& bareJid);

    Ui::mainDialogClass* ui;
    QXmppClient m_xmppClient;
    rosterItemModel m_rosterItemModel;
    rosterItemSortFilterProxyModel m_rosterItemSortFilterModel;
    statusWidget m_statusWidget;
    vCardManager m_vCardManager;

    // map of bare jids and respective chatdlg
    QMap<QString, chatDialog*> m_chatDlgsList;
};

#endif // MAINDIALOG_H
