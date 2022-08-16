// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPATMTRUSTSTORAGE_H
#define QXMPPATMTRUSTSTORAGE_H

#include "QXmppTrustStorage.h"

#include <QList>

class QXmppTrustMessageKeyOwner;

class QXMPP_EXPORT QXmppAtmTrustStorage : virtual public QXmppTrustStorage
{
public:
    virtual ~QXmppAtmTrustStorage() = default;

    virtual QXmppTask<void> addKeysForPostponedTrustDecisions(const QString &encryption, const QByteArray &senderKeyId, const QList<QXmppTrustMessageKeyOwner> &keyOwners) = 0;
    virtual QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting) = 0;
    virtual QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds) = 0;
    virtual QXmppTask<void> removeKeysForPostponedTrustDecisions(const QString &encryption) = 0;
    virtual QXmppTask<QHash<bool, QMultiHash<QString, QByteArray>>> keysForPostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds = {}) = 0;
};

#endif  // QXMPPATMTRUSTSTORAGE_H
