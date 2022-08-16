// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppOmemoMemoryStorage.h"

#include "QXmppFutureUtils_p.h"

using namespace QXmpp::Private;

///
/// \class QXmppOmemoMemoryStorage
///
/// \brief The QXmppOmemoMemoryStorage class stores data used by
/// \xep{0384, OMEMO Encryption} in the memory.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///

class QXmppOmemoMemoryStoragePrivate
{
public:
    bool isSetUp = false;

    std::optional<QXmppOmemoStorage::OwnDevice> ownDevice;

    // IDs of pre key pairs mapped to pre key pairs
    QHash<uint32_t, QByteArray> preKeyPairs;

    // IDs of signed pre key pairs mapped to signed pre key pairs
    QHash<uint32_t, QXmppOmemoStorage::SignedPreKeyPair> signedPreKeyPairs;

    // recipient JID mapped to device ID mapped to device
    QHash<QString, QHash<uint32_t, QXmppOmemoStorage::Device>> devices;
};

///
/// Constructs an OMEMO memory storage.
///
QXmppOmemoMemoryStorage::QXmppOmemoMemoryStorage()
    : d(new QXmppOmemoMemoryStoragePrivate)
{
}

QXmppOmemoMemoryStorage::~QXmppOmemoMemoryStorage() = default;

/// \cond
QXmppTask<QXmppOmemoStorage::OmemoData> QXmppOmemoMemoryStorage::allData()
{
    return makeReadyTask(std::move(OmemoData { d->ownDevice,
                                               d->signedPreKeyPairs,
                                               d->preKeyPairs,
                                               d->devices }));
}

QXmppTask<void> QXmppOmemoMemoryStorage::setOwnDevice(const std::optional<OwnDevice> &device)
{
    d->ownDevice = device;
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::addSignedPreKeyPair(const uint32_t keyId, const SignedPreKeyPair &keyPair)
{
    d->signedPreKeyPairs.insert(keyId, keyPair);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::removeSignedPreKeyPair(const uint32_t keyId)
{
    d->signedPreKeyPairs.remove(keyId);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs)
{
    d->preKeyPairs.insert(keyPairs);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::removePreKeyPair(const uint32_t keyId)
{
    d->preKeyPairs.remove(keyId);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::addDevice(const QString &jid, const uint32_t deviceId, const QXmppOmemoStorage::Device &device)
{
    d->devices[jid].insert(deviceId, device);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::removeDevice(const QString &jid, const uint32_t deviceId)
{
    auto &devices = d->devices[jid];
    devices.remove(deviceId);

    // Remove the container for the passed JID if the container stores no
    // devices anymore.
    if (devices.isEmpty()) {
        d->devices.remove(jid);
    }

    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::removeDevices(const QString &jid)
{
    d->devices.remove(jid);
    return makeReadyTask();
}

QXmppTask<void> QXmppOmemoMemoryStorage::resetAll()
{
    d.reset(new QXmppOmemoMemoryStoragePrivate());
    return makeReadyTask();
}
/// \endcond
