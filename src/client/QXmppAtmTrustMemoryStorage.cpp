// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAtmTrustMemoryStorage.h"

#include "QXmppFutureUtils_p.h"
#include "QXmppTrustMessageKeyOwner.h"

#include <QMultiHash>

using namespace QXmpp::Private;

///
/// \class QXmppAtmTrustMemoryStorage
///
/// \brief The QXmppAtmTrustMemoryStorage class stores trust data for
/// \xep{0450, Automatic Trust Management (ATM)} in the memory.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

struct UnprocessedKey
{
    QByteArray id;
    QString ownerJid;
    QByteArray senderKeyId;
    bool trust;
};

class QXmppAtmTrustMemoryStoragePrivate
{
public:
    // encryption protocols mapped to trust message data received from endpoints
    // with unauthenticated keys
    QMultiHash<QString, UnprocessedKey> keys;
};

///
/// Constructs an ATM trust memory storage.
///
QXmppAtmTrustMemoryStorage::QXmppAtmTrustMemoryStorage()
    : d(new QXmppAtmTrustMemoryStoragePrivate)
{
}

QXmppAtmTrustMemoryStorage::~QXmppAtmTrustMemoryStorage() = default;

/// \cond
QXmppTask<void> QXmppAtmTrustMemoryStorage::addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners)
{
    const auto addKeys = [&](const QXmppTrustMessageKeyOwner &keyOwner, bool trust, const QList<QByteArray> &keyIds) {
        for (const auto &keyId : keyIds) {
            auto isKeyFound = false;

            for (auto itr = d->keys.find(encryption); itr != d->keys.end() && itr.key() == encryption; ++itr) {
                auto &key = itr.value();
                if (key.id == keyId && key.ownerJid == keyOwner.jid() && key.senderKeyId == senderKeyId) {
                    // Update the stored trust if it differs from the new one.
                    if (key.trust != trust) {
                        key.trust = trust;
                    }

                    isKeyFound = true;
                    break;
                }
            }

            // Create a new entry and store it if there is no such entry yet.
            if (!isKeyFound) {
                UnprocessedKey key;
                key.id = keyId;
                key.ownerJid = keyOwner.jid();
                key.senderKeyId = senderKeyId;
                key.trust = trust;
                d->keys.insert(encryption, key);
            }
        }
    };

    for (const auto &keyOwner : keyOwners) {
        addKeys(keyOwner, true, keyOwner.trustedKeys());
        addKeys(keyOwner, false, keyOwner.distrustedKeys());
    }

    return makeReadyTask();
}

QXmppTask<void> QXmppAtmTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting)
{
    for (auto itr = d->keys.find(encryption);
         itr != d->keys.end() && itr.key() == encryption;) {
        const auto &key = itr.value();
        if ((key.trust && keyIdsForAuthentication.contains(key.id)) ||
            (!key.trust && keyIdsForDistrusting.contains(key.id))) {
            itr = d->keys.erase(itr);
        } else {
            ++itr;
        }
    }

    return makeReadyTask();
}

QXmppTask<void> QXmppAtmTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    for (auto itr = d->keys.find(encryption);
         itr != d->keys.end() && itr.key() == encryption;) {
        if (senderKeyIds.contains(itr.value().senderKeyId)) {
            itr = d->keys.erase(itr);
        } else {
            ++itr;
        }
    }

    return makeReadyTask();
}

QXmppTask<void> QXmppAtmTrustMemoryStorage::removeKeysForPostponedTrustDecisions(const QString &encryption)
{
    d->keys.remove(encryption);
    return makeReadyTask();
}

QXmppTask<QHash<bool, QMultiHash<QString, QByteArray>>> QXmppAtmTrustMemoryStorage::keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    QHash<bool, QMultiHash<QString, QByteArray>> keys;

    const auto storedKeys = d->keys.values(encryption);
    for (const auto &key : storedKeys) {
        if (senderKeyIds.contains(key.senderKeyId) || senderKeyIds.isEmpty()) {
            keys[key.trust].insert(key.ownerJid, key.id);
        }
    }

    return makeReadyTask(std::move(keys));
}

QXmppTask<void> QXmppAtmTrustMemoryStorage::resetAll(const QString &encryption)
{
    QXmppTrustMemoryStorage::resetAll(encryption);
    d->keys.remove(encryption);
    return makeReadyTask();
}
/// \endcond
