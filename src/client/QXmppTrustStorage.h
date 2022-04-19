// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTSTORAGE_H
#define QXMPPTRUSTSTORAGE_H

#include "QXmppGlobal.h"

#include <QFuture>

class QXMPP_EXPORT QXmppTrustStorage
{
public:
    ///
    /// Security policy to decide which public long-term keys are used for
    /// encryption because they are trusted
    ///
    enum SecurityPolicy {
        NoSecurityPolicy,  ///< New keys must be trusted manually.
        Toakafa,           ///< New keys are trusted automatically until the first authentication but automatically distrusted afterwards. \see \xep{0450, Automatic Trust Management (ATM)}
    };

    ///
    /// Trust level of public long-term keys used by end-to-end encryption
    /// protocols
    ///
    enum TrustLevel {
        Undecided = 1,                ///< The key's trust is not decided.
        AutomaticallyDistrusted = 2,  ///< The key is automatically distrusted (e.g., by the security policy TOAKAFA). \see SecurityPolicy
        ManuallyDistrusted = 4,       ///< The key is manually distrusted (e.g., by clicking a button or \xep{0450, Automatic Trust Management (ATM)}).
        AutomaticallyTrusted = 8,     ///< The key is automatically trusted (e.g., by the client for all keys of a bare JID until one of it is authenticated).
        ManuallyTrusted = 16,         ///< The key is manually trusted (e.g., by clicking a button).
        Authenticated = 32,           ///< The key is authenticated (e.g., by QR code scanning or \xep{0450, Automatic Trust Management (ATM)}).
    };
    Q_DECLARE_FLAGS(TrustLevels, TrustLevel)

    virtual ~QXmppTrustStorage() = default;

    virtual QFuture<void> setSecurityPolicy(const QString &encryption, SecurityPolicy securityPolicy) = 0;
    virtual QFuture<void> resetSecurityPolicy(const QString &encryption) = 0;
    virtual QFuture<SecurityPolicy> securityPolicy(const QString &encryption) = 0;

    virtual QFuture<void> setOwnKey(const QString &encryption, const QByteArray &keyId) = 0;
    virtual QFuture<void> resetOwnKey(const QString &encryption) = 0;
    virtual QFuture<QByteArray> ownKey(const QString &encryption) = 0;

    virtual QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption, const QString &keyOwnerJid) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption) = 0;
    virtual QFuture<QHash<TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, TrustLevels trustLevels = {}) = 0;
    virtual QFuture<QHash<QString, QHash<QByteArray, TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevels trustLevels = {}) = 0;
    virtual QFuture<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, TrustLevels trustLevels) = 0;

    virtual QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, TrustLevel trustLevel) = 0;
    virtual QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevel oldTrustLevel, TrustLevel newTrustLevel) = 0;
    virtual QFuture<TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId) = 0;

    virtual QFuture<void> resetAll(const QString &encryption) = 0;
};

Q_DECLARE_METATYPE(QXmppTrustStorage::SecurityPolicy)
Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppTrustStorage::TrustLevels)

#endif  // QXMPPTRUSTSTORAGE_H
