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


#include "profileDialog.h"
#include "ui_profileDialog.h"
#include "utils.h"

#include "QXmppClient.h"
#include "QXmppVersionIq.h"
#include "QXmppVersionManager.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"
#include "QXmppEntityTimeManager.h"
#include "QXmppEntityTimeIq.h"
#include "QXmppConstants.h"

profileDialog::profileDialog(QWidget *parent, const QString& bareJid, QXmppClient& client, capabilitiesCache& caps) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::profileDialog), m_bareJid(bareJid), m_xmppClient(client), m_caps(caps)
{
    bool check;
    Q_UNUSED(check);

    ui->setupUi(this);

    check = connect(&m_xmppClient.versionManager(), SIGNAL(versionReceived(QXmppVersionIq)),
            SLOT(versionReceived(QXmppVersionIq)));
    Q_ASSERT(check);

    QXmppEntityTimeManager* timeManager = m_xmppClient.findExtension<QXmppEntityTimeManager>();

    if(timeManager)
    {
        check = connect(timeManager, SIGNAL(timeReceived(QXmppEntityTimeIq)),
            SLOT(timeReceived(QXmppEntityTimeIq)));
        Q_ASSERT(check);
    }

    QStringList resources = m_xmppClient.rosterManager().getResources(bareJid);
    foreach(QString resource, resources)
    {
        QString jid = bareJid + "/" + resource;
        m_xmppClient.versionManager().requestVersion(jid);
        if(timeManager)
            timeManager->requestTime(jid);
    }
    updateText();
}

profileDialog::~profileDialog()
{
    delete ui;
}

void profileDialog::setAvatar(const QImage& image)
{
    ui->label_avatar->setPixmap(QPixmap::fromImage(image));
}

void profileDialog::setBareJid(const QString& bareJid)
{
    ui->label_jid->setText(bareJid);
    setWindowTitle(bareJid);
}

void profileDialog::setFullName(const QString& fullName)
{
    if(fullName.isEmpty())
        ui->label_fullName->hide();
    else
        ui->label_fullName->show();

    ui->label_fullName->setText(fullName);
}

void profileDialog::setStatusText(const QString& status)
{
    ui->label_status->setText(status);
}

void profileDialog::versionReceived(const QXmppVersionIq& ver)
{
    m_versions[QXmppUtils::jidToResource(ver.from())] = ver;
    if(ver.type() == QXmppIq::Result)
        updateText();
}

void profileDialog::timeReceived(const QXmppEntityTimeIq& time)
{
    m_time[QXmppUtils::jidToResource(time.from())] = time;
    if(time.type() == QXmppIq::Result)
        updateText();
}

void profileDialog::updateText()
{
    QStringList resources = m_xmppClient.rosterManager().getResources(m_bareJid);
    QString statusText;
    for(int i = 0; i < resources.count(); ++i)
    {
        QString resource = resources.at(i);
        statusText += "<B>Resource: </B>" + resource;
        statusText += "<BR>";
        QXmppPresence presence = m_xmppClient.rosterManager().getPresence(m_bareJid, resource);
        statusText += "<B>Status: </B>" + presenceToStatusText(presence);
        statusText += "<BR>";
        if(m_versions.contains(resource))
        {
            statusText += "<B>Software: </B>" + QString("%1 %2 %3").
                          arg(m_versions[resource].name()).
                          arg(m_versions[resource].version()).
                          arg(m_versions[resource].os());
            statusText += "<BR>";
        }

        if(m_time.contains(resource))
        {
            statusText += "<B>Time: </B>" + QString("utc=%1 [tzo=%2]").
                          arg(m_time[resource].utc().toString()).
                          arg(QXmppUtils::timezoneOffsetToString(m_time[resource].tzo()));
            statusText += "<BR>";
        }

        statusText += getCapability(resource);

        if(i < resources.count() - 1) // skip for the last item
            statusText += "<BR>";
    }
    setStatusText(statusText);
}

QString profileDialog::getCapability(const QString& resource)
{
    QMap<QString, QXmppPresence> presences = m_xmppClient.rosterManager().
                                             getAllPresencesForBareJid(m_bareJid);
    QXmppPresence& pre = presences[resource];
    QString nodeVer;
    QStringList resultFeatures;
    QStringList resultIdentities;

    QString node = pre.capabilityNode();
    QString ver = pre.capabilityVer().toBase64();
    QStringList exts = pre.capabilityExt();
    nodeVer = node + "#" + ver;
    if(m_caps.isCapabilityAvailable(nodeVer))
    {
        resultFeatures << m_caps.getFeatures(nodeVer);
        resultIdentities << m_caps.getIdentities(nodeVer);
    }
    foreach(QString ext, exts)
    {
        nodeVer = node + "#" + ext;
        if(m_caps.isCapabilityAvailable(nodeVer))
        {
            resultFeatures << m_caps.getFeatures(nodeVer);
            resultIdentities << m_caps.getIdentities(nodeVer);
        }
    }

    resultIdentities.removeDuplicates();
    resultFeatures.removeDuplicates();

    QString result;
    result += "<B>Disco Identities:</B><BR>";
    result += resultIdentities.join("<BR>");
    result += "<BR>";
    result += "<B>Disco Features:</B><BR>";
    result += resultFeatures.join("<BR>");
    result += "<BR>";
    return result;
}
