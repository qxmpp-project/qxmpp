// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

///
/// \class QXmppOmemoStorage
///
/// \brief The QXmppOmemoStorage class stores data used by
/// \xep{0384, OMEMO Encryption}.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

///
/// \fn QXmppOmemoStorage::allData()
///
/// Returns all data used by OMEMO.
///
/// \return the OMEMO data
///

///
/// \fn QXmppOmemoStorage::setOwnDevice(const std::optional<OwnDevice> &device)
///
/// Sets the own device (i.e., the device used by this client instance).
///
/// \param device own device
///

///
/// \fn QXmppOmemoStorage::addSignedPreKeyPair(uint32_t keyId, const SignedPreKeyPair &keyPair)
///
/// Adds a signed pre key pair.
///
/// \param keyId ID of the signed pre key pair
/// \param keyPair signed pre key pair
///

///
/// \fn QXmppOmemoStorage::removeSignedPreKeyPair(uint32_t keyId)
///
/// Removes a signed pre key pair.
///
/// \param keyId ID of the signed pre key pair
///

///
/// \fn QXmppOmemoStorage::addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs)
///
/// Adds pre key pairs.
///
/// \param keyPairs key IDs mapped to the pre key pairs
///

///
/// \fn QXmppOmemoStorage::removePreKeyPair(uint32_t keyId)
///
/// Removes a pre key pair.
///
/// \param keyId ID of the pre key pair
///

///
/// \fn QXmppOmemoStorage::addDevice(const QString &jid, uint32_t deviceId, const Device &device)
///
/// Adds other devices (i.e., all devices but the own one).
///
/// \param jid JID of the device owner
/// \param deviceId ID of the device
/// \param device device being added
///

///
/// \fn QXmppOmemoStorage::removeDevice(const QString &jid, uint32_t deviceId)
///
/// Removes a device of the other devices (i.e., all devices but the own one).
///
/// \param jid JID of the device owner
/// \param deviceId ID of the device being removed
///

///
/// \fn QXmppOmemoStorage::removeDevices(const QString &jid)
///
/// Removes all devices of a passed JID from the other devices (i.e., all
/// devices but the own one).
///
/// \param jid JID of the device owner
///

///
/// \fn QXmppOmemoStorage::resetAll()
///
/// Resets all data.
///
