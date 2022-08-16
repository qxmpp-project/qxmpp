// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPATMTRUSTMEMORYSTORAGE_H
#define QXMPPATMTRUSTMEMORYSTORAGE_H

#include "QXmppAtmTrustStorage.h"
#include "QXmppTrustMemoryStorage.h"

class QXmppAtmTrustMemoryStoragePrivate;

class QXMPP_EXPORT QXmppAtmTrustMemoryStorage : virtual public QXmppAtmTrustStorage, public QXmppTrustMemoryStorage
{
public:
    QXmppAtmTrustMemoryStorage();
    ~QXmppAtmTrustMemoryStorage();

    /// \cond
    QXmppTask<void> addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners) override;
    QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting) override;
    QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds) override;
    QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption) override;
    QXmppTask<QHash<bool, QMultiHash<QString, QByteArray>>> keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds = {}) override;

    QXmppTask<void> resetAll(const QString &encryption) override;
    /// \endcond

private:
    const std::unique_ptr<QXmppAtmTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPATMTRUSTMEMORYSTORAGE_H
