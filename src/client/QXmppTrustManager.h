// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPTRUSTMANAGER_H
#define QXMPPTRUSTMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppTrustLevel.h"
#include "QXmppTrustSecurityPolicy.h"

template<typename T>
class QXmppTask;

class QXmppTrustStorage;

class QXMPP_EXPORT QXmppTrustManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppTrustManager(QXmppTrustStorage *trustStorage);
    ~QXmppTrustManager();

    QXmppTask<void> setSecurityPolicy(const QString &encryption, QXmpp::TrustSecurityPolicy securityPolicy);
    QXmppTask<void> resetSecurityPolicy(const QString &encryption);
    QXmppTask<QXmpp::TrustSecurityPolicy> securityPolicy(const QString &encryption);

    QXmppTask<void> setOwnKey(const QString &encryption, const QByteArray &keyId);
    QXmppTask<void> resetOwnKey(const QString &encryption);
    QXmppTask<QByteArray> ownKey(const QString &encryption);

    QXmppTask<void> addKeys(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIds, QXmpp::TrustLevel trustLevel = QXmpp::TrustLevel::AutomaticallyDistrusted);
    QXmppTask<void> removeKeys(const QString &encryption, const QList<QByteArray> &keyIds);
    QXmppTask<void> removeKeys(const QString &encryption, const QString &keyOwnerJid);
    QXmppTask<void> removeKeys(const QString &encryption);
    QXmppTask<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> keys(const QString &encryption, QXmpp::TrustLevels trustLevels = {});
    QXmppTask<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> keys(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevels trustLevels = {});
    QXmppTask<bool> hasKey(const QString &encryption, const QString &keyOwnerJid, QXmpp::TrustLevels trustLevels);

    QXmppTask<void> setTrustLevel(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel);
    QXmppTask<void> setTrustLevel(const QString &encryption, const QList<QString> &keyOwnerJids, QXmpp::TrustLevel oldTrustLevel, QXmpp::TrustLevel newTrustLevel);
    QXmppTask<QXmpp::TrustLevel> trustLevel(const QString &encryption, const QString &keyOwnerJid, const QByteArray &keyId);

    QXmppTask<void> resetAll(const QString &encryption);

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
