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

class QXMPP_EXPORT QXmppTrustStorage
{
public:
    // trust level of public long-term keys used by end-to-end encryption
    // protocols
    enum TrustLevel {
        AutomaticallyDistrusted,  ///< The key is automatically distrusted (e.g., by ATM's security policy).
        ManuallyDistrusted,       ///< The key is manually distrusted (e.g., by clicking a button or ATM).
        AutomaticallyTrusted,     ///< The key is automatically trusted (e.g., by the client for all keys of a bare JID until one of it is authenticated).
        ManuallyTrusted,          ///< The key is manually trusted (e.g., by clicking a button).
        Authenticated,            ///< The key is authenticated (e.g., by QR code scanning or ATM).
    };

    virtual void addOwnKey(const QString &encryption, const QString &keyId) = 0;
    virtual void removeOwnKey(const QString &encryption) = 0;
    virtual QFuture<QString> ownKey(const QString &encryption) const = 0;

    virtual void addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) = 0;
    virtual void removeKeys(const QString &encryption = {}, const QList<QString> &keyIds = {}) = 0;

    virtual QFuture<QList<QString>> keys(const QString &encryption) const = 0;
    virtual QFuture<QList<QString>> keys(const QString &encryption, const QString &keyOwnerJid) const = 0;
    virtual QFuture<QList<QString>> keys(const QString &encryption, const QString &keyOwnerJid, const TrustLevel trustLevel) const = 0;

    virtual QFuture<QList<QString>> trustedKeys(const QString &encryption, const QString &keyOwnerJid) const = 0;
    virtual QFuture<QList<QString>> unauthenticatedKeys(const QString &encryption, const QString &keyOwnerJid) const = 0;
    virtual QFuture<QMultiHash<QString, QString>> authenticatedKeys(const QString &encryption) const = 0;
    virtual QFuture<QMultiHash<QString, QString>> unauthenticatedKeys(const QString &encryption) const = 0;

    virtual QFuture<QList<QString>> keyOwnersWithAuthenticatedKeys(const QString &encryption) const = 0;

    virtual void setTrustLevel(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const TrustLevel trustLevel) = 0;
    virtual QFuture<TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QString &keyId) const = 0;

    virtual void addKeysForLaterTrustDecisions(const QString &encryption, const QString &senderKeyId, const QString &keyOwnerJid, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting) = 0;
    virtual void removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &keyIds, const bool trust) = 0;
    virtual void removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds) = 0;

    virtual QFuture<QMultiHash<QString, QString>> keysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds, const bool trust) = 0;
};

#endif  // QXMPPTRUSTSTORAGE_H
