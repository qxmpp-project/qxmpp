// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPATMMANAGER_H
#define QXMPPATMMANAGER_H

#include "QXmppAtmTrustStorage.h"
#include "QXmppSendResult.h"
#include "QXmppTrustManager.h"

class QXmppMessage;
class QXmppTrustMessageKeyOwner;

class QXMPP_EXPORT QXmppAtmManager : public QXmppTrustManager
{
    Q_OBJECT

public:
    QXmppAtmManager(QXmppAtmTrustStorage *trustStorage);
    QFuture<void> makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting = {});

protected:
    /// \cond
    void setClient(QXmppClient *client) override;

private slots:
    void handleMessageReceived(const QXmppMessage &message);
    /// \endcond

private:
    QFuture<void> makeTrustDecisions(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIdsForAuthentication, const QMultiHash<QString, QByteArray> &keyIdsForDistrusting);
    QFuture<void> handleMessage(const QXmppMessage &message);

    QFuture<void> authenticate(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds);
    QFuture<void> distrust(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds);

    QFuture<void> distrustAutomaticallyTrustedKeys(const QString &encryption, const QList<QString> &keyOwnerJids);
    QFuture<void> makePostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds);

    QFuture<QXmpp::SendResult> sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid);

    inline QXmppAtmTrustStorage *trustStorage() const
    {
        return dynamic_cast<QXmppAtmTrustStorage *>(QXmppTrustManager::trustStorage());
    }

    friend class tst_QXmppAtmManager;
};

#endif  // QXMPPATMMANAGER_H
