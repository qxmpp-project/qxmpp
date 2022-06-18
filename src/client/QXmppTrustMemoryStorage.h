// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTMEMORYSTORAGE_H
#define QXMPPTRUSTMEMORYSTORAGE_H

#include "QXmppTrustStorage.h"

#include <memory>

class QXmppTrustMemoryStoragePrivate;

class QXMPP_EXPORT QXmppTrustMemoryStorage : virtual public QXmppTrustStorage
{
public:
    QXmppTrustMemoryStorage();
    ~QXmppTrustMemoryStorage();

    /// \cond
    QFuture<void> setSecurityPolicy(const QString &encryption, QXmpp::TrustSecurityPolicy securityPolicy) override;
    QFuture<void> resetSecurityPolicy(const QString &encryption) override;
    QFuture<QXmpp::TrustSecurityPolicy> securityPolicy(const QString &encryption) override;

    QFuture<void> setOwnKey(const QString &encryption, const QByteArray &keyId) override;
    QFuture<void> resetOwnKey(const QString &encryption) override;
    QFuture<QByteArray> ownKey(const QString &encryption) override;

    QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, QXmpp::TrustLevel trustLevel = QXmpp::TrustLevel::AutomaticallyDistrusted) override;
    QFuture<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds) override;
    QFuture<void> removeKeys(const QString &encryption, const QString &keyOwnerJid) override;
    QFuture<void> removeKeys(const QString &encryption) override;
    QFuture<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, QXmpp::TrustLevels trustLevels = {}) override;
    QFuture<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevels trustLevels = {}) override;
    QFuture<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, QXmpp::TrustLevels trustLevels) override;

    QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel) override;
    QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevel oldTrustLevel, QXmpp::TrustLevel newTrustLevel) override;
    QFuture<QXmpp::TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId) override;

    QFuture<void> resetAll(const QString &encryption) override;
    /// \endcond

private:
    std::unique_ptr<QXmppTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPTRUSTMEMORYSTORAGE_H
