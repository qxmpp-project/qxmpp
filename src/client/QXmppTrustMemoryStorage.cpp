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

///
/// \class QXmppTrustMemoryStorage
///
/// \brief The QXmppTrustMemoryStorage class stores trust data for end-to-end
/// encryption.
///
/// \since QXmpp 1.5
///

bool ProcessedKey::operator==(const ProcessedKey &other) const
{
    return id == other.id &&
        ownerJid == other.ownerJid &&
        trustLevel == other.trustLevel;
}

bool UnprocessedKey::operator==(const UnprocessedKey &other) const
{
    return id == other.id &&
        ownerJid == other.ownerJid &&
        senderKeyId == other.senderKeyId &&
        trust == other.trust;
}

class QXmppTrustMemoryStoragePrivate : public QSharedData
{
public:
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

QXmppTrustMemoryStorage::~QXmppTrustMemoryStorage() = default;

///
/// Adds an own key (i.e., the key used by this client instance).
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///
void QXmppTrustMemoryStorage::addOwnKey(const QString &encryption, const QString &keyId)
{
    d->ownKeys.insert(encryption, keyId);
}

///
/// Removes an own key (i.e., the key used by this client instance).
///
/// \param encryption encryption protocol namespace
///
void QXmppTrustMemoryStorage::removeOwnKey(const QString &encryption)
{
    d->ownKeys.remove(encryption);
}

///
/// Returns an own key (i.e., the key used by this client instance).
///
/// \param encryption encryption protocol namespace
///
/// \return the ID of the own key
///
QFuture<QString> QXmppTrustMemoryStorage::ownKey(const QString &encryption) const
{
    auto key = d->ownKeys[encryption];
    return QXmpp::Private::makeReadyFuture(std::move(key));
}

///
/// Adds keys.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyIds IDs of the keys
/// \param trustLevel trust level of the keys
///
void QXmppTrustMemoryStorage::addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const QXmppTrustStorage::TrustLevel trustLevel)
{
    for (auto keyId : keyIds) {
        ProcessedKey key;
        key.id = keyId;
        key.ownerJid = keyOwnerJid;
        key.trustLevel = trustLevel;
        d->processedKeys.insert(encryption, key);
    }
}

///
/// Removes keys.
///
/// If keyIds is not passed, all keys for encryption are removed.
/// If encryption is also not passed, all keys are removed.
///
/// \param encryption encryption protocol namespace
/// \param keyIds IDs of the keys
///
void QXmppTrustMemoryStorage::removeKeys(const QString &encryption, const QList<QString> &keyIds)
{
    if (encryption.isEmpty()) {
        d->processedKeys.clear();
    } else if (keyIds.isEmpty()) {
        d->processedKeys.remove(encryption);
    } else {
        QList<ProcessedKey> keys;

        for (auto &key : d->processedKeys.values(encryption)) {
            if (keyIds.contains(key.id)) {
                keys.append(key);
            }
        }

        for (const auto &key : keys) {
            d->processedKeys.remove(encryption, key);
        }
    }
}

///
/// Returns the IDs of all keys for an encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the key IDs for encryption
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::keys(const QString &encryption) const
{
    QList<QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        keys.append(key.id);
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the IDs of all keys for an encryption protocol of a specific owner.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
///
/// \return the key IDs for encryption and keyOwnerJid
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::keys(const QString &encryption, const QString &keyOwnerJid) const
{
    QList<QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.ownerJid == keyOwnerJid) {
            keys.append(key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the IDs of all keys for an encryption protocol of a specific owner
/// with a specific trust level.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param trustLevel trust level of the keys
///
/// \return the key IDs for encryption, keyOwnerJid and trustLevel
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::keys(const QString &encryption, const QString &keyOwnerJid, const TrustLevel trustLevel) const
{
    QList<QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.ownerJid == keyOwnerJid && key.trustLevel == trustLevel) {
            keys.append(key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the IDs of all trusted keys for an encryption protocol of a specific
/// owner.
///
/// A key is trusted if it is automatically / manually trusted or authenticated.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
///
/// \return the IDs of trusted keys for encryption and keyOwnerJid
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::trustedKeys(const QString &encryption, const QString &keyOwnerJid) const
{
    QList<QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.ownerJid == keyOwnerJid &&
            (key.trustLevel == TrustLevel::AutomaticallyTrusted ||
             key.trustLevel == TrustLevel::ManuallyTrusted ||
             key.trustLevel == TrustLevel::Authenticated)) {
            keys.append(key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the IDs of all unauthenticated keys for an encryption protocol of a
/// specific owner.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
///
/// \return the IDs of unauthenticated keys for encryption and keyOwnerJid
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::unauthenticatedKeys(const QString &encryption, const QString &keyOwnerJid) const
{
    QList<QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.ownerJid == keyOwnerJid && key.trustLevel != TrustLevel::Authenticated) {
            keys.append(key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the JIDs of the key owners mapped to the IDs of their authenticated
/// keys.
///
/// \param encryption encryption protocol namespace
///
/// \return the key owner JIDs mapped to authenticated keys
///
QFuture<QMultiHash<QString, QString>> QXmppTrustMemoryStorage::authenticatedKeys(const QString &encryption) const
{
    QMultiHash<QString, QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.trustLevel == TrustLevel::Authenticated) {
            keys.insert(key.ownerJid, key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the JIDs of the key owners mapped to the IDs of their unauthenticated
/// keys.
///
/// \param encryption encryption protocol namespace
///
/// \return the key owner JIDs mapped to unauthenticated keys
///
QFuture<QMultiHash<QString, QString>> QXmppTrustMemoryStorage::unauthenticatedKeys(const QString &encryption) const
{
    QMultiHash<QString, QString> keys;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.trustLevel != TrustLevel::Authenticated) {
            keys.insert(key.ownerJid, key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}

///
/// Returns the JIDs of all key owners that have authenticated keys for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the key owner JIDs for authenticated keys and encryption
///
QFuture<QList<QString>> QXmppTrustMemoryStorage::keyOwnersWithAuthenticatedKeys(const QString &encryption) const
{
    QList<QString> keyOwners;

    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.trustLevel == TrustLevel::Authenticated) {
            keyOwners.append(key.ownerJid);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keyOwners));
}

///
/// Sets the trust level of keys.
///
/// If a key is not stored, it is added to the storage.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyIds IDs of the keys
/// \param trustLevel trust level being set
///
void QXmppTrustMemoryStorage::setTrustLevel(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds, const TrustLevel trustLevel)
{
    for (auto keyId : keyIds) {
        auto isKeyFound = false;

        for (auto i = d->processedKeys.find(encryption); i != d->processedKeys.end(); ++i) {
            auto &key = i.value();
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
}

///
/// Returns the trust level of a key.
///
/// If the key is not stored, it is seen as automatically distrusted.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyId ID of the key
///
/// \return the key's trust level
///
QFuture<QXmppTrustStorage::TrustLevel> QXmppTrustMemoryStorage::trustLevel(const QString &encryption, const QString &keyOwnerJid, const QString &keyId) const
{
    for (const auto &key : d->processedKeys.values(encryption)) {
        if (key.id == keyId && key.ownerJid == keyOwnerJid) {
            return QXmpp::Private::makeReadyFuture(std::move(QXmppTrustStorage::TrustLevel(key.trustLevel)));
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(TrustLevel::AutomaticallyDistrusted));
}

///
/// Adds keys that cannot be authenticated or distrusted directly because the
/// key of the trust message's sender is not yet authenticated.
///
/// Those keys are being authenticated or distrusted once the sender's key is
/// authenticated.
///
/// If keys of keyIdsForAuthentication are already stored for distrusting, they
/// are changed to be used for later authentication.
/// If keys of keyIdsForDistrusting are already stored for authentication, they
/// are changed to be used for later distrusting.
/// If the same keys are in keyIdsForAuthentication and keyIdsForDistrusting,
/// they are used for later distrusting.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyId key ID of the trust message's sender
/// \param keyOwnerJid key owner's bare JID
/// \param keyIdsForAuthentication IDs of the keys being authenticated later
/// \param keyIdsForDistrusting IDs of the keys being distrusted later
/// \param trust true if the keys are trusted by the sender, otherwise false
///
void QXmppTrustMemoryStorage::addKeysForLaterTrustDecisions(const QString &encryption, const QString &senderKeyId, const QString &keyOwnerJid, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting)
{
    QList<UnprocessedKey> keys;

    for (const auto trust : { true, false }) {
        const auto keyIds = trust ? keyIdsForAuthentication : keyIdsForDistrusting;

        for (auto keyId : keyIds) {
            auto isKeyFound = false;

            for (auto i = d->unprocessedKeys.find(encryption); i != d->unprocessedKeys.end(); ++i) {
                auto &key = i.value();
                if (key.id == keyId && key.ownerJid == keyOwnerJid && key.senderKeyId == senderKeyId) {
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
                key.ownerJid = keyOwnerJid;
                key.senderKeyId = senderKeyId;
                key.trust = trust;
                d->unprocessedKeys.insert(encryption, key);
            }
        }
    }
}

///
/// Removes keys that could not be authenticated or distrusted directly because
/// the key of the trust message's sender was not yet authenticated.
///
/// \param encryption encryption protocol namespace
/// \param keyIds IDs of the keys
/// \param trust true if the keys are trusted by the sender, otherwise false
///
void QXmppTrustMemoryStorage::removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &keyIds, const bool trust)
{
    QList<UnprocessedKey> keys;

    for (const auto &key : d->unprocessedKeys.values(encryption)) {
        if (keyIds.contains(key.id) && key.trust == trust) {
            keys.append(key);
        }
    }

    for (const auto &key : keys) {
        d->unprocessedKeys.remove(encryption, key);
    }
}

///
/// Removes keys that could not be authenticated or distrusted directly because
/// the key of the trust message's sender was not yet authenticated.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds key IDs of the trust messages' senders
///
void QXmppTrustMemoryStorage::removeKeysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds)
{
    QList<UnprocessedKey> keys;

    for (auto &key : d->unprocessedKeys.values(encryption)) {
        if (senderKeyIds.contains(key.senderKeyId)) {
            keys.append(key);
        }
    }

    for (const auto &key : keys) {
        d->unprocessedKeys.remove(encryption, key);
    }
}

///
/// Returns the JIDs of key owners mapped to the IDs of their keys not used for
/// directly authenticating or distrusting.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds key IDs of the trust messages' senders
/// \param trust true if the keys are trusted by the sender, otherwise false
///
/// \return the key owner JIDs mapped to their keys
///
QFuture<QMultiHash<QString, QString>> QXmppTrustMemoryStorage::keysForLaterTrustDecisions(const QString &encryption, const QList<QString> &senderKeyIds, const bool trust)
{
    QMultiHash<QString, QString> keys;

    for (const auto &key : d->unprocessedKeys.values(encryption)) {
        if (senderKeyIds.contains(key.senderKeyId) && key.trust == trust) {
            keys.insert(key.ownerJid, key.id);
        }
    }

    return QXmpp::Private::makeReadyFuture(std::move(keys));
}
