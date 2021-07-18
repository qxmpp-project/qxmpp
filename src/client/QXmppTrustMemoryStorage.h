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

class ProcessedKey
{
public:
    bool operator==(const ProcessedKey &other) const;

    QString id;
    QString ownerJid;
    QXmppTrustStorage::TrustLevel trustLevel;
};

class UnprocessedKey
{
public:
    bool operator==(const UnprocessedKey &other) const;

    QString id;
    QString ownerJid;
    QString senderKeyId;
    bool trust;
};

class QXmppTrustMemoryStoragePrivate;

class QXMPP_EXPORT QXmppTrustMemoryStorage : public QXmppTrustStorage
{
public:
    QXmppTrustMemoryStorage();
    ~QXmppTrustMemoryStorage();

    void addOwnKey(const QString &encryption, const QString &keyId) override;
    void removeOwnKey(const QString &encryption) override;
    QFuture<QString> ownKey(const QString &encryption) const override;

    void addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) override;
    void removeKeys(const QString &encryption = {}, const QList<QString> &keyIds = {}) override;

    QFuture<QList<QString>> keys(const QString &encryption) const override;
    QFuture<QList<QString>> keys(const QString &encryption, const QString &keyOwnerJid) const override;
    QFuture<QList<QString>> keys(const QString &encryption, const QString &keyOwnerJid, const TrustLevel trustLevel) const override;

    QFuture<QList<QString>> trustedKeys(const QString &encryption, const QString &keyOwnerJid) const override;
    QFuture<QList<QString>> unauthenticatedKeys(const QString &encryption, const QString &keyOwnerJid) const override;
    QFuture<QMultiHash<QString, QString>> authenticatedKeys(const QString &encryption) const override;
    QFuture<QMultiHash<QString, QString>> unauthenticatedKeys(const QString &encryption) const override;

    QFuture<QList<QString>> keyOwnersWithAuthenticatedKeys(const QString &encryption) const override;

    void setTrustLevel(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const TrustLevel trustLevel) override;
    QFuture<TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QString &keyId) const override;

    void addKeysForLaterTrustDecisions(const QString &encryption, const QString &senderKeyId, const QString &keyOwnerJid, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting) override;
    void removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &keyIds, const bool trust) override;
    void removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds) override;

    QFuture<QMultiHash<QString, QString>> keysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds, const bool trust) override;

private:
    QSharedDataPointer<QXmppTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPTRUSTMEMORYSTORAGE_H
