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

#include "QXmppTrustMemoryStorage.h"

#include "QXmppFutureUtils_p.h"
#include "QXmppTrustMessageKeyOwner.h"

using namespace QXmpp::Private;

///
/// \class QXmppTrustMemoryStorage
///
/// \brief The QXmppTrustMemoryStorage class stores trust data for end-to-end
/// encryption in the memory.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

struct ProcessedKey
{
    bool operator==(const ProcessedKey &other) const
    {
        return id == other.id &&
            ownerJid == other.ownerJid &&
            trustLevel == other.trustLevel;
    }

    QString id;
    QString ownerJid;
    QXmppTrustStorage::TrustLevel trustLevel;
};

struct UnprocessedKey
{
    bool operator==(const UnprocessedKey &other) const
    {
        return id == other.id &&
            ownerJid == other.ownerJid &&
            senderKeyId == other.senderKeyId &&
            trust == other.trust;
    }

    QString id;
    QString ownerJid;
    QString senderKeyId;
    bool trust;
};

class QXmppTrustMemoryStoragePrivate
{
public:
    // encryption protocols mapped to security policies
    QMap<QString, QXmppTrustStorage::SecurityPolicy> securityPolicies;

    // encryption protocols mapped to keys of this client instance
    QMap<QString, QString> ownKeys;

    // encryption protocols mapped to keys with specified trust levels
    QMultiHash<QString, ProcessedKey> processedKeys;

    // encryption protocols mapped to trust message data received from endpoints
    // with unauthenticated keys
    QMultiHash<QString, UnprocessedKey> unprocessedKeys;
};

///
/// Constructs a trust memory storage.
///
QXmppTrustMemoryStorage::QXmppTrustMemoryStorage()
    : d(new QXmppTrustMemoryStoragePrivate)
{
}

QFuture<void> QXmppTrustMemoryStorage::setSecurityPolicies(const QString &encryption, const QXmppTrustStorage::SecurityPolicy securityPolicy)
{
    if (encryption.isEmpty()) {
        d->securityPolicies.clear();
    } else if (securityPolicy == QXmppTrustStorage::NoSecurityPolicy) {
        d->securityPolicies.remove(encryption);
    } else {
        d->securityPolicies.insert(encryption, securityPolicy);
    }

    return makeReadyFuture();
}

QFuture<QXmppTrustStorage::SecurityPolicy> QXmppTrustMemoryStorage::securityPolicy(const QString &encryption)
{
    return makeReadyFuture(std::move(d->securityPolicies.value(encryption)));
}

QXmppTrustMemoryStorage::~QXmppTrustMemoryStorage() = default;

/// \cond
QFuture<void> QXmppTrustMemoryStorage::addOwnKey(const QString &encryption, const QString &keyId)
{
    d->ownKeys.insert(encryption, keyId);
    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeOwnKey(const QString &encryption)
{
    d->ownKeys.remove(encryption);
    return makeReadyFuture();
}

QFuture<QString> QXmppTrustMemoryStorage::ownKey(const QString &encryption) const
{
    auto key = d->ownKeys[encryption];
    return makeReadyFuture(std::move(key));
}

QFuture<void> QXmppTrustMemoryStorage::addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const QXmppTrustStorage::TrustLevel trustLevel)
{
    for (const auto &keyId : keyIds) {
        ProcessedKey key;
        key.id = keyId;
        key.ownerJid = keyOwnerJid;
        key.trustLevel = trustLevel;
        d->processedKeys.insert(encryption, key);
    }

    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeKeys(const QString &encryption, const QList<QString> &keyIds)
{
    if (encryption.isEmpty()) {
        d->processedKeys.clear();
    } else if (keyIds.isEmpty()) {
        d->processedKeys.remove(encryption);
    } else {
        QList<ProcessedKey> keys;

        const auto processedKeys = d->processedKeys.values(encryption);
        for (const auto &key : processedKeys) {
            if (keyIds.contains(key.id)) {
                keys.append(key);
            }
        }

        for (const auto &key : std::as_const(keys)) {
            d->processedKeys.remove(encryption, key);
        }
    }

    return makeReadyFuture();
}

QFuture<QHash<QXmppTrustStorage::TrustLevel, QMultiHash<QString, QString>>> QXmppTrustMemoryStorage::keys(const QString &encryption, const TrustLevels trustLevels) const
{
    QHash<TrustLevel, QMultiHash<QString, QString>> keys;

    const auto processedKeys = d->processedKeys.values(encryption);
    for (const auto &key : processedKeys) {
        const auto trustLevel = key.trustLevel;
        if (trustLevels.testFlag(trustLevel) || !trustLevels) {
            keys[trustLevel].insert(key.ownerJid, key.id);
        }
    }

    return makeReadyFuture(std::move(keys));
}

QFuture<void> QXmppTrustMemoryStorage::setTrustLevel(const QString &encryption, const QMultiHash<QString, QString> &keyIds, const TrustLevel trustLevel)
{
    for (auto itr = keyIds.constBegin(); itr != keyIds.constEnd(); ++itr) {
        const auto keyOwnerJid = itr.key();
        const auto keyId = itr.value();

        auto isKeyFound = false;

        for (auto itrProcessedKeys = d->processedKeys.find(encryption); itrProcessedKeys != d->processedKeys.end() && itrProcessedKeys.key() == encryption; ++itrProcessedKeys) {
            auto &key = itrProcessedKeys.value();
            if (key.id == keyId && key.ownerJid == keyOwnerJid) {
                if (key.trustLevel != trustLevel) {
                    // Update the stored trust level if it differs from the new one.
                    key.trustLevel = trustLevel;
                }

                isKeyFound = true;
                break;
            }
        }

        if (!isKeyFound) {
            // Create a new entry and store it if there is no such entry yet.
            ProcessedKey key;
            key.id = keyId;
            key.ownerJid = keyOwnerJid;
            key.trustLevel = trustLevel;
            d->processedKeys.insert(encryption, key);
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, const QXmppTrustStorage::TrustLevel oldTrustLevel, const QXmppTrustStorage::TrustLevel newTrustLevel)
{
    for (auto itr = d->processedKeys.find(encryption); itr != d->processedKeys.end() && itr.key() == encryption; ++itr) {
        auto &key = itr.value();
        if (keyOwnerJids.contains(key.ownerJid) && key.trustLevel == oldTrustLevel) {
            key.trustLevel = newTrustLevel;
        }
    }

    return makeReadyFuture();
}

QFuture<QXmppTrustStorage::TrustLevel> QXmppTrustMemoryStorage::trustLevel(const QString &encryption, const QString &keyId) const
{
    const auto processedKeys = d->processedKeys.values(encryption);
    for (const auto &key : processedKeys) {
        if (key.id == keyId) {
            return makeReadyFuture(std::move(QXmppTrustStorage::TrustLevel(key.trustLevel)));
        }
    }

    return makeReadyFuture(std::move(TrustLevel::AutomaticallyDistrusted));
}

QFuture<void> QXmppTrustMemoryStorage::addKeysForPostponedTrustDecisions(const QString &encryption, const QString &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners)
{
    const auto addKeys = [&](const QXmppTrustMessageKeyOwner &keyOwner, bool trust, const QList<QString> &keyIds) {
        for (const auto &keyId : keyIds) {
            auto isKeyFound = false;

            for (auto itr = d->unprocessedKeys.find(encryption); itr != d->unprocessedKeys.end() && itr.key() == encryption; ++itr) {
                auto &key = itr.value();
                if (key.id == keyId && key.ownerJid == keyOwner.jid() && key.senderKeyId == senderKeyId) {
                    if (key.trust != trust) {
                        // Update the stored trust if it differs from the new one.
                        key.trust = trust;
                    }

                    isKeyFound = true;
                    break;
                }
            }

            if (!isKeyFound) {
                // Create a new entry and store it if there is no such entry yet.
                UnprocessedKey key;
                key.id = keyId;
                key.ownerJid = keyOwner.jid();
                key.senderKeyId = senderKeyId;
                key.trust = trust;
                d->unprocessedKeys.insert(encryption, key);
            }
        }
    };

    for (const auto &keyOwner : keyOwners) {
        addKeys(keyOwner, true, keyOwner.trustedKeys());
        addKeys(keyOwner, false, keyOwner.distrustedKeys());
    }

    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting)
{
    QList<UnprocessedKey> keys;

    const auto unprocessedKeys = d->unprocessedKeys.values(encryption);
    for (const auto &key : unprocessedKeys) {
        if ((key.trust && keyIdsForAuthentication.contains(key.id)) ||
            (!key.trust && keyIdsForDistrusting.contains(key.id))) {
            keys.append(key);
        }
    }

    for (const auto &key : std::as_const(keys)) {
        d->unprocessedKeys.remove(encryption, key);
    }

    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds)
{
    if (encryption.isEmpty()) {
        d->unprocessedKeys.clear();
    } else if (senderKeyIds.isEmpty()) {
        d->unprocessedKeys.remove(encryption);
    } else {
        QList<UnprocessedKey> keys;

        const auto unprocessedKeys = d->unprocessedKeys.values(encryption);
        for (auto &key : unprocessedKeys) {
            if (senderKeyIds.contains(key.senderKeyId)) {
                keys.append(key);
            }
        }

        for (const auto &key : std::as_const(keys)) {
            d->unprocessedKeys.remove(encryption, key);
        }
    }

    return makeReadyFuture();
}

QFuture<QHash<bool, QMultiHash<QString, QString>>> QXmppTrustMemoryStorage::keysForPostponedTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds)
{
    QHash<bool, QMultiHash<QString, QString>> keys;

    const auto unprocessedKeys = d->unprocessedKeys.values(encryption);
    for (const auto &key : unprocessedKeys) {
        if (senderKeyIds.contains(key.senderKeyId) || senderKeyIds.isEmpty()) {
            keys[key.trust].insert(key.ownerJid, key.id);
        }
    }

    return makeReadyFuture(std::move(keys));
}
/// \endcond
