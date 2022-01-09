/*
 * Copyright (C) 2008-2022 The QXmpp developers
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
    QByteArray id;
    QString ownerJid;
    QXmppTrustStorage::TrustLevel trustLevel;
};

struct UnprocessedKey
{
    QByteArray id;
    QString ownerJid;
    QByteArray senderKeyId;
    bool trust;
};

class QXmppTrustMemoryStoragePrivate
{
public:
    // encryption protocols mapped to security policies
    QMap<QString, QXmppTrustStorage::SecurityPolicy> securityPolicies;

    // encryption protocols mapped to keys of this client instance
    QMap<QString, QByteArray> ownKeys;

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

QXmppTrustMemoryStorage::~QXmppTrustMemoryStorage() = default;

/// \cond
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

QFuture<void> QXmppTrustMemoryStorage::addOwnKey(const QString &encryption, const QByteArray &keyId)
{
    d->ownKeys.insert(encryption, keyId);
    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeOwnKey(const QString &encryption)
{
    d->ownKeys.remove(encryption);
    return makeReadyFuture();
}

QFuture<QByteArray> QXmppTrustMemoryStorage::ownKey(const QString &encryption)
{
    auto key = d->ownKeys[encryption];
    return makeReadyFuture(std::move(key));
}

QFuture<void> QXmppTrustMemoryStorage::addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, const QXmppTrustStorage::TrustLevel trustLevel)
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

QFuture<void> QXmppTrustMemoryStorage::removeKeys(const QString &encryption, const QList<QByteArray> &keyIds)
{
    if (encryption.isEmpty()) {
        d->processedKeys.clear();
    } else if (keyIds.isEmpty()) {
        d->processedKeys.remove(encryption);
    } else {
        for (auto itr = d->processedKeys.find(encryption);
             itr != d->processedKeys.end() && itr.key() == encryption;) {
            if (keyIds.contains(itr.value().id)) {
                itr = d->processedKeys.erase(itr);
            } else {
                itr++;
            }
        }
    }

    return makeReadyFuture();
}

QFuture<QHash<QXmppTrustStorage::TrustLevel, QMultiHash<QString, QByteArray>>> QXmppTrustMemoryStorage::keys(const QString &encryption, const TrustLevels trustLevels)
{
    QHash<TrustLevel, QMultiHash<QString, QByteArray>> keys;

    const auto processedKeys = d->processedKeys.values(encryption);
    for (const auto &key : processedKeys) {
        const auto trustLevel = key.trustLevel;
        if (trustLevels.testFlag(trustLevel) || !trustLevels) {
            keys[trustLevel].insert(key.ownerJid, key.id);
        }
    }

    return makeReadyFuture(std::move(keys));
}

QFuture<void> QXmppTrustMemoryStorage::setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, const TrustLevel trustLevel)
{
    for (auto itr = keyIds.constBegin(); itr != keyIds.constEnd(); ++itr) {
        const auto keyOwnerJid = itr.key();
        const auto keyId = itr.value();

        auto isKeyFound = false;

        for (auto itrProcessedKeys = d->processedKeys.find(encryption);
             itrProcessedKeys != d->processedKeys.end() && itrProcessedKeys.key() == encryption;
             ++itrProcessedKeys) {
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

QFuture<QXmppTrustStorage::TrustLevel> QXmppTrustMemoryStorage::trustLevel(const QString &encryption, const QByteArray &keyId)
{
    const auto processedKeys = d->processedKeys.values(encryption);
    for (const auto &key : processedKeys) {
        if (key.id == keyId) {
            return makeReadyFuture(std::move(QXmppTrustStorage::TrustLevel(key.trustLevel)));
        }
    }

    return makeReadyFuture(std::move(TrustLevel::AutomaticallyDistrusted));
}

QFuture<void> QXmppTrustMemoryStorage::addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners)
{
    const auto addKeys = [&](const QXmppTrustMessageKeyOwner &keyOwner, bool trust, const QList<QByteArray> &keyIds) {
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

QFuture<void> QXmppTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting)
{
    for (auto itr = d->unprocessedKeys.find(encryption);
         itr != d->unprocessedKeys.end() && itr.key() == encryption;) {
        const auto &key = itr.value();
        if ((key.trust && keyIdsForAuthentication.contains(key.id)) ||
            (!key.trust && keyIdsForDistrusting.contains(key.id))) {
            itr = d->unprocessedKeys.erase(itr);
        } else {
            ++itr;
        }
    }
    return makeReadyFuture();
}

QFuture<void> QXmppTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    if (encryption.isEmpty()) {
        d->unprocessedKeys.clear();
    } else if (senderKeyIds.isEmpty()) {
        d->unprocessedKeys.remove(encryption);
    } else {
        for (auto itr = d->unprocessedKeys.find(encryption);
             itr != d->unprocessedKeys.end() && itr.key() == encryption;) {
            if (senderKeyIds.contains(itr.value().senderKeyId)) {
                itr = d->unprocessedKeys.erase(itr);
            } else {
                ++itr;
            }
        }
    }

    return makeReadyFuture();
}

QFuture<QHash<bool, QMultiHash<QString, QByteArray>>> QXmppTrustMemoryStorage::keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    QHash<bool, QMultiHash<QString, QByteArray>> keys;

    const auto unprocessedKeys = d->unprocessedKeys.values(encryption);
    for (const auto &key : unprocessedKeys) {
        if (senderKeyIds.contains(key.senderKeyId) || senderKeyIds.isEmpty()) {
            keys[key.trust].insert(key.ownerJid, key.id);
        }
    }

    return makeReadyFuture(std::move(keys));
}
/// \endcond
