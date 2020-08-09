/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include <QFutureInterface>
#include <QSharedPointer>

class QXmppStanza;

class QXmppPacket
{
public:
    QXmppPacket(const QXmppStanza &stanza);

    QByteArray data() const;
    bool isXmppStanza() const;

    QFuture<QXmpp::PacketState> future();

    void reportFinished();
    void reportResult(QXmpp::PacketState);

private:
    QSharedPointer<QFutureInterface<QXmpp::PacketState>> m_interface;
    QByteArray m_data;
    bool m_isXmppStanza;
};

#endif  // QXMPPPACKET_H
