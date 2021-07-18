/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin <melvo@olomono.de>
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#ifndef QXMPPATMMANAGER_H
#define QXMPPATMMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppMessage.h"
#include "QXmppTrustMessageKeyOwner.h"
#include "QXmppTrustStorage.h"

class QXmppAtmManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppAtmManager(QXmppTrustStorage *trustStorage);
    void makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting = {});

    /// \cond
    bool handleStanza(const QDomElement &stanza) override;

protected:
    void setClient(QXmppClient *client) override;

private slots:
    void handleMessageReceived(const QXmppMessage &message);
    /// \endcond

private:
    void authenticate(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds);
    void distrust(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds);

    void distrustAutomaticallyTrustedKeys(const QString &encryption, const QString &keyOwnerJid);
    void makeUpForTrustDecision(const QString &encryption, const QList<QString> &senderKeyIds);

    void sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid);

    QXmppTrustStorage *m_trustStorage;

    friend class tst_QXmppAtmManager;
};

#endif  // QXMPPATMMANAGER_H
