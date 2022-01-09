/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

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
