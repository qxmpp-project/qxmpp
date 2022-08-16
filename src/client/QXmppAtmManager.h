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
template<typename T>
class QXmppTask;

class QXMPP_EXPORT QXmppAtmManager : public QXmppTrustManager
{
    Q_OBJECT

public:
    QXmppAtmManager(QXmppAtmTrustStorage *trustStorage);
    QXmppTask<void> makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting = {});

protected:
    /// \cond
    void setClient(QXmppClient *client) override;

private:
    Q_SLOT void handleMessageReceived(const QXmppMessage &message);
    /// \endcond

    QXmppTask<void> makeTrustDecisions(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIdsForAuthentication, const QMultiHash<QString, QByteArray> &keyIdsForDistrusting);
    QXmppTask<void> handleMessage(const QXmppMessage &message);

    QXmppTask<void> authenticate(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds);
    QXmppTask<void> distrust(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds);

    QXmppTask<void> distrustAutomaticallyTrustedKeys(const QString &encryption, const QList<QString> &keyOwnerJids);
    QXmppTask<void> makePostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds);

    QXmppTask<QXmpp::SendResult> sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid);

    /// \cond
    inline QXmppAtmTrustStorage *trustStorage() const
    {
        return dynamic_cast<QXmppAtmTrustStorage *>(QXmppTrustManager::trustStorage());
    }
    /// \endcond

    friend class tst_QXmppAtmManager;
};

#endif  // QXMPPATMMANAGER_H
