// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOSTORAGE_H
#define QXMPPOMEMOSTORAGE_H

#include "QXmppTask.h"
#include "qxmppomemo_export.h"

#include <optional>

#include <QDateTime>
#include <QFuture>

class QXMPPOMEMO_EXPORT QXmppOmemoStorage
{
public:
    ///
    /// Contains the data of this client instance's OMEMO device.
    ///
    struct OwnDevice
    {
        ///
        /// ID used to identify a device and fetch its bundle
        ///
        /// A valid ID must be at least 1 and at most
        /// \c std::numeric_limits<int32_t>::max().
        ///
        uint32_t id = 0;

        ///
        /// Human-readable string used to identify the device by users
        ///
        /// The label should not contain more than 53 characters.
        ///
        QString label;

        ///
        /// Private long-term key which never changes
        ///
        QByteArray privateIdentityKey;

        ///
        /// Public long-term key which never changes
        ///
        QByteArray publicIdentityKey;

        ///
        /// ID of the latest pre key pair whose public key is signed
        ///
        /// A valid ID must be at least 1 and at most
        /// \c std::numeric_limits<int32_t>::max().
        ///
        uint32_t latestSignedPreKeyId = 1;

        ///
        /// ID of the latest pre key pair
        ///
        /// A valid ID must be at least 1 and at most
        /// \c std::numeric_limits<int32_t>::max().
        ///
        uint32_t latestPreKeyId = 1;
    };

    ///
    /// Contains the data of another OMEMO device.
    /// That includes another own device (i.e., not this client instance's one)
    /// or a contact's device.
    ///
    struct Device
    {
        ///
        /// Human-readable string used to identify the device by users
        ///
        QString label;

        ///
        /// ID of the public long-term key which never changes
        ///
        QByteArray keyId;

        ///
        /// Session data which is only used internally by the OMEMO library
        ///
        QByteArray session;

        ///
        /// Count of stanzas sent to the device without receiving a response
        ///
        /// It can be used to stop encryption in order to maintain a secure
        /// communication.
        ///
        int unrespondedSentStanzasCount = 0;

        ///
        /// Count of stanzas received from the device without sending a
        /// response
        ///
        /// It can be used to send an empty response (heartbeat message) in
        /// order to maintain a secure communication.
        ///
        int unrespondedReceivedStanzasCount = 0;

        ///
        /// Date when the device was removed from the owner's device list
        ///
        /// It can be used to stop encrypting when a device is not used anymore.
        ///
        QDateTime removalFromDeviceListDate;
    };

    ///
    /// Contains the data needed to manage an OMEMO signed pre key pair.
    ///
    struct SignedPreKeyPair
    {
        ///
        /// Date when the signed pre key pair was created
        ///
        QDateTime creationDate;

        ///
        /// Actual signed pre key pair
        ///
        QByteArray data;
    };

    ///
    /// Contains all OMEMO data.
    ///
    struct OmemoData
    {
        ///
        /// Device of this client instance
        ///
        std::optional<OwnDevice> ownDevice;

        ///
        /// Key IDs mapped to their signed pre key pairs
        ///
        QHash<uint32_t, SignedPreKeyPair> signedPreKeyPairs;

        ///
        /// Key IDs mapped to their pre key pairs
        ///
        QHash<uint32_t, QByteArray> preKeyPairs;

        ///
        /// JIDs of the device owners mapped to device IDs mapped to the other
        /// devices (i.e., all devices except the own one)
        ///
        QHash<QString, QHash<uint32_t, Device>> devices;
    };

    virtual ~QXmppOmemoStorage() = default;

    virtual QXmppTask<OmemoData> allData() = 0;

    virtual QXmppTask<void> setOwnDevice(const std::optional<OwnDevice> &device) = 0;

    virtual QXmppTask<void> addSignedPreKeyPair(uint32_t keyId, const SignedPreKeyPair &keyPair) = 0;
    virtual QXmppTask<void> removeSignedPreKeyPair(uint32_t keyId) = 0;

    virtual QXmppTask<void> addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs) = 0;
    virtual QXmppTask<void> removePreKeyPair(uint32_t keyId) = 0;

    virtual QXmppTask<void> addDevice(const QString &jid, uint32_t deviceId, const Device &device) = 0;
    virtual QXmppTask<void> removeDevice(const QString &jid, uint32_t deviceId) = 0;
    virtual QXmppTask<void> removeDevices(const QString &jid) = 0;

    virtual QXmppTask<void> resetAll() = 0;
};

#endif  // QXMPPOMEMOSTORAGE_H
