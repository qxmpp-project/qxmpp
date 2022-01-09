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

#ifndef QXMPPPACKET_H
#define QXMPPPACKET_H

#include "QXmppGlobal.h"
#include "QXmppSendResult.h"

#include <memory>

#include <QFutureInterface>

class QXmppNonza;

class QXmppPacket
{
public:
    QXmppPacket(const QXmppNonza &nonza);
    QXmppPacket(const QXmppNonza &nonza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>>);
    QXmppPacket(const QByteArray &data, bool isXmppStanza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>>);

    QByteArray data() const;
    bool isXmppStanza() const;

    QFuture<QXmpp::SendResult> future();

    void reportFinished();
    void reportResult(const QXmpp::SendResult &);

private:
    std::shared_ptr<QFutureInterface<QXmpp::SendResult>> m_interface;
    QByteArray m_data;
    bool m_isXmppStanza;
};

#endif  // QXMPPPACKET_H
