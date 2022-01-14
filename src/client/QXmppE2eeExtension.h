// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPE2EEEXTENSION_H
#define QXMPPE2EEEXTENSION_H

#include "QXmppSendResult.h"

class QDomElement;
class QXmppMessage;
class QXmppIq;
template<typename T>
class QFuture;

class QXmppE2eeExtension
{
public:
    struct NotEncrypted {};

    using EncryptMessageResult = std::variant<QByteArray, QXmpp::SendError>;
    using IqEncryptResult = std::variant<QByteArray, QXmpp::SendError>;
    using IqDecryptResult = std::variant<QDomElement, NotEncrypted, QXmpp::SendError>;

    virtual QFuture<EncryptMessageResult> encryptMessage(QXmppMessage &&) = 0;

    virtual QFuture<IqEncryptResult> encryptIq(QXmppIq &&) = 0;
    virtual QFuture<IqDecryptResult> decryptIq(const QDomElement &) = 0;
};

#endif // QXMPPE2EEEXTENSION_H
