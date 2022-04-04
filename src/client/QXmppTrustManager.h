// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTMANAGER_H
#define QXMPPTRUSTMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppTrustStorage.h"

class QXMPP_EXPORT QXmppTrustManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppTrustManager(QXmppTrustStorage *trustStorage);
    ~QXmppTrustManager();

    QFuture<void> setSecurityPolicy(const QString &encryption, QXmppTrustStorage::SecurityPolicy securityPolicy);
    QFuture<void> resetSecurityPolicy(const QString &encryption);
    QFuture<QXmppTrustStorage::SecurityPolicy> securityPolicy(const QString &encryption);

    QFuture<void> setOwnKey(const QString &encryption, const QByteArray &keyId);
    QFuture<void> resetOwnKey(const QString &encryption);
    QFuture<QByteArray> ownKey(const QString &encryption);

    QFuture<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, QXmppTrustStorage::TrustLevel trustLevel = QXmppTrustStorage::AutomaticallyDistrusted);
    QFuture<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds);
    QFuture<void> removeKeys(const QString &encryption, const QString &keyOwnerJid);
    QFuture<void> removeKeys(const QString &encryption);
    QFuture<QHash<QXmppTrustStorage::TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, QXmppTrustStorage::TrustLevels trustLevels = {});
    QFuture<QHash<QString, QHash<QByteArray, QXmppTrustStorage::TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmppTrustStorage::TrustLevels trustLevels = {});
    QFuture<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, QXmppTrustStorage::TrustLevels trustLevels);

    QFuture<void> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, QXmppTrustStorage::TrustLevel trustLevel);
    QFuture<void> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, QXmppTrustStorage::TrustLevel oldTrustLevel, QXmppTrustStorage::TrustLevel newTrustLevel);
    QFuture<QXmppTrustStorage::TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId);

    QFuture<void> resetAll(const QString &encryption);

    Q_SIGNAL void trustLevelsChanged(const QHash<QString, QMultiHash<QString, QByteArray>> &modifiedKeys);

protected:
    /// \cond
    inline QXmppTrustStorage *trustStorage() const
    {
        return m_trustStorage;
    }
    /// \endcond

private:
    QXmppTrustStorage *m_trustStorage;
};

#endif  // QXMPPTRUSTMANAGER_H
