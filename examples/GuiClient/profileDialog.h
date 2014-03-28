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


#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QMap>
#include "capabilitiesCache.h"

namespace Ui {
    class profileDialog;
}

class QXmppClient;
class QXmppVersionIq;
class QXmppEntityTimeIq;

class profileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit profileDialog(QWidget *parent, const QString& bareJid, QXmppClient& client, capabilitiesCache& caps);
    ~profileDialog();

    void setClientRef(QXmppClient& m_xmppClient);
    void setAvatar(const QImage&);
    void setBareJid(const QString&);
    void setFullName(const QString&);
    void setStatusText(const QString&);

private slots:
    void versionReceived(const QXmppVersionIq&);
    void timeReceived(const QXmppEntityTimeIq&);

private:
    void updateText();
    QString getCapability(const QString& resource);

private:
    Ui::profileDialog *ui;
    QString m_bareJid;
    QXmppClient& m_xmppClient;  // reference to the active QXmppClient (No ownership)
    capabilitiesCache& m_caps;  // reference to the active QXmppClient (No ownership)
    QMap<QString, QXmppVersionIq> m_versions;
    QMap<QString, QXmppEntityTimeIq> m_time;
};

#endif // PROFILEDIALOG_H
