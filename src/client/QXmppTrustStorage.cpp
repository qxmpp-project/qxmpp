// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

///
/// \class QXmppTrustStorage
///
/// \brief The QXmppTrustStorage class stores trust data for end-to-end
/// encryption.
///
/// The term "key" is used for a public long-term key.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppTrustStorage::setSecurityPolicy(const QString &encryption, SecurityPolicy securityPolicy)
///
/// Sets the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
/// \param securityPolicy security policy being applied
///

///
/// \fn QXmppTrustStorage::resetSecurityPolicy(const QString &encryption)
///
/// Resets the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
///

///
/// \fn QXmppTrustStorage::securityPolicy(const QString &encryption)
///
/// Returns the security policy for an encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the set security policy
///

///
/// \fn QXmppTrustStorage::setOwnKey(const QString &encryption, const QByteArray &keyId)
///
/// Sets the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///

///
/// \fn QXmppTrustStorage::resetOwnKey(const QString &encryption)
///
/// Resets the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
///

///
/// \fn QXmppTrustStorage::ownKey(const QString &encryption)
///
/// Returns the own key (i.e., the key used by this client instance) for an
/// encryption protocol.
///
/// \param encryption encryption protocol namespace
///
/// \return the ID of the own key
///

///
/// \fn QXmppTrustStorage::addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, const QXmppTrustStorage::TrustLevel trustLevel)
///
/// Adds keys.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
/// \param keyIds IDs of the keys
/// \param trustLevel trust level of the keys
///

///
/// \fn QXmppTrustStorage::removeKeys(const QString &encryption, const QList<QByteArray> &keyIds)
///
/// Removes keys.
///
/// \param encryption encryption protocol namespace
/// \param keyIds IDs of the keys
///

///
/// \fn QXmppTrustStorage::removeKeys(const QString &encryption, const QString &keyOwnerJid)
///
/// Removes all keys of a key owner.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid key owner's bare JID
///

///
/// \fn QXmppTrustStorage::removeKeys(const QString &encryption)
///
/// Removes all keys for encryption.
///
/// \param encryption encryption protocol namespace
///

///
/// \fn QXmppTrustStorage::keys(const QString &encryption, TrustLevels trustLevels = {})
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

///
/// \fn QXmppTrustStorage::keys(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevels trustLevels = {})
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

///
/// \fn QXmppTrustStorage::hasKey(const QString &encryption, const QString &keyOwnerJid, TrustLevels trustLevels)
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

///
/// \fn QXmppTrustStorage::setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, TrustLevel trustLevel)
///
/// Sets the trust level of keys.
///
/// If a key is not stored, it is added to the storage.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
/// \param trustLevel trust level being set
///

///
/// \fn QXmppTrustStorage::setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, TrustLevel oldTrustLevel, TrustLevel newTrustLevel)
///
/// Sets the trust level of keys specified by their key owner and trust level.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJids key owners' bare JIDs
/// \param oldTrustLevel trust level being changed
/// \param newTrustLevel trust level being set
///

///
/// \fn QXmppTrustStorage::trustLevel(const QString &encryption, const QByteArray &keyId)
///
/// Returns the trust level of a key.
///
/// If the key is not stored, the trust in that key is undecided.
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///
/// \return the key's trust level
///

///
/// \fn QXmppTrustStorage::resetAll(const QString &encryption)
///
/// Resets all data for encryption.
///
/// \param encryption encryption protocol namespace
///
