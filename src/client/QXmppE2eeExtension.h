// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPE2EEEXTENSION_H
#define QXMPPE2EEEXTENSION_H

#include "QXmppError.h"
#include "QXmppExtension.h"
#include "QXmppSendResult.h"
#include "QXmppSendStanzaParams.h"

#include <memory>
#include <optional>

class QDomElement;
class QXmppMessage;
class QXmppIq;
template<typename T>
class QXmppTask;

class QXmppE2eeExtension : public QXmppExtension
{
public:
    struct NotEncrypted
    {
    };

    using MessageEncryptResult = std::variant<std::unique_ptr<QXmppMessage>, QXmppError>;
    using MessageDecryptResult = std::variant<QXmppMessage, NotEncrypted, QXmppError>;
    using IqEncryptResult = std::variant<std::unique_ptr<QXmppIq>, QXmppError>;
    using IqDecryptResult = std::variant<QDomElement, NotEncrypted, QXmppError>;

    virtual QXmppTask<MessageEncryptResult> encryptMessage(QXmppMessage &&, const std::optional<QXmppSendStanzaParams> &) = 0;
    virtual QXmppTask<MessageDecryptResult> decryptMessage(QXmppMessage &&) = 0;
    virtual QXmppTask<IqEncryptResult> encryptIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> &) = 0;
    virtual QXmppTask<IqDecryptResult> decryptIq(const QDomElement &) = 0;
    virtual bool isEncrypted(const QDomElement &) = 0;
    virtual bool isEncrypted(const QXmppMessage &) = 0;
};

#endif  // QXMPPE2EEEXTENSION_H
