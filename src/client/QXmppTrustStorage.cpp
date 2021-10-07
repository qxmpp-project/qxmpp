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

///
/// \class QXmppTrustStorage
///
/// \brief The QXmppTrustStorage class stores trust data for end-to-end
/// encryption.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppTrustStorage::setSecurityPolicies(const QString &encryption = {}, SecurityPolicy securityPolicy = SecurityPolicy::NoSecurityPolicy)
///
/// Sets the security policy for an encryption protocol or resets the set
/// security policies.
///
/// If securityPolicy is not passed, the set security policy for encryption is
/// reset.
/// If also encryption is not passed, all set security policies are reset.
///
/// \param encryption encryption protocol namespace
/// \param securityPolicy security policy being applied
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
/// \fn QXmppTrustStorage::addOwnKey(const QString &encryption, const QByteArray &keyId)
///
/// Adds an own key (i.e., the key used by this client instance).
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///

///
/// \fn QXmppTrustStorage::removeOwnKey(const QString &encryption)
///
/// Removes an own key (i.e., the key used by this client instance).
///
/// \param encryption encryption protocol namespace
///

///
/// \fn QXmppTrustStorage::ownKey(const QString &encryption)
///
/// Returns an own key (i.e., the key used by this client instance).
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
/// If keyIds is not passed, all keys for encryption are removed.
/// If encryption is also not passed, all keys are removed.
///
/// \param encryption encryption protocol namespace
/// \param keyIds IDs of the keys
///

///
/// \fn QXmppTrustStorage::keys(const QString &encryption, TrustLevels trustLevels = {})
///
/// Returns the JIDs of the key owners mapped to the IDs of their keys with a
/// specific trust level.
///
/// If no trust levels are passed, all keys are returned.
///
/// \param encryption encryption protocol namespace
/// \param trustLevels trust levels of the keys
///
/// \return the key owner JIDs mapped to their keys with a specific trust level
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
/// If the key is not stored, it is seen as automatically distrusted.
///
/// \param encryption encryption protocol namespace
/// \param keyId ID of the key
///
/// \return the key's trust level
///

///
/// \fn QXmppTrustStorage::addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners)
///
/// Adds keys that cannot be authenticated or distrusted directly because the
/// key of the trust message's sender is not yet authenticated.
///
/// Those keys are being authenticated or distrusted once the sender's key is
/// authenticated.
/// Each element of keyOwners (i.e., keyOwner) can contain keys for postponed
/// authentication as trustedKeys or for postponed distrusting as
/// distrustedKeys.
///
/// If keys of keyOwner.trustedKeys() are already stored for postponed
/// distrusting, they are changed to be used for postponed authentication.
/// If keys of keyOwner.distrustedKeys() are already stored for postponed
/// authentication, they are changed to be used for postponed distrusting.
/// If the same keys are in keyOwner.trustedKeys() and
/// keyOwner.distrustedKeys(), they are used for postponed distrusting.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyId key ID of the trust message's sender
/// \param keyOwners key owners containing key IDs for postponed trust decisions
///

///
/// \fn QXmppTrustStorage::removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting)
///
/// Removes keys for postponed authentication or distrusting.
///
/// \param encryption encryption protocol namespace
/// \param keyIdsForAuthentication IDs of the keys for postponed authentication
/// \param keyIdsForDistrusting IDs of the keys for postponed distrusting
///

///
/// \fn QXmppTrustStorage::removeKeysForPostponedTrustDecisions(const QString &encryption = {}, const QList<QByteArray> &senderKeyIds = {})
///
/// Removes keys for postponed authentication or distrusting by the trust
/// message's sender's key ID.
///
/// If senderKeyIds is empty, all keys for encryption are removed.
/// If encryption is empty too, all keys are removed.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds key IDs of the trust messages' senders
///

///
/// \fn QXmppTrustStorage::keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds = {})
///
/// Returns the JIDs of key owners mapped to the IDs of their keys stored for
/// postponed authentication (true) or postponed distrusting (false).
///
/// If senderKeyIds is empty, all keys for encryption are returned.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds key IDs of the trust messages' senders
///
/// \return the key owner JIDs mapped to their keys
///
