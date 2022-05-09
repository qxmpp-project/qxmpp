// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOMEMORYSTORAGE_H
#define QXMPPOMEMOMEMORYSTORAGE_H

#include "QXmppGlobal.h"
#include "QXmppOmemoStorage.h"

#include <memory>

class QXmppOmemoMemoryStoragePrivate;

class QXMPP_EXPORT QXmppOmemoMemoryStorage : public QXmppOmemoStorage
{
public:
    QXmppOmemoMemoryStorage();
    ~QXmppOmemoMemoryStorage() override;

    /// \cond
    QFuture<OmemoData> allData() override;

    QFuture<void> setOwnDevice(const std::optional<OwnDevice> &device) override;

    QFuture<void> addSignedPreKeyPair(uint32_t keyId, const SignedPreKeyPair &keyPair) override;
    QFuture<void> removeSignedPreKeyPair(uint32_t keyId) override;

    QFuture<void> addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs) override;
    QFuture<void> removePreKeyPair(uint32_t keyId) override;

    QFuture<void> addDevice(const QString &jid, uint32_t deviceId, const Device &device) override;
    QFuture<void> removeDevice(const QString &jid, uint32_t deviceId) override;
    QFuture<void> removeDevices(const QString &jid) override;

    QFuture<void> resetAll() override;
    /// \endcond

private:
    std::unique_ptr<QXmppOmemoMemoryStoragePrivate> d;
};

#endif  // QXMPPOMEMOMEMORYSTORAGE_H
