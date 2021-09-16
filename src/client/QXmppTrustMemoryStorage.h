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

#ifndef QXMPPTRUSTMEMORYSTORAGE_H
#define QXMPPTRUSTMEMORYSTORAGE_H

#include "QXmppGlobal.h"
#include "QXmppTrustStorage.h"

#include <memory>

class QXmppTrustMemoryStoragePrivate;

class QXMPP_EXPORT QXmppTrustMemoryStorage : public QXmppTrustStorage
{
public:
    QXmppTrustMemoryStorage();
    ~QXmppTrustMemoryStorage();

    /// \cond
    QFuture<void> setSecurityPolicies(const QString &encryption = {}, SecurityPolicy securityPolicy = QXmppTrustStorage::NoSecurityPolicy) override;
    QFuture<SecurityPolicy> securityPolicy(const QString &encryption) override;

    QFuture<void> addOwnKey(const QString &encryption, const QString &keyId) override;
    QFuture<void> removeOwnKey(const QString &encryption) override;
    QFuture<QString> ownKey(const QString &encryption) const override;

    QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) override;
    QFuture<void> removeKeys(const QString &encryption = {}, const QList<QString> &keyIds = {}) override;
    QFuture<QHash<TrustLevel, QMultiHash<QString, QString>>> keys(const QString &encryption, TrustLevels trustLevels = {}) const override;

    QFuture<void> setTrustLevel(const QString &encryption, const QMultiHash<QString, QString> &keyIds, const TrustLevel trustLevel) override;
    QFuture<void> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, const TrustLevel oldTrustLevel, const TrustLevel newTrustLevel) override;
    QFuture<TrustLevel> trustLevel(const QString &encryption, const QString &keyId) const override;

    QFuture<void> addKeysForPostponedTrustDecisions(const QString &encryption, const QString &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners) override;
    QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting) override;
    QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption = {}, const QList<QString> &senderKeyIds = {}) override;
    QFuture<QHash<bool, QMultiHash<QString, QString>>> keysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds = {}) override;
    /// \endcond

private:
    std::unique_ptr<QXmppTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPTRUSTMEMORYSTORAGE_H
