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

#include "QXmppPacket_p.h"
#include "QXmppNonza.h"

#include <QFuture>
#include <QXmlStreamWriter>

/// \cond
QXmppPacket::QXmppPacket(const QXmppNonza &nonza)
    : m_interface(new QFutureInterface<QXmpp::PacketState>(QFutureInterfaceBase::Started)),
      m_isXmppStanza(nonza.isXmppStanza())
{
    QXmlStreamWriter xmlStream(&m_data);
    nonza.toXml(&xmlStream);
}

QByteArray QXmppPacket::data() const
{
    return m_data;
}

bool QXmppPacket::isXmppStanza() const
{
    return m_isXmppStanza;
}

QFuture<QXmpp::PacketState> QXmppPacket::future()
{
    return m_interface->future();
}

void QXmppPacket::reportFinished()
{
    m_interface->reportFinished();
}

void QXmppPacket::reportResult(QXmpp::PacketState result)
{
    m_interface->reportResult(result);
}
/// \endcond
