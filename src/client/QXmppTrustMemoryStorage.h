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
    QXmppTask<void> setSecurityPolicy(const QString &encryption, QXmpp::TrustSecurityPolicy securityPolicy) override;
    QXmppTask<void> resetSecurityPolicy(const QString &encryption) override;
    QXmppTask<QXmpp::TrustSecurityPolicy> securityPolicy(const QString &encryption) override;

    QXmppTask<void> setOwnKey(const QString &encryption, const QByteArray &keyId) override;
    QXmppTask<void> resetOwnKey(const QString &encryption) override;
    QXmppTask<QByteArray> ownKey(const QString &encryption) override;

    QXmppTask<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, QXmpp::TrustLevel trustLevel = QXmpp::TrustLevel::AutomaticallyDistrusted) override;
    QXmppTask<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds) override;
    QXmppTask<void> removeKeys(const QString &encryption, const QString &keyOwnerJid) override;
    QXmppTask<void> removeKeys(const QString &encryption) override;
    QXmppTask<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, QXmpp::TrustLevels trustLevels = {}) override;
    QXmppTask<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevels trustLevels = {}) override;
    QXmppTask<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, QXmpp::TrustLevels trustLevels) override;

    QXmppTask<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel) override;
    QXmppTask<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevel oldTrustLevel, QXmpp::TrustLevel newTrustLevel) override;
    QXmppTask<QXmpp::TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId) override;

    QXmppTask<void> resetAll(const QString &encryption) override;
    /// \endcond

private:
    std::unique_ptr<QXmppTrustMemoryStoragePrivate> d;
};

#endif  // QXMPPTRUSTMEMORYSTORAGE_H
