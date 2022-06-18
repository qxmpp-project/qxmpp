// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTSTORAGE_H
#define QXMPPTRUSTSTORAGE_H

#include "QXmppGlobal.h"
#include "QXmppTrustLevel.h"
#include "QXmppTrustSecurityPolicy.h"

#include <QFuture>

class QXMPP_EXPORT QXmppTrustStorage
{
public:
    virtual ~QXmppTrustStorage() = default;

    virtual QFuture<void> setSecurityPolicy(const QString &encryption, QXmpp::TrustSecurityPolicy securityPolicy) = 0;
    virtual QFuture<void> resetSecurityPolicy(const QString &encryption) = 0;
    virtual QFuture<QXmpp::TrustSecurityPolicy> securityPolicy(const QString &encryption) = 0;

    virtual QFuture<void> setOwnKey(const QString &encryption, const QByteArray &keyId) = 0;
    virtual QFuture<void> resetOwnKey(const QString &encryption) = 0;
    virtual QFuture<QByteArray> ownKey(const QString &encryption) = 0;

    virtual QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, QXmpp::TrustLevel trustLevel = QXmpp::TrustLevel::AutomaticallyDistrusted) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption, const QString &keyOwnerJid) = 0;
    virtual QFuture<void> removeKeys(const QString &encryption) = 0;
    virtual QFuture<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, QXmpp::TrustLevels trustLevels = {}) = 0;
    virtual QFuture<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevels trustLevels = {}) = 0;
    virtual QFuture<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, QXmpp::TrustLevels trustLevels) = 0;

    virtual QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel) = 0;
    virtual QFuture<QHash<QString, QMultiHash<QString, QByteArray>>> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevel oldTrustLevel, QXmpp::TrustLevel newTrustLevel) = 0;
    virtual QFuture<QXmpp::TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId) = 0;

    virtual QFuture<void> resetAll(const QString &encryption) = 0;
};

#endif  // QXMPPTRUSTSTORAGE_H
