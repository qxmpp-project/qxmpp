// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOMEMORYSTORAGE_H
#define QXMPPOMEMOMEMORYSTORAGE_H

#include "QXmppOmemoStorage.h"
#include "QXmppTask.h"
#include "qxmppomemo_export.h"

#include <memory>

class QXmppOmemoMemoryStoragePrivate;

class QXMPPOMEMO_EXPORT QXmppOmemoMemoryStorage : public QXmppOmemoStorage
{
public:
    QXmppOmemoMemoryStorage();
    ~QXmppOmemoMemoryStorage() override;

    /// \cond
    QXmppTask<OmemoData> allData() override;

    QXmppTask<void> setOwnDevice(const std::optional<OwnDevice> &device) override;

    QXmppTask<void> addSignedPreKeyPair(uint32_t keyId, const SignedPreKeyPair &keyPair) override;
    QXmppTask<void> removeSignedPreKeyPair(uint32_t keyId) override;

    QXmppTask<void> addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs) override;
    QXmppTask<void> removePreKeyPair(uint32_t keyId) override;

    QXmppTask<void> addDevice(const QString &jid, uint32_t deviceId, const Device &device) override;
    QXmppTask<void> removeDevice(const QString &jid, uint32_t deviceId) override;
    QXmppTask<void> removeDevices(const QString &jid) override;

    QXmppTask<void> resetAll() override;
    /// \endcond

private:
    std::unique_ptr<QXmppOmemoMemoryStoragePrivate> d;
};

#endif  // QXMPPOMEMOMEMORYSTORAGE_H
