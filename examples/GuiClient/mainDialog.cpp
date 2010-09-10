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


#include "mainDialog.h"
#include "ui_mainDialog.h"
#include "QXmppRosterManager.h"
#include "QXmppPresence.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include "utils.h"
#include "QXmppReconnectionManager.h"
#include "QXmppVCardManager.h"
#include "QXmppLogger.h"
#include "QXmppVCardIq.h"
#include "QXmppRosterManager.h"
#include "QXmppRosterIq.h"

#include "profileDialog.h"

#include <QMovie>

mainDialog::mainDialog(QWidget *parent): QDialog(parent, Qt::Window),
    ui(new Ui::mainDialogClass), m_rosterItemModel(this),
    m_rosterItemSortFilterModel(this), m_vCardManager(&m_xmppClient)
{
    ui->setupUi(this);
    ui->pushButton_cancel->setDisabled(true);
    ui->label_throbber->setMovie(new QMovie(":/icons/resource/ajax-loader.gif"));
    ui->label_throbber->movie()->start();
    showSignInPage();

    bool check = connect(&m_xmppClient.rosterManager(),
                         SIGNAL(rosterReceived()),
                         this, SLOT(rosterReceived()));

    check = connect(&m_xmppClient.rosterManager(),
                         SIGNAL(rosterChanged(const QString&)),
                         this, SLOT(rosterChanged(const QString&)));
    Q_ASSERT(check);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::FileLogging);


    check = connect(&m_xmppClient.rosterManager(),
                    SIGNAL(presenceChanged(const QString&, const QString&)),
                    this, SLOT(presenceChanged(const QString&, const QString&)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_view, SIGNAL(clicked()), this, SLOT(sort()));
    Q_ASSERT(check);

    check = connect(ui->lineEdit_filter, SIGNAL(textChanged(const QString&)),
                    this, SLOT(filterChanged(const QString&)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showChatDialog(const QString&)),
                    this, SLOT(showChatDialog(const QString&)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showProfile(const QString&)),
                    this, SLOT(showProfile(const QString&)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(messageReceived(const QXmppMessage&)),
                    SLOT(messageReceived(const QXmppMessage&)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_signIn, SIGNAL(clicked(bool)), SLOT(signIn()));
    Q_ASSERT(check);

    check = connect(ui->pushButton_cancel, SIGNAL(clicked(bool)),
                    SLOT(cancelSignIn()));
    Q_ASSERT(check);

    m_rosterItemSortFilterModel.setSourceModel(&m_rosterItemModel);
    ui->listView->setModel(&m_rosterItemSortFilterModel);
    sort();

    ItemDelegate *delegate = new ItemDelegate();
    ui->listView->setItemDelegate(delegate);
    ui->listView->setFocus();
    ui->verticalLayout_3->insertWidget(0, &m_statusWidget);

    check = connect(&m_statusWidget, SIGNAL(statusTextChanged(const QString&)),
                    SLOT(statusTextChanged(const QString&)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget, SIGNAL(presenceTypeChanged(QXmppPresence::Type)),
                    SLOT(presenceTypeChanged(QXmppPresence::Type)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget,
                    SIGNAL(presenceStatusTypeChanged(QXmppPresence::Status::Type)),
                    SLOT(presenceStatusTypeChanged(QXmppPresence::Status::Type)));
    Q_ASSERT(check);
    check = connect(&m_statusWidget,
                    SIGNAL(avatarChanged(const QImage&)),
                    SLOT(avatarChanged(const QImage&)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(updateStatusWidget()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(showRosterPage()));
    Q_ASSERT(check);

    check = connect(m_xmppClient.reconnectionManager(),
                    SIGNAL(reconnectingIn(int)),
                    SLOT(showSignInPageForAutoReconnection(int)));
    Q_ASSERT(check);

    check = connect(m_xmppClient.reconnectionManager(),
                    SIGNAL(reconnectingNow()),
                    SLOT(showSignInPageForAutoReconnectionNow()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.vCardManager(),
                    SIGNAL(vCardReceived(const QXmppVCardIq&)), &m_vCardManager,
                    SLOT(vCardReceived(const QXmppVCardIq&)));
    Q_ASSERT(check);

    check = connect(&m_vCardManager,
                    SIGNAL(vCardReadyToUse(const QString&)),
                    SLOT(updateVCard(const QString&)));
    Q_ASSERT(check);
}

void mainDialog::rosterChanged(const QString& bareJid)
{
    m_rosterItemModel.updateRosterEntry(bareJid, m_xmppClient.rosterManager().
                                        getRosterEntry(bareJid));

    bool check = m_vCardManager.isVCardAvailable(bareJid);
    if(check)
    {
        m_rosterItemModel.updateAvatar(bareJid,
                                   m_vCardManager.getVCard(bareJid).image);
    }
    else
    {
        m_vCardManager.requestVCard(bareJid);
    }
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

    QMap<QString, QXmppPresence> presences = m_xmppClient.rosterManager().
                                             getAllPresencesForBareJid(bareJid);
    m_rosterItemModel.updatePresence(bareJid, presences);

//    QXmppPresence::Type presenceType = presences.begin().value().getType();

//    if(!m_vCardManager.isVCardAvailable(bareJid) &&
//       presenceType == QXmppPresence::Available)
//    {
//        m_rosterItemModel.updateAvatar(bareJid,
//                                   m_vCardManager.getVCard(bareJid).image);
//    }
}

void mainDialog::sort()
{
    m_rosterItemSortFilterModel.sort(0);
}

void mainDialog::filterChanged(const QString& filter)
{
    m_rosterItemSortFilterModel.setFilterRegExp(filter);
    ui->listView->selectionModel()->select(ui->listView->model()->index(0, 0),
                                           QItemSelectionModel::ClearAndSelect);
}

void mainDialog::keyPressEvent(QKeyEvent* event1)
{
    if(ui->stackedWidget->currentIndex() == 1) // sign in page
    {
        QDialog::keyPressEvent(event1);
        return;
    }

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
            m_chatDlgsList[bareJid]->setDisplayName(jidToUser(bareJid));

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
    QString from = msg.from();
    getChatDialog(jidToBareJid(from))->show();
    getChatDialog(jidToBareJid(from))->messageReceived(msg.body());
}

void mainDialog::statusTextChanged(const QString& status)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    presence.status().setStatusText(status);
    m_xmppClient.setClientPresence(presence);
}

void mainDialog::presenceTypeChanged(QXmppPresence::Type presenceType)
{
    if(presenceType == QXmppPresence::Unavailable)
    {
        m_xmppClient.disconnectFromServer();
        showSignInPageAfterUserDisconnection();
    }
    else
    {
        QXmppPresence newPresence = m_xmppClient.clientPresence();
        newPresence.setType(presenceType);
        m_xmppClient.setClientPresence(newPresence);
    }
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::presenceStatusTypeChanged(QXmppPresence::Status::Type statusType)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    if(statusType == QXmppPresence::Status::Offline)
        presence.setType(QXmppPresence::Unavailable);
    else
        presence.setType(QXmppPresence::Available);
    presence.status().setType(statusType);
    m_xmppClient.setClientPresence(presence);
}

void mainDialog::avatarChanged(const QImage& image)
{
    QXmppVCardIq vcard;
    vcard.setType(QXmppIq::Set);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    vcard.setPhoto(ba);

    m_xmppClient.sendPacket(vcard);
    m_statusWidget.setAvatar(image);
}

void mainDialog::updateStatusWidget()
{
    // fetch selfVCard
    m_xmppClient.vCardManager().requestVCard();

    m_statusWidget.setDisplayName(m_xmppClient.configuration().jidBare());
    m_statusWidget.setStatusText(presenceToStatusText(m_xmppClient.clientPresence()));
    m_statusWidget.setPresenceAndStatusType(m_xmppClient.clientPresence().type(),
                                            m_xmppClient.clientPresence().status().type());
}

void mainDialog::signIn()
{
    ui->label_throbber->show();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox->setDisabled(true);
    showLoginStatusWithProgress("Connecting");

    QString bareJid = ui->lineEdit_userName->text();
    QString passwd = ui->lineEdit_password->text();

    m_xmppClient.configuration().setJid(bareJid);
    m_xmppClient.configuration().setPassword(passwd);

    m_vCardManager.loadAllFromCache();
    m_rosterItemModel.clear();
    startConnection();
}

void mainDialog::cancelSignIn()
{
    ui->label_throbber->hide();
    m_xmppClient.reconnectionManager()->cancelReconnection();
    m_xmppClient.disconnectFromServer();
    showSignInPage();
    showLoginStatus("Sign in cancelled");
}

void mainDialog::showSignInPage()
{
    ui->label_throbber->hide();
    ui->pushButton_signIn->setDisabled(false);
    ui->pushButton_cancel->setDisabled(true);
    ui->lineEdit_userName->setDisabled(false);
    ui->lineEdit_password->setDisabled(false);
    ui->checkBox->setDisabled(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void mainDialog::showSignInPageAfterUserDisconnection()
{
    ui->label_throbber->hide();
    showLoginStatus("Disconnected");
    showSignInPage();
}

void mainDialog::showSignInPageForAutoReconnection(int i)
{
    ui->label_throbber->hide();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox->setDisabled(true);
    showLoginStatusWithCounter(QString("Reconnecting in %1 sec..."), i);
    ui->stackedWidget->setCurrentIndex(1);
}

void mainDialog::showSignInPageForAutoReconnectionNow()
{
    ui->label_throbber->show();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox->setDisabled(true);
    showLoginStatusWithProgress(QString("Connecting"));
    ui->stackedWidget->setCurrentIndex(1);
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
    ui->label_status->setCustomText(msg, customLabel::None);
}

void mainDialog::showLoginStatusWithProgress(const QString& msg)
{
    ui->label_status->setCustomText(msg, customLabel::WithProgressEllipsis);
}

void mainDialog::showLoginStatusWithCounter(const QString& msg, int time)
{
    ui->label_status->setCustomText(msg, customLabel::CountDown, time);
}

void mainDialog::updateVCard(const QString& bareJid)
{
    if(bareJid != m_xmppClient.configuration().jidBare())
        m_rosterItemModel.updateAvatar(bareJid,
                                   m_vCardManager.getVCard(bareJid).image);
    else
    {
        if(m_vCardManager.getSelfFullName().isEmpty())
            m_statusWidget.setDisplayName(m_xmppClient.configuration().jidBare());
        else
            m_statusWidget.setDisplayName(m_vCardManager.getSelfFullName());

        m_statusWidget.setAvatar(m_vCardManager.getVCard(bareJid).image);
    }
}

void mainDialog::showProfile(const QString& bareJid)
{
    if(bareJid.isEmpty())
        return;

    profileDialog dlg(this, bareJid, m_xmppClient);
    dlg.setBareJid(bareJid);
    if(!m_vCardManager.getVCard(bareJid).imageOriginal.isNull())
        dlg.setAvatar(m_vCardManager.getVCard(bareJid).imageOriginal);
    QStringList resources = m_xmppClient.rosterManager().getResources(bareJid);

    dlg.setFullName(m_vCardManager.getVCard(bareJid).fullName);

    if(m_vCardManager.getVCard(bareJid).fullName.isEmpty())
        dlg.setFullName(m_xmppClient.rosterManager().getRosterEntry(bareJid).name());

    dlg.exec();
}
