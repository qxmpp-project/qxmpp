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

#ifndef QXMPPTRUSTSTORAGE_H
#define QXMPPTRUSTSTORAGE_H

#include "QXmppGlobal.h"

#include <QFuture>

class QXmppTrustMessageKeyOwner;

class QXMPP_EXPORT QXmppTrustStorage
{
public:
    ///
    /// security policy to decide which public long-term keys are used for
    /// encryption because they are trusted
    ///
    enum SecurityPolicy {
        NoSecurityPolicy,  ///< New keys must be trusted manually.
        Toakafa,           ///< New keys are trusted automatically until the first authentication but automatically distrusted afterwards.
    };

    ///
    /// trust level of public long-term keys used by end-to-end encryption
    /// protocols
    ///
    enum TrustLevel {
        AutomaticallyDistrusted = 1,  ///< The key is automatically distrusted (e.g., by ATM's security policy).
        ManuallyDistrusted = 2,       ///< The key is manually distrusted (e.g., by clicking a button or ATM).
        AutomaticallyTrusted = 4,     ///< The key is automatically trusted (e.g., by the client for all keys of a bare JID until one of it is authenticated).
        ManuallyTrusted = 8,          ///< The key is manually trusted (e.g., by clicking a button).
        Authenticated = 16,           ///< The key is authenticated (e.g., by QR code scanning or ATM).
    };
    Q_DECLARE_FLAGS(TrustLevels, TrustLevel)

    virtual QFuture<void> setSecurityPolicies(const QString &encryption = {}, SecurityPolicy securityPolicy = SecurityPolicy::NoSecurityPolicy) = 0;
    virtual QFuture<SecurityPolicy> securityPolicy(const QString &encryption) = 0;

    virtual QFuture<void> addOwnKey(const QString &encryption, const QString &keyId) = 0;
    virtual QFuture<void> removeOwnKey(const QString &encryption) = 0;
    virtual QFuture<QString> ownKey(const QString &encryption) const = 0;

    virtual QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption = {}, const QList<QString> &keyIds = {}) = 0;
    virtual QFuture<QHash<TrustLevel, QMultiHash<QString, QString>>> keys(const QString &encryption, TrustLevels trustLevels = {}) const = 0;

    virtual QFuture<void> setTrustLevel(const QString &encryption, const QMultiHash<QString, QString> &keyIds, TrustLevel trustLevel) = 0;
    virtual QFuture<void> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevel oldTrustLevel, TrustLevel newTrustLevel) = 0;
    virtual QFuture<TrustLevel> trustLevel(const QString &encryption, const QString &keyId) const = 0;

    virtual QFuture<void> addKeysForPostponedTrustDecisions(const QString &encryption, const QString &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners) = 0;
    virtual QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting) = 0;
    virtual QFuture<void> removeKeysForPostponedTrustDecisions(const QString &encryption = {}, const QList<QString> &senderKeyIds = {}) = 0;
    virtual QFuture<QHash<bool, QMultiHash<QString, QString>>> keysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds = {}) = 0;
};

Q_DECLARE_METATYPE(QXmppTrustStorage::SecurityPolicy)
Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppTrustStorage::TrustLevels)

#endif  // QXMPPTRUSTSTORAGE_H
