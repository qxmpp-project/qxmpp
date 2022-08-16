// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTrustManager.h"

#include "QXmppFutureUtils_p.h"
#include "QXmppPromise.h"
#include "QXmppTask.h"
#include "QXmppTrustStorage.h"

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \class QXmppTrustManager
///
/// \brief The QXmppTrustManager manages end-to-end encryption trust decisions.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

///
/// Constructs a trust manager.
///
/// \param trustStorage trust storage implementation
///
QXmppTrustManager::QXmppTrustManager(QXmppTrustStorage *trustStorage)
    : m_trustStorage(trustStorage)
{
}

QXmppTrustManager::~QXmppTrustManager() = default;

///
/// Sets the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
/// \param securityPolicy security policy being applied
///
QXmppTask<void> QXmppTrustManager::setSecurityPolicy(const QString &encryption, TrustSecurityPolicy securityPolicy)
{
    return m_trustStorage->setSecurityPolicy(encryption, securityPolicy);
}

///
/// Resets the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
///
QXmppTask<void> QXmppTrustManager::resetSecurityPolicy(const QString &encryption)
{
    return m_trustStorage->resetSecurityPolicy(encryption);
}

///
/// Returns the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the set security policy
///
QXmppTask<TrustSecurityPolicy> QXmppTrustManager::securityPolicy(const QString &encryption)
{
    return m_trustStorage->securityPolicy(encryption);
}

///
/// Sets the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///
QXmppTask<void> QXmppTrustManager::setOwnKey(const QString &encryption, const QByteArray &keyId)
{
    return m_trustStorage->setOwnKey(encryption, keyId);
}

///
/// Resets the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
///
QXmppTask<void> QXmppTrustManager::resetOwnKey(const QString &encryption)
{
    return m_trustStorage->resetOwnKey(encryption);
}

///
/// Returns the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the ID of the own key
///
QXmppTask<QByteArray> QXmppTrustManager::ownKey(const QString &encryption)
{
    return m_trustStorage->ownKey(encryption);
}

///
/// Adds keys.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyIds IDs of the keys
/// \param trustLevel trust level of the keys
///
QXmppTask<void> QXmppTrustManager::addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, TrustLevel trustLevel)
{
    return m_trustStorage->addKeys(encryption, keyOwnerJid, keyIds, trustLevel);
}

///
/// Removes keys.
///
/// \param encryption encryption protocol namespace
/// \param keyIds IDs of the keys
///
QXmppTask<void> QXmppTrustManager::removeKeys(const QString &encryption, const QList<QByteArray> &keyIds)
{
    return m_trustStorage->removeKeys(encryption, keyIds);
}

///
/// Removes all keys of a key owner.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
///
QXmppTask<void> QXmppTrustManager::removeKeys(const QString &encryption, const QString &keyOwnerJid)
{
    return m_trustStorage->removeKeys(encryption, keyOwnerJid);
}

///
/// Removes all keys for encryption.
///
/// \param encryption encryption protocol namespace
///
QXmppTask<void> QXmppTrustManager::removeKeys(const QString &encryption)
{
    return m_trustStorage->removeKeys(encryption);
}

///
/// Returns the JIDs of all key owners mapped to the IDs of their keys with
/// specific trust levels.
///
/// If no trust levels are passed, all keys for encryption are returned.
///
/// \param encryption encryption protocol namespace
/// \param trustLevels trust levels of the keys
///
/// \return the key owner JIDs mapped to their keys with specific trust levels
///
QXmppTask<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> QXmppTrustManager::keys(const QString &encryption, QXmpp::TrustLevels trustLevels)
{
    return m_trustStorage->keys(encryption, trustLevels);
}

///
/// Returns the IDs of keys mapped to their trust levels for specific key
/// owners.
///
/// If no trust levels are passed, all keys for encryption and keyOwnerJids are
/// returned.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJids key owners' bare JIDs
/// \param trustLevels trust levels of the keys
///
/// \return the key IDs mapped to their trust levels for specific key owners
///
QXmppTask<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> QXmppTrustManager::keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevels trustLevels)
{
    return m_trustStorage->keys(encryption, keyOwnerJids, trustLevels);
}

///
/// Returns whether at least one key of a key owner with a specific trust level
/// is stored.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param trustLevels possible trust levels of the key
///
/// \return whether a key of the key owner with a passed trust level is stored
///
QXmppTask<bool> QXmppTrustManager::hasKey(const QString &encryption, const QString &keyOwnerJid, TrustLevels trustLevels)
{
    return m_trustStorage->hasKey(encryption, keyOwnerJid, trustLevels);
}

///
/// Sets the trust level of keys.
///
/// If a key is not stored, it is added to the storage.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
/// \param trustLevel trust level being set
///
QXmppTask<void> QXmppTrustManager::setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, TrustLevel trustLevel)
{
    QXmppPromise<void> promise;

    auto future = m_trustStorage->setTrustLevel(encryption, keyIds, trustLevel);
    future.then(this, [=](QHash<QString, QMultiHash<QString, QByteArray>> modifiedKeys) mutable {
        Q_EMIT trustLevelsChanged(modifiedKeys);
        promise.finish();
    });

    return promise.task();
}

///
/// Sets the trust level of keys specified by their key owner and trust level.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJids key owners' bare JIDs
/// \param oldTrustLevel trust level being changed
/// \param newTrustLevel trust level being set
///
QXmppTask<void> QXmppTrustManager::setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevel oldTrustLevel, TrustLevel newTrustLevel)
{
    QXmppPromise<void> promise;
    m_trustStorage->setTrustLevel(encryption, keyOwnerJids, oldTrustLevel, newTrustLevel)
        .then(this, [=](QHash<QString, QMultiHash<QString, QByteArray>> modifiedKeys) mutable {
            Q_EMIT trustLevelsChanged(modifiedKeys);
            promise.finish();
        });

    return promise.task();
}

///
/// Returns the trust level of a key.
///
/// If the key is not stored, the trust in that key is undecided.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyId ID of the key
///
/// \return the key's trust level
///
QXmppTask<TrustLevel> QXmppTrustManager::trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId)
{
    return m_trustStorage->trustLevel(encryption, keyOwnerJid, keyId);
}

///
/// Resets all data for encryption.
///
/// \param encryption encryption protocol namespace
///
QXmppTask<void> QXmppTrustManager::resetAll(const QString &encryption)
{
    return m_trustStorage->resetAll(encryption);
}

///
/// \fn QXmppTrustManager::trustLevelsChanged(const QHash<QString, QMultiHash<QString, QByteArray>> &modifiedKeys)
///
/// Emitted when the trust levels of keys changed because \c setTrustLevel()
/// added a new key or modified an existing one.
///
/// \param modifiedKeys key owners' bare JIDs mapped to their modified keys for
///        specific encryption protocol namespaces
///
