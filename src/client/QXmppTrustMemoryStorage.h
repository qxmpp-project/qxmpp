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

    QFuture<void> addOwnKey(const QString &encryption, const QByteArray &keyId) override;
    QFuture<void> removeOwnKey(const QString &encryption) override;
    QFuture<QByteArray> ownKey(const QString &encryption) override;

    QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) override;
    QFuture<void> removeKeys(const QString &encryption = {}, const QList<QByteArray> &keyIds = {}) override;
    QFuture<QHash<TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, TrustLevels trustLevels = {}) override;

    QFuture<void> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, const TrustLevel trustLevel) override;
    QFuture<void> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, const TrustLevel oldTrustLevel, const TrustLevel newTrustLevel) override;
    QFuture<TrustLevel> trustLevel(const QString &encryption, const QByteArray &keyId) override;

    QFuture<void> addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners) override;
    QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting) override;
    QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption = {}, const QList<QByteArray> &senderKeyIds = {}) override;
    QFuture<QHash<bool, QMultiHash<QString, QByteArray>>> keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds = {}) override;
    /// \endcond

private:
    std::unique_ptr<QXmppTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPTRUSTMEMORYSTORAGE_H
