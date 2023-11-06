// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDateTime;
class QXmppE2eeMetadataPrivate;

class QXMPP_EXPORT QXmppE2eeMetadata
{
public:
    QXmppE2eeMetadata();
    /// \cond
    QXmppE2eeMetadata(QSharedDataPointer<QXmppE2eeMetadataPrivate> d);
    /// \endcond
    QXmppE2eeMetadata(const QXmppE2eeMetadata &other);
    QXmppE2eeMetadata(QXmppE2eeMetadata &&);
    ~QXmppE2eeMetadata();

    QXmppE2eeMetadata &operator=(const QXmppE2eeMetadata &other);
    QXmppE2eeMetadata &operator=(QXmppE2eeMetadata &&);

    QXmpp::EncryptionMethod encryption() const;
    void setEncryption(QXmpp::EncryptionMethod encryption);

    QByteArray senderKey() const;
    void setSenderKey(const QByteArray &keyId);

    // XEP-0420: Stanza Content Encryption
    QDateTime sceTimestamp() const;
    void setSceTimestamp(const QDateTime &timestamp);

private:
    friend class QXmppStanza;

    QSharedDataPointer<QXmppE2eeMetadataPrivate> d;
};
