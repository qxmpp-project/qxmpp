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


#include "mainDialog.h"
#include "ui_mainDialog.h"

#include "utils.h"
#include "profileDialog.h"
#include "aboutDialog.h"
#include "chatDialog.h"

#include "QXmppPresence.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include "QXmppVCardManager.h"

#include <QMovie>
#include <QCompleter>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>


mainDialog::mainDialog(QWidget *parent): QDialog(parent, Qt::Window),
    ui(new Ui::mainDialogClass), m_rosterItemModel(this),
    m_rosterItemSortFilterModel(this), m_vCardCache(&m_xmppClient),
    m_capabilitiesCache(&m_xmppClient), m_accountsCache(this),

#ifndef QT_NO_SYSTEMTRAYICON
    m_trayIcon(this), m_trayIconMenu(this),
#endif

    m_quitAction("Quit", this),
    m_signOutAction("Sign out", this),
    m_settingsMenu(0)
{
    bool check;
    Q_UNUSED(check);

    ui->setupUi(this);
    createTrayIconAndMenu();
    createSettingsMenu();

    ui->pushButton_cancel->setDisabled(true);
    ui->label_throbber->setMovie(new QMovie(":/icons/resource/ajax-loader.gif"));
    ui->label_throbber->movie()->start();
    showSignInPage();
    loadAccounts();

    check = connect(ui->lineEdit_userName->completer(),
                         SIGNAL(activated(QString)),
                         this, SLOT(userNameCompleter_activated(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.rosterManager(),
                         SIGNAL(rosterReceived()),
                         this, SLOT(rosterReceived()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.rosterManager(),
                         SIGNAL(itemChanged(QString)),
                         this, SLOT(rosterChanged(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,
                         SIGNAL(error(QXmppClient::Error)),
                         this, SLOT(errorClient(QXmppClient::Error)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,
                         SIGNAL(presenceReceived(QXmppPresence)),
                         this, SLOT(presenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::SignalLogging);


    check = connect(&m_xmppClient.rosterManager(),
                    SIGNAL(presenceChanged(QString,QString)),
                    this, SLOT(presenceChanged(QString,QString)));
    Q_ASSERT(check);

    check = connect(ui->lineEdit_filter, SIGNAL(textChanged(QString)),
                    this, SLOT(filterChanged(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showChatDialog(QString)),
                    this, SLOT(showChatDialog(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showProfile(QString)),
                    this, SLOT(showProfile(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(removeContact(QString)),
                    this, SLOT(action_removeContact(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(messageReceived(QXmppMessage)),
                    SLOT(messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_signIn, SIGNAL(clicked(bool)), SLOT(signIn()));
    Q_ASSERT(check);

    check = connect(ui->pushButton_cancel, SIGNAL(clicked(bool)),
                    SLOT(cancelSignIn()));
    Q_ASSERT(check);

    m_rosterItemSortFilterModel.setSourceModel(&m_rosterItemModel);
    ui->listView->setModel(&m_rosterItemSortFilterModel);
    m_rosterItemSortFilterModel.sort(0);

    rosterItemDelegate *delegate = new rosterItemDelegate();
    ui->listView->setItemDelegate(delegate);
    ui->listView->setFocus();
    ui->verticalLayout_3->insertWidget(0, &m_statusWidget);

    check = connect(&m_statusWidget, SIGNAL(statusTextChanged(QString)),
                    SLOT(statusTextChanged(QString)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget, SIGNAL(presenceTypeChanged(QXmppPresence::Type)),
                    SLOT(presenceTypeChanged(QXmppPresence::Type)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget,
                    SIGNAL(presenceStatusTypeChanged(QXmppPresence::AvailableStatusType)),
                    SLOT(presenceStatusTypeChanged(QXmppPresence::AvailableStatusType)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget,
                    SIGNAL(avatarChanged(QImage)),
                    SLOT(avatarChanged(QImage)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(updateStatusWidget()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(showRosterPage()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(addAccountToCache()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(disconnected()), SLOT(showSignInPageAfterUserDisconnection()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.vCardManager(),
                    SIGNAL(vCardReceived(QXmppVCardIq)), &m_vCardCache,
                    SLOT(vCardReceived(QXmppVCardIq)));
    Q_ASSERT(check);

    check = connect(&m_vCardCache,
                    SIGNAL(vCardReadyToUse(QString)),
                    SLOT(updateVCard(QString)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_addContact, SIGNAL(clicked()), SLOT(action_addContact()));
    Q_ASSERT(check);

    check = connect(QXmppLogger::getLogger(),
                 SIGNAL(message(QXmppLogger::MessageType,QString)),
                 &m_consoleDlg,
                 SLOT(message(QXmppLogger::MessageType,QString)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_settings,
                 SIGNAL(pressed()),
                 SLOT(action_settingsPressed()));
    Q_ASSERT(check);
}

void mainDialog::rosterChanged(const QString& bareJid)
{
    m_rosterItemModel.updateRosterEntry(bareJid, m_xmppClient.rosterManager().
                                        getRosterEntry(bareJid));

    // if available in cache, update it else based on presence it will request if not available
    if(m_vCardCache.isVCardAvailable(bareJid))
        updateVCard(bareJid);
}

void mainDialog::rosterReceived()
{
    QStringList list = m_xmppClient.rosterManager().getRosterBareJids();
    QString bareJid;
    foreach(bareJid, list)
        rosterChanged(bareJid);
}

void mainDialog::presenceChanged(const QString& bareJid, const QString& resource)
{
    if(bareJid == m_xmppClient.configuration().jidBare())
        return;

    if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid))
        return;

    QString jid = bareJid + "/" + resource;
    QMap<QString, QXmppPresence> presences = m_xmppClient.rosterManager().
                                             getAllPresencesForBareJid(bareJid);
    m_rosterItemModel.updatePresence(bareJid, presences);

    QXmppPresence& pre = presences[resource];

    if(pre.type() == QXmppPresence::Available)
    {
        QString node = pre.capabilityNode();
        QString ver = pre.capabilityVer().toBase64();
        QStringList exts = pre.capabilityExt();

        QString nodeVer = node + "#" + ver;
        if(!m_capabilitiesCache.isCapabilityAvailable(nodeVer))
            m_capabilitiesCache.requestInfo(jid, nodeVer);

        foreach(QString ext, exts)
        {
            nodeVer = node + "#" + ext;
            if(!m_capabilitiesCache.isCapabilityAvailable(nodeVer))
                m_capabilitiesCache.requestInfo(jid, nodeVer);
        }

        switch(pre.vCardUpdateType())
        {
        case QXmppPresence::VCardUpdateNone:
            if(!m_vCardCache.isVCardAvailable(bareJid))
                m_vCardCache.requestVCard(bareJid);
        case QXmppPresence::VCardUpdateNotReady:
            break;
        case QXmppPresence::VCardUpdateNoPhoto:
        case QXmppPresence::VCardUpdateValidPhoto:
            if(m_vCardCache.getPhotoHash(bareJid) != pre.photoHash())
                m_vCardCache.requestVCard(bareJid);
            break;
        }
    }

//    QXmppPresence::Type presenceType = presences.begin().value().getType();

//    if(!m_vCardCache.isVCardAvailable(bareJid) &&
//       presenceType == QXmppPresence::Available)
//    {
//        m_rosterItemModel.updateAvatar(bareJid,
//                                   m_vCardCache.getVCard(bareJid).image);
//    }
}

void mainDialog::filterChanged(const QString& filter)
{
    m_rosterItemSortFilterModel.setFilterRegExp(filter);

    // follow statement selects the first row
    ui->listView->selectionModel()->select(ui->listView->model()->index(0, 0),
                                           QItemSelectionModel::ClearAndSelect);
}

void mainDialog::keyPressEvent(QKeyEvent* event1)
{
    if(ui->stackedWidget->currentIndex() == 0) // roster page
    {
        if(event1->matches(QKeySequence::Find) ||(
           event1->key() <= Qt::Key_9 && event1->key() >= Qt::Key_1) ||
           (event1->key() <= Qt::Key_Z && event1->key() >= Qt::Key_At) ||
           event1->key() == Qt::Key_Backspace)
        {
            ui->lineEdit_filter->setFocus();
            ui->lineEdit_filter->event(event1);
        }
        else if(event1->key() == Qt::Key_Escape)
        {
            ui->lineEdit_filter->clear();
            ui->listView->setFocus();
        }
        else if(event1->key() == Qt::Key_Up ||
                event1->key() == Qt::Key_Down ||
                event1->key() == Qt::Key_PageUp ||
                event1->key() == Qt::Key_PageDown)
        {
            ui->listView->setFocus();
            ui->listView->event(event1);
        }
        else if(event1->key() == Qt::Key_Return && ui->listView->hasFocus())
        {
            ui->listView->event(event1);
        }
    }

// don't close on escape
    if(event1->key() == Qt::Key_Escape)
    {
        event1->ignore();
        return;
    }
// FIXME: I'm not sure what this is supposed to do, but it does not compile.
#if 0
    else if(minimize && e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
    {
        event1->ignore();
        return;
    }
#endif
// don't close on escape

    if(ui->stackedWidget->currentIndex() == 1) // sign in page
    {
        QDialog::keyPressEvent(event1);
        return;
    }
}

chatDialog* mainDialog::getChatDialog(const QString& bareJid)
{
    if(!m_chatDlgsList.contains(bareJid))
    {
        m_chatDlgsList[bareJid] = new chatDialog();
        m_chatDlgsList[bareJid]->setBareJid(bareJid);

        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid))
            return 0;

        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid)->
           getName().isEmpty())
            m_chatDlgsList[bareJid]->setDisplayName(m_rosterItemModel.
                                                getRosterItemFromBareJid(bareJid)->getName());
        else
            m_chatDlgsList[bareJid]->setDisplayName(QXmppUtils::jidToUser(bareJid));

        m_chatDlgsList[bareJid]->setQXmppClient(&m_xmppClient);
    }

    return m_chatDlgsList[bareJid];
}

void mainDialog::showChatDialog(const QString& bareJid)
{
    if(!bareJid.isEmpty())
        getChatDialog(bareJid)->show();
}

void mainDialog::messageReceived(const QXmppMessage& msg)
{
    if (msg.body().isEmpty())
        return;

    chatDialog *dialog = getChatDialog(QXmppUtils::jidToBareJid(msg.from()));
    if (dialog) {
        dialog->show();
        dialog->messageReceived(msg.body());
    }
}

void mainDialog::statusTextChanged(const QString& status)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    presence.setStatusText(status);
    addPhotoHash(presence);
    m_xmppClient.setClientPresence(presence);
}

void mainDialog::presenceTypeChanged(QXmppPresence::Type presenceType)
{
    if(presenceType == QXmppPresence::Unavailable)
    {
        m_xmppClient.disconnectFromServer();
    }
    else if(presenceType == QXmppPresence::Available)
    {
        QXmppPresence newPresence = m_xmppClient.clientPresence();
        newPresence.setType(presenceType);
        newPresence.setAvailableStatusType(QXmppPresence::Online);
        addPhotoHash(newPresence);
        m_xmppClient.setClientPresence(newPresence);
    }
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::presenceStatusTypeChanged(QXmppPresence::AvailableStatusType statusType)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    presence.setType(QXmppPresence::Available);
    presence.setAvailableStatusType(statusType);
    addPhotoHash(presence);
    m_xmppClient.setClientPresence(presence);
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::avatarChanged(const QImage& image)
{
    QXmppVCardIq vcard;
    vcard.setType(QXmppIq::Set);

    QByteArray ba;
    QBuffer buffer(&ba);
    if(buffer.open(QIODevice::WriteOnly))
    {
        if(image.save(&buffer, "PNG"))
        {
            vcard.setPhoto(ba);
            m_xmppClient.sendPacket(vcard);
            m_statusWidget.setAvatar(image);

            m_vCardCache.getVCard(m_xmppClient.configuration().jidBare()) = vcard;
            // update photo hash
            QXmppPresence presence = m_xmppClient.clientPresence();
            addPhotoHash(presence);
            m_xmppClient.setClientPresence(presence);
        }
    }
}

void mainDialog::updateStatusWidget()
{
    const QString bareJid = m_xmppClient.configuration().jidBare();

    // initialise status widget
    updateVCard(bareJid);
    m_statusWidget.setStatusText(presenceToStatusText(m_xmppClient.clientPresence()));
    m_statusWidget.setPresenceAndStatusType(m_xmppClient.clientPresence().type(),
                                            m_xmppClient.clientPresence().availableStatusType());

    // fetch own vCard
    m_vCardCache.requestVCard(bareJid);
}

void mainDialog::signIn()
{
    ui->label_throbber->show();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox_rememberPasswd->setDisabled(true);
    showLoginStatusWithProgress("Connecting");

    QString bareJid = ui->lineEdit_userName->text();
    QString passwd = ui->lineEdit_password->text();

    m_xmppClient.configuration().setJid(bareJid);
    m_xmppClient.configuration().setPassword(passwd);

    m_rosterItemModel.clear();

    m_vCardCache.loadFromFile();
    m_capabilitiesCache.loadFromFile();

    startConnection();
}

void mainDialog::cancelSignIn()
{
    if(!ui->checkBox_rememberPasswd->isChecked())
        ui->lineEdit_password->setText("");

    ui->label_throbber->hide();
    m_xmppClient.disconnectFromServer();
    showSignInPage();
    showLoginStatus("Sign in cancelled");
    addAccountToCache();
}

void mainDialog::showSignInPage()
{
    ui->label_throbber->hide();
    ui->pushButton_signIn->setDisabled(false);
    ui->pushButton_cancel->setDisabled(true);
    ui->lineEdit_userName->setDisabled(false);
    ui->lineEdit_password->setDisabled(false);
    ui->checkBox_rememberPasswd->setDisabled(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void mainDialog::showSignInPageAfterUserDisconnection()
{
    if(!ui->checkBox_rememberPasswd->isChecked())
        ui->lineEdit_password->setText("");

    ui->label_throbber->hide();

    showLoginStatus("Disconnected");
    showSignInPage();
}

void mainDialog::showRosterPage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainDialog::startConnection()
{
//    m_xmppClient.setClientPresence(QXmppPresence());
    m_xmppClient.connectToServer(m_xmppClient.configuration());
}

void mainDialog::showLoginStatus(const QString& msg)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::None);
}

void mainDialog::showLoginStatusWithProgress(const QString& msg)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::WithProgressEllipsis);
}

void mainDialog::showLoginStatusWithCounter(const QString& msg, int time)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::CountDown, time);
}

void mainDialog::updateVCard(const QString& bareJid)
{
    // determine full name
    const QXmppVCardIq vCard = m_vCardCache.getVCard(bareJid);
    QString fullName = vCard.fullName();
    if (fullName.isEmpty())
        fullName = bareJid;

    // determine avatar
    QImage avatar = m_vCardCache.getAvatar(bareJid);
    if (avatar.isNull())
        avatar = QImage(":/icons/resource/avatar.png");

    if (bareJid == m_xmppClient.configuration().jidBare()) {
        // update our own information
        m_statusWidget.setAvatar(avatar);
        m_statusWidget.setDisplayName(fullName);
    } else {
        // update roster information
        m_rosterItemModel.updateAvatar(bareJid, avatar);
        m_rosterItemModel.updateName(bareJid, fullName);
    }
}

void mainDialog::showProfile(const QString& bareJid)
{
    if(bareJid.isEmpty())
        return;

    profileDialog dlg(this, bareJid, m_xmppClient, m_capabilitiesCache);
    dlg.setBareJid(bareJid);
    // TODO use original image
    if(!m_vCardCache.getAvatar(bareJid).isNull())
        dlg.setAvatar(m_vCardCache.getAvatar(bareJid));
    QStringList resources = m_xmppClient.rosterManager().getResources(bareJid);

    dlg.setFullName(m_vCardCache.getVCard(bareJid).fullName());

    if(m_vCardCache.getVCard(bareJid).fullName().isEmpty())
        dlg.setFullName(m_xmppClient.rosterManager().getRosterEntry(bareJid).name());

    dlg.exec();
}

void mainDialog::loadAccounts()
{
    m_accountsCache.loadFromFile();
    QStringList list = m_accountsCache.getBareJids();
    QCompleter *completer = new QCompleter(list, this);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEdit_userName->setCompleter(completer);

    if(!list.isEmpty())
    {
        ui->lineEdit_userName->setText(list.last());
        QString passwd = m_accountsCache.getPassword(list.last());
        ui->lineEdit_password->setText(passwd);
        if(!passwd.isEmpty())
            ui->checkBox_rememberPasswd->setChecked(true);
    }
}

void mainDialog::userNameCompleter_activated(const QString& user)
{
    QString passwd = m_accountsCache.getPassword(user);
    ui->lineEdit_password->setText(passwd);
    if(!passwd.isEmpty())
        ui->checkBox_rememberPasswd->setChecked(true);
}

void mainDialog::addAccountToCache()
{
    QString bareJid = ui->lineEdit_userName->text();
    QString passwd = ui->lineEdit_password->text();
    if(!ui->checkBox_rememberPasswd->isChecked())
        passwd = "";
    m_accountsCache.addAccount(bareJid, passwd);
}

void mainDialog::action_signOut()
{
    m_xmppClient.disconnectFromServer();

    // update widget
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::action_quit()
{
    m_xmppClient.disconnectFromServer();
    QApplication::quit();
}

void mainDialog::createTrayIconAndMenu()
{
    bool check;
    Q_UNUSED(check);

    check = connect(&m_quitAction, SIGNAL(triggered()), SLOT(action_quit()));
    Q_ASSERT(check);

    check = connect(&m_signOutAction, SIGNAL(triggered()), SLOT(action_signOut()));
    Q_ASSERT(check);

#ifndef QT_NO_SYSTEMTRAYICON
    m_trayIcon.setIcon(QIcon(":/icons/resource/icon.png"));

    check = connect(&m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                    SLOT(action_trayIconActivated(QSystemTrayIcon::ActivationReason)));
    Q_ASSERT(check);

    m_trayIconMenu.addAction(&m_signOutAction);
    m_trayIconMenu.addSeparator();
    m_trayIconMenu.addAction(&m_quitAction);

    m_trayIcon.setContextMenu(&m_trayIconMenu);
    m_trayIcon.show();
#endif
}

void mainDialog::createSettingsMenu()
{
    m_settingsMenu = new QMenu(ui->pushButton_settings);
//    ui->pushButton_settings->setMenu(m_settingsMenu);

    QAction* aboutDlg = new QAction("About", ui->pushButton_settings);
    connect(aboutDlg, SIGNAL(triggered()), SLOT(action_aboutDlg()));
    m_settingsMenu->addAction(aboutDlg);

    m_settingsMenu->addSeparator();

    QAction* showXml = new QAction("Show XML Console...", ui->pushButton_settings);
    connect(showXml, SIGNAL(triggered()), SLOT(action_showXml()));
    m_settingsMenu->addAction(showXml);

    QMenu* viewMenu = new QMenu("View", ui->pushButton_settings);
    m_settingsMenu->addMenu(viewMenu);

    QAction* showOfflineContacts = new QAction("Show offline contacts", ui->pushButton_settings);
    showOfflineContacts->setCheckable(true);
    showOfflineContacts->setChecked(true);
    connect(showOfflineContacts, SIGNAL(triggered(bool)),
            &m_rosterItemSortFilterModel, SLOT(setShowOfflineContacts(bool)));
    viewMenu->addAction(showOfflineContacts);

    QAction* sortByName = new QAction("Sort by name", ui->pushButton_settings);
    sortByName->setCheckable(true);
    sortByName->setChecked(false);
    connect(sortByName, SIGNAL(triggered(bool)),
            &m_rosterItemSortFilterModel, SLOT(sortByName(bool)));
    viewMenu->addAction(sortByName);

    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(&m_quitAction);
}

void mainDialog::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void mainDialog::action_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    default:
        ;
    }
}

void mainDialog::action_addContact()
{
    bool ok;
    QString bareJid = QInputDialog::getText(this, "Add a jabber contact",
                                            "Contact ID:", QLineEdit::Normal, "", &ok);

    if(!ok)
        return;

    if(!isValidBareJid(bareJid))
    {
        QMessageBox::information(this, "Invalid ID", "Specified ID <I>"+bareJid + " </I> is invalid.");
        return;
    }

    if(ok && !bareJid.isEmpty())
    {
        QXmppPresence subscribe;
        subscribe.setTo(bareJid);
        subscribe.setType(QXmppPresence::Subscribe);
        m_xmppClient.sendPacket(subscribe);
    }
}

void mainDialog::presenceReceived(const QXmppPresence& presence)
{
    QString from = presence.from();

    QString message;
    switch(presence.type())
    {
    case QXmppPresence::Subscribe:
        {
            message = "<B>%1</B> wants to subscribe";

            int retButton = QMessageBox::question(
                    this, "Contact Subscription", message.arg(from),
                    QMessageBox::Yes, QMessageBox::No);

            switch(retButton)
            {
            case QMessageBox::Yes:
                {
                    QXmppPresence subscribed;
                    subscribed.setTo(from);
                    subscribed.setType(QXmppPresence::Subscribed);
                    m_xmppClient.sendPacket(subscribed);

                    // reciprocal subscription
                    QXmppPresence subscribe;
                    subscribe.setTo(from);
                    subscribe.setType(QXmppPresence::Subscribe);
                    m_xmppClient.sendPacket(subscribe);
                }
                break;
            case QMessageBox::No:
                {
                    QXmppPresence unsubscribed;
                    unsubscribed.setTo(from);
                    unsubscribed.setType(QXmppPresence::Unsubscribed);
                    m_xmppClient.sendPacket(unsubscribed);
                }
                break;
            default:
                break;
            }

            return;
        }
        break;
    case QXmppPresence::Subscribed:
        message = "<B>%1</B> accepted your request";
        break;
    case QXmppPresence::Unsubscribe:
        message = "<B>%1</B> unsubscribe";
        break;
    case QXmppPresence::Unsubscribed:
        message = "<B>%1</B> unsubscribed";
        break;
    default:
        return;
        break;
    }

    if(message.isEmpty())
        return;

    QMessageBox::information(this, "Contact Subscription", message.arg(from),
            QMessageBox::Ok);
}

void mainDialog::action_removeContact(const QString& bareJid)
{
    if(!isValidBareJid(bareJid))
        return;

    int answer = QMessageBox::question(this, "Remove contact",
                            QString("Do you want to remove the contact <I>%1</I>").arg(bareJid),
                            QMessageBox::Yes, QMessageBox::No);
    if(answer == QMessageBox::Yes)
    {
        QXmppRosterIq remove;
        remove.setType(QXmppIq::Set);
        QXmppRosterIq::Item itemRemove;
        itemRemove.setSubscriptionType(QXmppRosterIq::Item::Remove);
        itemRemove.setBareJid(bareJid);
        remove.addItem(itemRemove);
        m_xmppClient.sendPacket(remove);

        m_rosterItemModel.removeRosterEntry(bareJid);
    }
}

void mainDialog::errorClient(QXmppClient::Error error)
{
    ui->label_throbber->hide();

    showSignInPage();

    switch(error)
    {
    case QXmppClient::SocketError:
        showLoginStatus("Socket error");
        break;
    case QXmppClient::KeepAliveError:
        showLoginStatus("Keep alive error");
        break;
    case QXmppClient::XmppStreamError:
        switch(m_xmppClient.xmppStreamError())
        {
        case QXmppStanza::Error::NotAuthorized:
            showLoginStatus("Invalid password");
            break;
        default:
            showLoginStatus("Stream error");
            break;
        }
        break;
    default:
        break;
    }
}

void mainDialog::action_showXml()
{
    m_consoleDlg.show();
}

void mainDialog::addPhotoHash(QXmppPresence& pre)
{
    QString clientBareJid = m_xmppClient.configuration().jidBare();

    if(m_vCardCache.isVCardAvailable(clientBareJid))
    {
        QByteArray hash = m_vCardCache.getPhotoHash(clientBareJid);
        if(hash.isEmpty())
            pre.setVCardUpdateType(QXmppPresence::VCardUpdateNoPhoto);
        else
            pre.setVCardUpdateType(QXmppPresence::VCardUpdateValidPhoto);
        pre.setPhotoHash(hash);
    }
    else
    {
        pre.setVCardUpdateType(QXmppPresence::VCardUpdateNone);
        pre.setPhotoHash(QByteArray());
    }
}

void mainDialog::action_aboutDlg()
{
    aboutDialog abtDlg(this);
    abtDlg.exec();
}

void mainDialog::action_settingsPressed()
{
    m_settingsMenu->exec(ui->pushButton_settings->mapToGlobal(QPoint(0, ui->pushButton_settings->height())));
}
